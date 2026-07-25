#include <Arduino.h>
#include "app/RobotApplication.h"
#include <math.h>

#include "Constants.h"
#include "Pins.h"
#include "RobotConfig.h"
#include "Types.h"
#include "Version.h"
#include "Watchdog.h"

#include "communication/Joystick.h"
#include "communication/UART.h"
#include "communication/WiFiJoystickLink.h"
#include "control/MotionController.h"
#include "control/RobotArm.h"
#include "control/SlotManager.h"
#include "core/ErrorManager.h"
#include "core/FailSafe.h"
#include "core/StateMachine.h"
#include "core/TaskScheduler.h"
#include "drivers/LinearActuator.h"
#include "drivers/MotorDriver.h"
#include "drivers/OLED.h"
#include "drivers/PowerManager.h"
#include "drivers/RFID.h"
#include "drivers/ServoDriver.h"
#include "control/ServoManager.h"
#include "mission/Finish.h"
#include "mission/MissionManager.h"
#include "mission/Zipline.h"
#include "ui/Display.h"

// =============================================================================
// UYGULAMA YAPILANDIRMASI
//
// Bu bolum yalnizca main.cpp icinde kullanilan calisma parametrelerini tutar.
// Donanima ait pinler Pins.h, proje genel ayarlari ise RobotConfig.h icinde
// tutulmalidir. Boylece ayni bilgi birden fazla dosyada tekrar edilmez.
// =============================================================================
namespace AppConfig {
constexpr uint32_t SERIAL_BAUD = Pins::LINK_BAUD;
constexpr uint32_t POWER_CHECK_INTERVAL_MS = 1000;
constexpr float LOW_BATTERY_THRESHOLD_V = 10.5f;
constexpr float RECOVERY_THRESHOLD_V = 11.1f;
constexpr uint8_t REQUIRED_LOW_SAMPLES = 3;
constexpr uint8_t DRIVE_MAX_PWM = 200;
constexpr uint8_t DRIVE_RAMP_STEP = 8;
constexpr uint16_t DRIVE_RAMP_INTERVAL_MS = 20;

constexpr JointConfig ARM_JOINTS[ServoManager::JOINT_COUNT] = {
    {0, 0, 180, 90, 60},
    {1, 0, 180, 90, 60},
    {2, 0, 180, 90, 60},
    {3, 0, 180, 90, 60},
    {4, 0, 180, 90, 90}
};

constexpr RobotArmConfig ARM_CONFIG = {
    Pins::JOYSTICK_TIMEOUT_MS,
    15,
    90,
    20,
    90,
    Constants::JoystickButtons::GRIPPER_OPEN,
    Constants::JoystickButtons::GRIPPER_CLOSE,
    false,
    false,
    false,
    false
};
}  // namespace AppConfig

// =============================================================================
// KONTROL MODU
//
// DRIVE: Joystick robotun surus motorlarini kontrol eder.
// ARM:   Joystick robot kolunu kontrol eder ve surus motorlari durdurulur.
//
// Bu ayrim, ayni joystick komutunun hem robotu hem de kolu ayni anda hareket
// ettirmesini onleyen temel guvenlik katmanidir.
// =============================================================================
enum class ControlMode : uint8_t {
    DRIVE,
    ARM
};

// =============================================================================
// SISTEM NESNELERI
//
// Nesneler global tutulur; cunku Arduino setup()/loop() yapisi boyunca omurleri
// devam etmelidir. Gruplama, bagimliliklarin ve sorumluluklarin okunmasini
// kolaylastirir.
// =============================================================================

// Cekirdek ve guvenlik servisleri
Watchdog watchdog;
FailSafe failSafe;
StateMachine stateMachine;
ErrorManager errorManager;
TaskScheduler taskScheduler;

// Haberlesme katmani
UARTLink uartLink(Serial);
WiFiJoystickLink wifiJoystickLink;
Joystick joystick;

// Donanim suruculeri
MotorDriver motorDriver;
LinearActuator linearActuator;
RFID rfid;
OLED oled;
PowerManager powerManager;
ServoDriver servoDriver;
ServoManager servoManager;

// Ust seviye kontrol ve arayuz modulleri
MotionController motionController;
SlotManager slotManager;
RobotArm robotArm;
Display display(oled);

// Gorev modulleri
Zipline zipline(motionController, linearActuator, failSafe);
Finish finish;
MissionManager missionManager(
    stateMachine,
    failSafe,
    motionController,
    slotManager,
    rfid,
    linearActuator,
    zipline,
    finish
);

// =============================================================================
// CALISMA ZAMANI DURUMU
//
// Bu degiskenler sabit ayar degildir; robot calisirken degisir. Her modulu
// kullanmadan once ilgili "Available" bayragi kontrol edilir. Boylece opsiyonel
// bir donanim baslatilamadiginda tum sistem gereksiz yere cokmez.
// =============================================================================
namespace Runtime {
ControlMode controlMode = ControlMode::DRIVE;
bool powerMonitoringAvailable = false;
bool servoSystemAvailable = false;
bool linearActuatorAvailable = false;
bool rfidAvailable = false;
bool communicationFailSafeTriggered = false;
bool lowBatteryFailSafeTriggered = false;
uint8_t consecutiveLowBatterySamples = 0;

// Mod degisiminden sonra eksenler merkeze gelmeden yeni mod komut kabul etmez.
bool controlInputArmed = false;

// ZIPLINE POSITIONING baslarken joystick merkeze alinmalidir.
bool ziplineDriveInputArmed = false;
}  // namespace Runtime

/**
 * @brief RobotState degerini seri port ve OLED icin okunabilir metne cevirir.
 */
static const char* robotStateName(RobotState state) {
    switch (state) {
        case RobotState::IDLE: return "IDLE";
        case RobotState::MANUAL: return "MANUAL";
        case RobotState::PICKUP: return "PICKUP";
        case RobotState::DELIVERY: return "DELIVERY";
        case RobotState::ZIPLINE: return "ZIPLINE";
        case RobotState::FINISH: return "FINISH";
        case RobotState::FAIL_SAFE: return "FAIL_SAFE";
    }
    return "UNKNOWN";
}

/**
 * @brief ErrorCode degerini kullaniciya gosterilecek metne cevirir.
 */
static const char* errorCodeName(ErrorCode code) {
    switch (code) {
        case ErrorCode::NONE: return "NONE";
        case ErrorCode::COMMUNICATION_LOST: return "COMMUNICATION LOST";
        case ErrorCode::RFID_READ_FAILED: return "RFID READ FAILED";
        case ErrorCode::MOTOR_FAULT: return "MOTOR FAULT";
        case ErrorCode::SERVO_FAULT: return "SERVO FAULT";
        case ErrorCode::LOW_BATTERY: return "LOW BATTERY";
        case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN ERROR";
    }
    return "UNKNOWN ERROR";
}

/**
 * @brief Aktif joystick kontrol modunun adini dondurur.
 */
static const char* controlModeName(ControlMode mode) {
    return mode == ControlMode::DRIVE ? "DRIVE" : "ARM";
}

/**
 * @brief ZIPLINE alt durumunu okunabilir metne cevirir.
 */
static const char* ziplineStateName(Zipline::State state) {
    switch (state) {
        case Zipline::State::IDLE: return "IDLE";
        case Zipline::State::EXTENDING: return "EXTENDING";
        case Zipline::State::POSITIONING: return "POSITIONING";
        case Zipline::State::SLIDING: return "SLIDING";
        case Zipline::State::RETRACTING: return "RETRACTING";
        case Zipline::State::COMPLETED: return "COMPLETED";
        case Zipline::State::CANCELLED: return "CANCELLED";
        case Zipline::State::FAILED: return "FAILED";
    }
    return "UNKNOWN";
}

/**
 * @brief X, Y ve twist eksenlerinin deadzone sonrasinda merkezde olup olmadigini kontrol eder.
 */
static bool joystickAxesCentered() {
    const JoystickPacket& packet = joystick.packet();
    return packet.valid &&
           packet.xPercent == 0 &&
           packet.yPercent == 0 &&
           packet.twistPercent == 0;
}

/**
 * @brief Mod degisimi sonrasinda joystick merkezlenene kadar hareketi kilitler.
 */
static bool armControlInputWhenCentered() {
    if (Runtime::controlInputArmed) {
        return true;
    }

    if (!joystickAxesCentered()) {
        motionController.stop();
        robotArm.setActive(false);
        return false;
    }

    Runtime::controlInputArmed = true;
    uartLink.sendOk("MODE_READY,", controlModeName(Runtime::controlMode));
    return true;
}

/**
 * @brief Bir BTS7960 kanalinin iki kontrol pininin de atanmis olup olmadigini
 *        kontrol eder. UNUSED_PIN bulunan bir modul baslatilmaz.
 */
static bool pinsAssigned(const Pins::BtsPins& pins) {
    return pins.rpwm != Pins::UNUSED_PIN && pins.lpwm != Pins::UNUSED_PIN;
}

/**
 * @brief Mevcut gorev durumunda manuel suruse izin verilip verilmedigini belirler.
 *
 * ZIPLINE durumunda surus yalnizca POSITIONING alt durumunda aciktir. Diger
 * zipline asamalarinda motorlar durmali ve mekanizma pilot onayini beklemelidir.
 */
static bool isDriveMissionState(RobotState state) {
    if (state == RobotState::MANUAL ||
        state == RobotState::PICKUP ||
        state == RobotState::DELIVERY) {
        return true;
    }

    return state == RobotState::ZIPLINE &&
           zipline.getState() == Zipline::State::POSITIONING;
}

/**
 * @brief Joystick baglanti zaman asiminin su anda kritik olup olmadigini belirler.
 *
 * Joystick yalnizca aktif olarak bir hareket sistemi kontrol ediyorsa timeout
 * FailSafe sebebi sayilir.
 */
static bool joystickControlActive() {
    const RobotState state = missionManager.currentState();

    if (state == RobotState::ZIPLINE) {
        return zipline.getState() == Zipline::State::POSITIONING;
    }

    const bool manualState =
        state == RobotState::MANUAL ||
        state == RobotState::PICKUP ||
        state == RobotState::DELIVERY;

    if (!manualState) {
        return false;
    }

    return Runtime::controlMode == ControlMode::DRIVE ||
           (Runtime::controlMode == ControlMode::ARM &&
            Runtime::servoSystemAvailable);
}

/**
 * @brief Acilis sirasinda proje, surum ve build bilgilerini seri porta yazar.
 */
static void printProjectInformation() {
    Serial.println();
    Serial.println("================================");
    Serial.println(Version::PROJECT_NAME);
    Serial.println(Version::COMPETITION_NAME);
    Serial.print("Software Version: ");
    Serial.println(Version::SOFTWARE_VERSION);
    Serial.print("Build Type: ");
    Serial.println(Version::BUILD_TYPE);
    Serial.println("================================");
}

/**
 * @brief Test ve pilot kullanimi icin desteklenen UART komutlarini listeler.
 */
static void printCommands() {
    Serial.println("Commands:");
    Serial.println("  JOY,x,y,buttons");
    Serial.println("  JOY,x,y,twist,throttle,hat,buttons");
    Serial.println("  MODE,DRIVE | MODE,ARM | MODE?");
    Serial.println("  ARM,HOME  (only in IDLE or MANUAL)");
    Serial.println("  STATE,IDLE | STATE,MANUAL | STATE,PICKUP");
    Serial.println("  STATE? | ERROR? | POWER? | RECOVER");
    Serial.println("  ZIP,EXTENDED | ZIP,POSITIONED | ZIP,SLID | ZIP,RETRACTED");
    Serial.println("================================");
}

/**
 * @brief Surus motorlarini yoneten MotionController modulunu baslatir.
 * @return Kritik surus sistemi hazirsa true, aksi halde false.
 */
static bool initializeMotionSystem() {
    if (!motionController.begin(
            motorDriver,
            AppConfig::DRIVE_MAX_PWM,
            AppConfig::DRIVE_RAMP_STEP,
            AppConfig::DRIVE_RAMP_INTERVAL_MS)) {
        Serial.println("ERROR: MotionController could not start.");
        errorManager.setError(ErrorCode::MOTOR_FAULT);
        return false;
    }

    Serial.println("MotionController started.");
    return true;
}

/**
 * @brief Lineer aktuatoru yalnizca etkinse ve pinleri atanmis ise baslatir.
 *
 * Lineer aktuator opsiyonel kabul edilir; baslatilamamasi ana surus sistemini
 * tek basina gecersiz kilmaz. Runtime bayragi zipline ve finish modullerine
 * guvenli sekilde aktarilir.
 */
static void initializeLinearActuator() {
    Runtime::linearActuatorAvailable = false;

    if (!RobotConfig::USE_LINEAR_ACTUATOR) {
        Serial.println("LinearActuator disabled.");
        return;
    }

    if (!pinsAssigned(Pins::LINEAR_ACTUATOR_PINS)) {
        Serial.println("WARNING: LinearActuator pins are not assigned.");
        return;
    }

    Runtime::linearActuatorAvailable = linearActuator.begin();
    Serial.println(Runtime::linearActuatorAvailable
                       ? "LinearActuator started."
                       : "WARNING: LinearActuator could not start.");
}

/**
 * @brief RFID okuyucuyu proje ayarina gore baslatir.
 */
static void initializeRfid() {
    Runtime::rfidAvailable = false;

    if (!RobotConfig::USE_RFID) {
        Serial.println("RFID disabled.");
        return;
    }

    Runtime::rfidAvailable = rfid.begin();
    if (!Runtime::rfidAvailable) {
        Serial.println("WARNING: RFID could not start.");
        return;
    }

    Serial.println("RFID started.");
}

/**
 * @brief Depolama slotlarini RobotConfig::SLOT_COUNT ile olusturur.
 * @return Slot yonetimi hazirsa true.
 */
static bool initializeSlotManager() {
    if (!slotManager.begin(RobotConfig::SLOT_COUNT)) {
        Serial.println("ERROR: SlotManager could not start.");
        errorManager.setError(ErrorCode::UNKNOWN_ERROR);
        return false;
    }

    Serial.print("SlotManager started. Slot count: ");
    Serial.println(slotManager.slotCount());
    return true;
}

/**
 * @brief OLED ekranini proje ayarina gore baslatir.
 */
static void initializeOled() {
    if (!RobotConfig::USE_OLED) {
        Serial.println("OLED disabled.");
        return;
    }

    Serial.println(oled.begin() ? "OLED started." : "WARNING: OLED could not start.");
}

/**
 * @brief Batarya gerilimi izleme modulunu baslatir ve ilk olcumu raporlar.
 */
static void initializePowerMonitoring() {
    Runtime::powerMonitoringAvailable = false;

    if (!RobotConfig::USE_POWER_MONITORING) {
        Serial.println("Power monitoring disabled.");
        return;
    }

    Runtime::powerMonitoringAvailable = powerManager.begin();
    if (!Runtime::powerMonitoringAvailable) {
        Serial.println("WARNING: PowerManager could not start. Check Pins::BATTERY_ADC.");
        return;
    }

    Serial.print("Battery voltage: ");
    Serial.print(powerManager.readBatteryVoltage(), 2);
    Serial.println(" V");
}

/**
 * @brief ServoDriver, ServoManager ve RobotArm modullerini dogru sirayla baslatir.
 *
 * Bagimlilik sirasi kritiktir:
 *   1. ServoDriver fiziksel PWM cikislarini hazirlar.
 *   2. ServoManager eklem limitlerini ve hedef acilari yonetir.
 *   3. RobotArm joystick verisini eklem hareketlerine cevirir.
 */
static void initializeServoSystem() {
    Runtime::servoSystemAvailable = false;

    if (!RobotConfig::USE_SERVO_SYSTEM) {
        Serial.println("Servo system disabled.");
        return;
    }

    const bool driverReady = servoDriver.begin();
    const bool managerReady = driverReady && servoManager.begin(
        servoDriver,
        AppConfig::ARM_JOINTS,
        ServoManager::JOINT_COUNT
    );
    const bool armReady = managerReady && robotArm.begin(
        joystick,
        servoManager,
        AppConfig::ARM_CONFIG
    );

    Runtime::servoSystemAvailable = armReady;
    if (!Runtime::servoSystemAvailable) {
        Serial.println("WARNING: Servo system could not start.");
        return;
    }

    // Guvenlik: Acilista robot kolu otomatik olarak home konumuna gitmez.
    // Servo hornlari fiziksel konumla uyusmuyorsa otomatik hareket mekanik
    // carpismaya, sikismaya ve yuksek akima neden olabilir.
    Serial.println("Servo system and RobotArm started.");
    Serial.println("WARNING: Automatic arm home movement is disabled.");
    Serial.println("Use ARM,HOME only after checking mechanical clearance.");
    Serial.println("FIRST TEST: Disconnect servo horns/links and remove the load.");
}

/**
 * @brief Yarisi tamamlayan Finish modulunu baslatir.
 *
 * Opsiyonel moduller hazir degilse nullptr verilir. Finish bu durumda yalnizca
 * mevcut donanimlari kullanmalidir.
 */
static bool initializeFinish() {
    if (!finish.begin(
            motionController,
            &slotManager,
            Runtime::linearActuatorAvailable ? &linearActuator : nullptr,
            oled.ready() ? &oled : nullptr,
            &Serial)) {
        Serial.println("ERROR: Finish module could not start.");
        errorManager.setError(ErrorCode::UNKNOWN_ERROR);
        return false;
    }

    Serial.println("Finish module started.");
    return true;
}

/**
 * @brief Tum robot modullerini bagimlilik sirasina gore baslatir.
 * @return Yalnizca kritik moduller hazirsa true.
 */
static bool initializeRobotModules() {
    errorManager.clearError();
    bool criticalModulesReady = true;

    // Surus sistemi olmadan robot guvenli sekilde gorev yapamaz.
    criticalModulesReady &= initializeMotionSystem();
    // Asagidaki moduller opsiyoneldir; kullanilabilirlikleri Runtime bayraklarinda tutulur.
    initializeLinearActuator();
    initializeRfid();
    // Slot mantigi gorev akisinin temel parcasi oldugu icin kritik kabul edilir.
    criticalModulesReady &= initializeSlotManager();
    initializeOled();
    initializePowerMonitoring();
    initializeServoSystem();
    criticalModulesReady &= initializeFinish();

    return criticalModulesReady;
}

/** @brief Mevcut gorev durumunu UART uzerinden bildirir. */
static void reportCurrentState() {
    uartLink.sendLine("STATE,", robotStateName(missionManager.currentState()));
}

/** @brief Kayitli son hata kodunu UART uzerinden bildirir. */
static void reportCurrentError() {
    uartLink.sendLine("ERROR,", errorCodeName(errorManager.getCurrentError()));
}

/** @brief Aktif DRIVE/ARM modunu UART uzerinden bildirir. */
static void reportControlMode() {
    uartLink.sendLine("MODE,", controlModeName(Runtime::controlMode));
}

/**
 * @brief Anlik batarya gerilimini ve tahmini 3S doluluk oranini bildirir.
 */
static void reportPowerStatus() {
    if (!Runtime::powerMonitoringAvailable || !powerManager.ready()) {
        uartLink.sendError("POWER_MONITOR_NOT_AVAILABLE");
        return;
    }

    char message[40];
    snprintf(
        message,
        sizeof(message),
        "POWER,%.2fV,%u%%",
        powerManager.readBatteryVoltage(),
        powerManager.estimate3sPercent()
    );
    uartLink.sendLine(message);
}

/**
 * @brief MissionManager uzerinden guvenli durum gecisi ister ve sonucu raporlar.
 */
static bool requestAndReportState(RobotState requestedState) {
    if (failSafe.isActive()) {
        uartLink.sendError("STATE_CHANGE_BLOCKED_FAILSAFE");
        return false;
    }

    if (!missionManager.requestState(requestedState)) {
        char message[64];
        snprintf(
            message,
            sizeof(message),
            "STATE_TRANSITION,%s,%s",
            robotStateName(missionManager.currentState()),
            robotStateName(requestedState)
        );
        uartLink.sendError(message);
        return false;
    }

    uartLink.sendOk("STATE,", robotStateName(requestedState));
    return true;
}

/**
 * @brief Joystick kontrolunu DRIVE veya ARM moduna gecirir.
 *
 * Mod degisiminden once hem surus hem kol komutlari durdurulur. Bu, onceki
 * moddan kalan bir hedefin yeni moda tasinmasini engeller.
 */
static bool setControlMode(ControlMode newMode) {
    if (failSafe.isActive()) {
        uartLink.sendError("MODE_CHANGE_BLOCKED_FAILSAFE");
        return false;
    }

    if (newMode == ControlMode::ARM && !Runtime::servoSystemAvailable) {
        uartLink.sendError("ARM_NOT_AVAILABLE");
        return false;
    }

    if (Runtime::controlMode == newMode) {
        return true;
    }

    motionController.stop();
    robotArm.setActive(false);

    Runtime::controlMode = newMode;
    Runtime::controlInputArmed = false;

    char message[48];
    snprintf(
        message,
        sizeof(message),
        "MODE,%s,CENTER_JOYSTICK",
        controlModeName(newMode)
    );
    uartLink.sendOk(message);

    if (oled.ready()) {
        oled.status(
            "CONTROL MODE",
            controlModeName(newMode),
            "CENTER JOYSTICK"
        );
    }

    return true;
}

/**
 * @brief Mevcut hata ortadan kalkmadan FailSafe recovery yapilmasini engeller.
 */
static bool canRecoverFromCurrentError() {
    if (!errorManager.hasError()) {
        return true;
    }

    switch (errorManager.getCurrentError()) {
        case ErrorCode::LOW_BATTERY: {
            if (!Runtime::powerMonitoringAvailable || !powerManager.ready()) {
                return false;
            }
            const float voltage = powerManager.readBatteryVoltage();
            return !isnan(voltage) && voltage >= AppConfig::RECOVERY_THRESHOLD_V;
        }

        case ErrorCode::COMMUNICATION_LOST:
            return joystick.packet().valid && !joystick.timedOut();

        default:
            return true;
    }
}

/**
 * @brief Robot kolunu acik operator komutuyla guvenli sekilde home hedefine yollar.
 *
 * Acilista otomatik cagrilmaz. Yalnizca IDLE veya MANUAL durumunda, FailSafe
 * aktif degilken ve servo sistemi hazirken calisir. Once surus durdurulur ve
 * joystick kol kontrolu yeniden merkezleme gerektirecek sekilde kilitlenir.
 */
static bool requestArmHome() {
    if (!Runtime::servoSystemAvailable || !robotArm.ready()) {
        uartLink.sendError("ARM_NOT_AVAILABLE");
        return true;
    }

    if (failSafe.isActive()) {
        uartLink.sendError("ARM_HOME_BLOCKED_FAILSAFE");
        return true;
    }

    const RobotState state = missionManager.currentState();
    if (state != RobotState::IDLE && state != RobotState::MANUAL) {
        uartLink.sendError(
            "ARM_HOME_BLOCKED_STATE,",
            robotStateName(state)
        );
        return true;
    }

    motionController.stop();
    robotArm.setActive(false);
    Runtime::controlInputArmed = false;

    robotArm.moveHome();
    uartLink.sendOk("ARM,HOME");
    return true;
}

/**
 * @brief RECOVER komutunu isler ve guvenli ise FailSafe oncesi duruma doner.
 */
static bool handleRecoveryCommand() {
    if (!failSafe.isActive() || missionManager.currentState() != RobotState::FAIL_SAFE) {
        uartLink.sendError("RECOVERY_NOT_REQUIRED");
        return true;
    }

    if (!canRecoverFromCurrentError()) {
        uartLink.sendError("RECOVERY_BLOCKED,", errorCodeName(errorManager.getCurrentError()));
        return true;
    }

    if (!missionManager.recoverFromFailSafe()) {
        uartLink.sendError("RECOVERY_FAILED");
        return true;
    }

    errorManager.clearError();
    Runtime::consecutiveLowBatterySamples = 0;
    Runtime::lowBatteryFailSafeTriggered = false;
    Runtime::communicationFailSafeTriggered = false;
    Runtime::controlMode = ControlMode::DRIVE;
    Runtime::controlInputArmed = false;
    Runtime::ziplineDriveInputArmed = false;
    robotArm.setActive(false);

    uartLink.sendOk("RECOVERED,", robotStateName(missionManager.currentState()));
    return true;
}

/**
 * @brief Joystick paketi disindaki metin tabanli UART komutlarini isler.
 * @return Komut tanindiysa true, bilinmiyorsa false.
 */
static bool handleTextCommand(const String& rawLine) {
    String command = rawLine;
    command.trim();
    command.toUpperCase();

    if (command == "STATE,IDLE") return requestAndReportState(RobotState::IDLE);
    if (command == "STATE,MANUAL") return requestAndReportState(RobotState::MANUAL);
    if (command == "STATE,PICKUP") return requestAndReportState(RobotState::PICKUP);
    if (command == "STATE?") { reportCurrentState(); return true; }
    if (command == "ERROR?") { reportCurrentError(); return true; }
    if (command == "POWER?") { reportPowerStatus(); return true; }
    if (command == "MODE?") { reportControlMode(); return true; }
    if (command == "MODE,DRIVE") return setControlMode(ControlMode::DRIVE);
    if (command == "MODE,ARM") return setControlMode(ControlMode::ARM);
    if (command == "ARM,HOME") return requestArmHome();
    if (command == "RECOVER") return handleRecoveryCommand();

    if (command == "ZIP,EXTENDED") {
        missionManager.confirmZiplineExtensionCompleted();
        uartLink.sendOk("ZIP,POSITIONING");
        return true;
    }
    if (command == "ZIP,POSITIONED") {
        missionManager.confirmZiplinePositioningCompleted();
        uartLink.sendOk("ZIP,SLIDING");
        return true;
    }
    if (command == "ZIP,SLID") {
        missionManager.confirmZiplineSlideCompleted();
        uartLink.sendOk("ZIP,RETRACTING");
        return true;
    }
    if (command == "ZIP,RETRACTED") {
        missionManager.confirmZiplineRetractionCompleted();
        uartLink.sendOk("ZIP,COMPLETED");
        return true;
    }

    return false;
}

/**
 * @brief Tum hareket cikislarini durdurur ve sistemi merkezi FailSafe durumuna alir.
 *
 * Guvenlik acisindan once fiziksel hareketler durdurulur, sonra hata kaydedilir
 * ve MissionManager FailSafe'e gecirilir.
 */
static void enterFailSafe(ErrorCode error, const char* message) {
    motionController.stop();
    robotArm.setActive(false);
    Runtime::controlInputArmed = false;
    Runtime::ziplineDriveInputArmed = false;
    linearActuator.stop();
    errorManager.setError(error);
    uartLink.sendError(message);
    missionManager.activateFailSafe();

    if (oled.ready()) {
        display.failSafe(errorCodeName(error));
    }
}

/**
 * @brief Joystick haberlesmesi kayboldugunda FailSafe'i yalnizca bir kez tetikler.
 */
static void triggerCommunicationLostFailSafe() {
    if (Runtime::communicationFailSafeTriggered) {
        return;
    }

    Runtime::communicationFailSafeTriggered = true;
    enterFailSafe(ErrorCode::COMMUNICATION_LOST, "COMMUNICATION_LOST");
}

/**
 * @brief Kalici dusuk batarya durumunda FailSafe'i yalnizca bir kez tetikler.
 */
static void triggerLowBatteryFailSafe(float voltage) {
    if (Runtime::lowBatteryFailSafeTriggered) {
        return;
    }

    Runtime::lowBatteryFailSafeTriggered = true;
    char message[32];
    snprintf(message, sizeof(message), "LOW_BATTERY,%.2fV", voltage);
    enterFailSafe(ErrorCode::LOW_BATTERY, message);
}

/**
 * @brief Batarya gerilimini periyodik olarak kontrol eder.
 *
 * Tek bir gurultulu ADC olcumuyle FailSafe'e gecmemek icin dusuk gerilim
 * art arda REQUIRED_LOW_SAMPLES kez gorulmelidir.
 */
static void updatePowerMonitoring() {
    if (!Runtime::powerMonitoringAvailable || !powerManager.ready()) {
        return;
    }

    const float voltage = powerManager.readBatteryVoltage();
    if (isnan(voltage)) {
        return;
    }

    // Gerilim esik altindaysa sayaci artir; tek olcumle karar verme.
    if (voltage <= AppConfig::LOW_BATTERY_THRESHOLD_V) {
        if (Runtime::consecutiveLowBatterySamples < AppConfig::REQUIRED_LOW_SAMPLES) {
            ++Runtime::consecutiveLowBatterySamples;
        }
        if (Runtime::consecutiveLowBatterySamples >= AppConfig::REQUIRED_LOW_SAMPLES) {
            triggerLowBatteryFailSafe(voltage);
        }
        return;
    }

    Runtime::consecutiveLowBatterySamples = 0;
}

/**
 * @brief Yeni joystick paketi geldiginde aktif moda gore hareket komutu uygular.
 *
 * IDLE durumundaki ilk gecerli joystick paketi robotu MANUAL durumuna tasir.
 * ARM modunda surus motorlari, DRIVE modunda ise robot kolu durdurulur.
 */
static void applyJoystickControl() {
    RobotState state = missionManager.currentState();

    if (failSafe.isActive() || state == RobotState::FAIL_SAFE) {
        motionController.stop();
        robotArm.setActive(false);
        Runtime::controlInputArmed = false;
        Runtime::ziplineDriveInputArmed = false;
        return;
    }

    /*
     * ZIPLINE sirasinda ARM modu kullanilmaz.
     * Button 4 (indeks 3) mevcut ZIPLINE adimini onaylar.
     */
    if (state == RobotState::ZIPLINE) {
        robotArm.setActive(false);
        Runtime::controlInputArmed = false;

        if (Runtime::controlMode != ControlMode::DRIVE) {
            motionController.stop();
            Runtime::controlMode = ControlMode::DRIVE;
            uartLink.sendOk("MODE,DRIVE,ZIPLINE");
        }

        if (joystick.buttonJustPressed(
                Constants::JoystickButtons::ZIPLINE_CONFIRM)) {
            motionController.stop();
            Runtime::ziplineDriveInputArmed = false;

            if (missionManager.confirmCurrentZiplineStep()) {
                uartLink.sendOk(
                    "ZIPLINE_CONFIRM,",
                    ziplineStateName(missionManager.currentZiplineState())
                );
            } else {
                uartLink.sendError("ZIPLINE_CONFIRM_REJECTED");
            }

            // Onay paketi ayni anda hareket komutu olarak kullanilmaz.
            return;
        }

        if (missionManager.currentZiplineState() ==
            Zipline::State::POSITIONING) {
            if (!Runtime::ziplineDriveInputArmed) {
                if (!joystickAxesCentered()) {
                    motionController.stop();
                    return;
                }

                Runtime::ziplineDriveInputArmed = true;
                uartLink.sendOk("ZIPLINE_POSITIONING_READY");
            }

            if (!motionController.driveFromJoystick(joystick)) {
                motionController.stop();
                uartLink.sendError("ZIPLINE_DRIVE_REJECTED");
            }
        } else {
            Runtime::ziplineDriveInputArmed = false;
            motionController.stop();
        }

        return;
    }

    Runtime::ziplineDriveInputArmed = false;

    // Ilk gecerli pilot komutu IDLE durumundan manuel kontrole gecisi baslatir.
    if (state == RobotState::IDLE) {
        if (!missionManager.requestState(RobotState::MANUAL)) {
            uartLink.sendError("AUTO_MANUAL_TRANSITION_FAILED");
            motionController.stop();
            robotArm.setActive(false);
            return;
        }

        state = RobotState::MANUAL;
        uartLink.sendOk("STATE,MANUAL");
    }

    const bool manualState =
        state == RobotState::MANUAL ||
        state == RobotState::PICKUP ||
        state == RobotState::DELIVERY;

    if (!manualState) {
        motionController.stop();
        robotArm.setActive(false);
        Runtime::controlInputArmed = false;
        return;
    }

    /*
     * Button 3 (indeks 2) DRIVE ve ARM arasinda gecis yapar.
     * buttonJustPressed kullanildigi icin basili tutulunca tekrar etmez.
     */
    if (joystick.buttonJustPressed(
            Constants::JoystickButtons::MODE_TOGGLE)) {
        const ControlMode nextMode =
            Runtime::controlMode == ControlMode::DRIVE
                ? ControlMode::ARM
                : ControlMode::DRIVE;

        setControlMode(nextMode);

        // Mod degistiren paket hareket komutu olarak kullanilmaz.
        return;
    }

    // Mod degisiminden sonra eksenler merkeze donmeden kontrol acilmaz.
    if (!armControlInputWhenCentered()) {
        return;
    }

    if (Runtime::controlMode == ControlMode::ARM) {
        motionController.stop();
        robotArm.setActive(true);
        return;
    }

    robotArm.setActive(false);

    if (!motionController.driveFromJoystick(joystick)) {
        motionController.stop();
        uartLink.sendError("JOYSTICK_DRIVE_REJECTED");
    }
}

/**
 * @brief UART tamponundaki tum satirlari isler ve joystick timeout kontrolu yapar.
 */
static void handleCommunication() {
    String line;

    // Normal joystick kontrolu PC'den Wi-Fi/UDP ile gelir.
    while (RobotConfig::USE_WIFI_JOYSTICK && wifiJoystickLink.readLine(line)) {
        if (RobotConfig::USE_JOYSTICK && joystick.parseLine(line)) {
            applyJoystickControl();
        }
    }

    // USB seri port test, diagnostik ve yedek metin komutlari icin korunur.
    while (uartLink.readLine(line)) {
        if (RobotConfig::USE_JOYSTICK && joystick.parseLine(line)) {
            applyJoystickControl();
            continue;
        }

        if (!handleTextCommand(line)) {
            uartLink.sendError("UNKNOWN_COMMAND,", line);
        }
    }

    // Aktif manuel kontrolde gecerli paket varken timeout olursa iletisim kaybi kabul edilir.
    if (!failSafe.isActive() &&
        joystickControlActive() &&
        joystick.packet().valid &&
        joystick.timedOut()) {
        triggerCommunicationLostFailSafe();
    }
}

/**
 * @brief Servo hedeflerini fiziksel cikislara uygular.
 *
 * ARM modunda RobotArm yeni hedef acilari hesaplar. Diger durumlarda yalnızca
 * ServoManager daha once belirlenen hedeflere yumusak gecisi surdurur.
 */
static void updateControlOutputs() {
    if (!Runtime::servoSystemAvailable) {
        return;
    }

    const RobotState state = missionManager.currentState();
    const bool armMissionState =
        state == RobotState::MANUAL ||
        state == RobotState::PICKUP ||
        state == RobotState::DELIVERY;

    if (Runtime::controlMode == ControlMode::ARM &&
        Runtime::controlInputArmed &&
        armMissionState &&
        !failSafe.isActive()) {
        robotArm.setActive(true);
        robotArm.update();
    } else {
        robotArm.setActive(false);
        servoManager.update();
    }
}

/**
 * @brief OLED ekranini mevcut durum, kontrol modu, joystick ve batarya bilgisiyle
 *        periyodik olarak yeniler.
 */
static void updateDisplay() {
    if (!oled.ready()) {
        return;
    }

    const RobotState state = missionManager.currentState();
    if (state == RobotState::FAIL_SAFE) {
        display.failSafe(errorCodeName(errorManager.getCurrentError()));
        return;
    }

    if (joystickControlActive() && joystick.packet().valid) {
        if (Runtime::controlMode == ControlMode::DRIVE) {
            display.joystick(
                joystick.packet(),
                motionController.currentLeftPwm(),
                motionController.currentRightPwm()
            );
        } else {
            oled.status("CONTROL MODE", "ARM", robotStateName(state));
        }
        return;
    }

    char thirdLine[24];

    if (Runtime::powerMonitoringAvailable && powerManager.ready()) {
        snprintf(
            thirdLine,
            sizeof(thirdLine),
            "%.2f V",
            powerManager.readBatteryVoltage()
        );
    } else {
        snprintf(
            thirdLine,
            sizeof(thirdLine),
            "MODE: %s",
            controlModeName(Runtime::controlMode)
        );
    }

    oled.status("ROBOT STATE", robotStateName(state), thirdLine);
}

/**
 * @brief Periyodik batarya ve ekran gorevlerini TaskScheduler'a kaydeder.
 */
static bool initializeScheduledTasks() {
    taskScheduler.begin();

    if (Runtime::powerMonitoringAvailable &&
        taskScheduler.addTask(
            updatePowerMonitoring,
            AppConfig::POWER_CHECK_INTERVAL_MS,
            true
        ) == TaskScheduler::INVALID_TASK_ID) {
        Serial.println("ERROR: Power monitoring task could not be added.");
        return false;
    }

    if (oled.ready() &&
        taskScheduler.addTask(
            updateDisplay,
            DISPLAY_UPDATE_INTERVAL_MS,
            true
        ) == TaskScheduler::INVALID_TASK_ID) {
        Serial.println("ERROR: Display task could not be added.");
        return false;
    }

    return true;
}

/**
 * @brief Baslatma sonucunu seri port ve OLED'e yazar; kritik hata varsa FailSafe'e
 *        gecirir.
 */
static void showStartupResult(bool systemReady) {
    if (!systemReady) {
        if (!errorManager.hasError()) {
            errorManager.setError(ErrorCode::UNKNOWN_ERROR);
        }
        missionManager.activateFailSafe();
        updateDisplay();
        Serial.println("ERROR: Critical module initialization failed.");
        return;
    }

    Serial.println("Robot initialized successfully.");
    if (oled.ready()) {
        oled.status("METEBOT READY", "STATE: IDLE", "MODE: DRIVE");
    }
}

// =============================================================================
// ARDUINO BASLANGIC NOKTASI
//
// setup() yalnizca bir kez calisir. Haberlesme, guvenlik, donanim modulleri,
// gorev yonetimi ve zamanlanmis gorevler bu sirayla baslatilir.
// =============================================================================
bool RobotApplication::begin() {
    uartLink.begin(AppConfig::SERIAL_BAUD);
    delay(300);

    printProjectInformation();

    if (RobotConfig::USE_WIFI_JOYSTICK) {
        const bool wifiStarted = wifiJoystickLink.begin(
            RobotConfig::WIFI_AP_SSID,
            RobotConfig::WIFI_AP_PASSWORD,
            RobotConfig::JOYSTICK_UDP_PORT
        );

        if (wifiStarted) {
            Serial.print("Joystick Wi-Fi started. SSID: ");
            Serial.println(RobotConfig::WIFI_AP_SSID);
            Serial.print("Robot IP: ");
            Serial.println(wifiJoystickLink.localIp());
            Serial.print("UDP port: ");
            Serial.println(RobotConfig::JOYSTICK_UDP_PORT);
        } else {
            Serial.println("ERROR: Joystick Wi-Fi could not start.");
        }
    }

    if (RobotConfig::ENABLE_WATCHDOG)
    {
        const bool hardwareWatchdogStarted =
            watchdog.begin();

        Serial.println(
            F("Software watchdog started.")
        );

        if (hardwareWatchdogStarted)
        {
            Serial.println(
                F("ESP32 Task Watchdog started.")
            );
        }
        else
        {
            Serial.println(
                F("WARNING: ESP32 Task Watchdog could not start.")
            );
        }
    }

    bool systemReady = initializeRobotModules();

    missionManager.begin();
    Serial.println("MissionManager started.");

    systemReady &= initializeScheduledTasks();
    showStartupResult(systemReady);
    printCommands();
    return systemReady;
}

// =============================================================================
// ANA CALISMA DONGUSU
//
// loop() bloklamadan ve delay kullanmadan surekli calisir. Sira bilincli olarak:
//   1. Watchdog guvenligi
//   2. Haberlesme ve pilot komutlari
//   3. Periyodik gorevler
//   4. Mission state machine
//   5. Fiziksel kontrol cikislari
// =============================================================================
void RobotApplication::update()
{
    if (RobotConfig::ENABLE_WATCHDOG)
    {
        // Onceki loop normalden uzun surduyse FailSafe'e gec.
        if (watchdog.hasTimedOut())
        {
            enterFailSafe(
                ErrorCode::UNKNOWN_ERROR,
                "SOFTWARE_WATCHDOG_TIMEOUT"
            );
        }
    }

    // Ana robot islemleri.
    handleCommunication();
    taskScheduler.update();
    missionManager.update();
    updateControlOutputs();

    if (RobotConfig::ENABLE_WATCHDOG)
    {
        // Yalnizca tum loop basariyla bittiyse besle.
        watchdog.feed();
    }
}
