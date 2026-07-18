#include <Arduino.h>
#include <math.h>

#include "RobotConfig.h"
#include "Version.h"
#include "Constants.h"
#include "Types.h"
#include "Watchdog.h"

#include "core/FailSafe.h"
#include "core/StateMachine.h"
#include "core/ErrorManager.h"
#include "core/TaskScheduler.h"

#include "communication/UART.h"
#include "communication/Joystick.h"

#include "drivers/MotorDriver.h"
#include "drivers/LinearActuator.h"
#include "drivers/RFID.h"
#include "drivers/OLED.h"
#include "drivers/PowerManager.h"

#include "control/MotionController.h"
#include "control/SlotManager.h"

#include "mission/Zipline.h"
#include "mission/Finish.h"
#include "mission/MissionManager.h"

#include "ui/Display.h"


// =====================================================
// CORE
// =====================================================

Watchdog watchdog;
FailSafe failSafe;
StateMachine stateMachine;
ErrorManager errorManager;

/*
 * Periyodik görevleri delay() kullanmadan çalıştırır.
 *
 * Bu projede şimdilik:
 * - Batarya kontrolü
 * - OLED ekran güncellemesi
 *
 * TaskScheduler tarafından yönetilir.
 */
TaskScheduler taskScheduler;


// =====================================================
// COMMUNICATION
// =====================================================

UARTLink uartLink(Serial);
Joystick joystick;


// =====================================================
// DRIVERS
// =====================================================

MotorDriver motorDriver;
LinearActuator linearActuator;
RFID rfid;
OLED oled;
PowerManager powerManager;


// =====================================================
// CONTROL / UI
// =====================================================

MotionController motionController;
SlotManager slotManager;
Display display(oled);


// =====================================================
// MISSION MODULES
// =====================================================

Zipline zipline(
    motionController,
    linearActuator,
    failSafe
);

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


// =====================================================
// POWER CONFIGURATION
// =====================================================

namespace PowerConfig
{
    /*
     * TaskScheduler batarya kontrol fonksiyonunu
     * her 1000 ms'de bir çağırır.
     */
    constexpr uint32_t CHECK_INTERVAL_MS = 1000;

    // 3S Li-Po düşük batarya sınırı.
    constexpr float LOW_BATTERY_THRESHOLD_V = 10.5f;

    // FailSafe'den çıkmak için gereken minimum voltaj.
    constexpr float RECOVERY_THRESHOLD_V = 11.1f;

    /*
     * Tek bir anlık voltaj düşüşünde FailSafe oluşturulmaz.
     * Arka arkaya üç düşük ölçüm görülmesi gerekir.
     */
    constexpr uint8_t REQUIRED_LOW_SAMPLES = 3;
}


// =====================================================
// RUNTIME STATE
// =====================================================

static uint8_t consecutiveLowBatterySamples = 0;

static bool powerMonitoringAvailable = false;
static bool lowBatteryFailSafeTriggered = false;
static bool communicationFailSafeTriggered = false;


// =====================================================
// HELPERS
// =====================================================

static const char* robotStateName(RobotState state)
{
    switch (state)
    {
        case RobotState::IDLE:
            return "IDLE";

        case RobotState::MANUAL:
            return "MANUAL";

        case RobotState::PICKUP:
            return "PICKUP";

        case RobotState::DELIVERY:
            return "DELIVERY";

        case RobotState::ZIPLINE:
            return "ZIPLINE";

        case RobotState::FINISH:
            return "FINISH";

        case RobotState::FAIL_SAFE:
            return "FAIL_SAFE";
    }

    return "UNKNOWN";
}


static const char* errorCodeName(ErrorCode errorCode)
{
    switch (errorCode)
    {
        case ErrorCode::NONE:
            return "NONE";

        case ErrorCode::COMMUNICATION_LOST:
            return "COMMUNICATION LOST";

        case ErrorCode::RFID_READ_FAILED:
            return "RFID READ FAILED";

        case ErrorCode::MOTOR_FAULT:
            return "MOTOR FAULT";

        case ErrorCode::SERVO_FAULT:
            return "SERVO FAULT";

        case ErrorCode::LOW_BATTERY:
            return "LOW BATTERY";

        case ErrorCode::UNKNOWN_ERROR:
            return "UNKNOWN ERROR";
    }

    return "UNKNOWN ERROR";
}


/*
 * Joystick ile sürüş yapılmasına izin verilen
 * robot durumlarını kontrol eder.
 */
static bool isManualDriveState(RobotState state)
{
    return state == RobotState::MANUAL ||
           state == RobotState::PICKUP ||
           state == RobotState::DELIVERY;
}


static void printProjectInformation()
{
    Serial.println();
    Serial.println("================================");
    Serial.println(Version::PROJECT_NAME);
    Serial.println(Version::COMPETITION_NAME);

    Serial.print("Software Version: ");
    Serial.println(Version::SOFTWARE_VERSION);

    Serial.print("Build Type: ");
    Serial.println(Version::BUILD_TYPE);

    Serial.println("System starting...");
    Serial.println("================================");
}


// =====================================================
// MODULE INITIALIZATION
// =====================================================

static bool initializeRobotModules()
{
    bool initializationSuccessful = true;

    errorManager.clearError();

    // -------------------------------------------------
    // MotionController
    // -------------------------------------------------

    if (!motionController.begin(motorDriver))
    {
        Serial.println(
            "ERROR: MotionController could not start."
        );

        errorManager.setError(
            ErrorCode::MOTOR_FAULT
        );

        initializationSuccessful = false;
    }
    else
    {
        Serial.println(
            "MotionController started."
        );
    }

    // -------------------------------------------------
    // LinearActuator
    // -------------------------------------------------

    if (!linearActuator.begin())
    {
        Serial.println(
            "WARNING: LinearActuator could not start."
        );

        initializationSuccessful = false;
    }
    else
    {
        Serial.println(
            "LinearActuator started."
        );
    }

    // -------------------------------------------------
    // RFID
    // -------------------------------------------------

    if (!rfid.begin())
    {
        Serial.println(
            "WARNING: RFID could not start."
        );

        errorManager.setError(
            ErrorCode::RFID_READ_FAILED
        );

        initializationSuccessful = false;
    }
    else
    {
        Serial.println("RFID started.");
    }

    // -------------------------------------------------
    // SlotManager
    // -------------------------------------------------

    if (!slotManager.begin(
            SlotManager::DEFAULT_SLOT_COUNT
        ))
    {
        Serial.println(
            "ERROR: SlotManager could not start."
        );

        errorManager.setError(
            ErrorCode::UNKNOWN_ERROR
        );

        initializationSuccessful = false;
    }
    else
    {
        Serial.print(
            "SlotManager started. Slot count: "
        );

        Serial.println(
            slotManager.slotCount()
        );
    }

    // -------------------------------------------------
    // OLED
    // -------------------------------------------------

    if (!oled.begin())
    {
        /*
         * OLED kritik güvenlik donanımı değildir.
         * Ekran başlamasa bile robot çalışmaya devam eder.
         */
        Serial.println(
            "WARNING: OLED could not start."
        );
    }
    else
    {
        Serial.println("OLED started.");
    }

    // -------------------------------------------------
    // PowerManager
    // -------------------------------------------------

    if (RobotConfig::USE_POWER_MONITORING)
    {
        if (!powerManager.begin())
        {
            powerMonitoringAvailable = false;

            Serial.println(
                "WARNING: PowerManager could not start."
            );

            Serial.println(
                "WARNING: Check Pins::BATTERY_ADC."
            );
        }
        else
        {
            powerMonitoringAvailable = true;

            Serial.println(
                "PowerManager started."
            );

            const float batteryVoltage =
                powerManager.readBatteryVoltage();

            Serial.print(
                "Battery voltage: "
            );

            Serial.print(
                batteryVoltage,
                2
            );

            Serial.println(" V");
        }
    }
    else
    {
        powerMonitoringAvailable = false;

        Serial.println(
            "Power monitoring disabled."
        );
    }

    // -------------------------------------------------
    // Finish
    // -------------------------------------------------

    if (!finish.begin(
            motionController,
            &slotManager,
            &linearActuator,
            &oled,
            &Serial
        ))
    {
        Serial.println(
            "ERROR: Finish module could not start."
        );

        errorManager.setError(
            ErrorCode::UNKNOWN_ERROR
        );

        initializationSuccessful = false;
    }
    else
    {
        Serial.println(
            "Finish module started."
        );
    }

    return initializationSuccessful;
}


// =====================================================
// STATUS REPORTING
// =====================================================

static void reportCurrentState()
{
    uartLink.sendLine(
        String("STATE,") +
        robotStateName(
            missionManager.currentState()
        )
    );
}


static void reportCurrentError()
{
    uartLink.sendLine(
        String("ERROR,") +
        errorCodeName(
            errorManager.getCurrentError()
        )
    );
}


static void reportPowerStatus()
{
    if (!powerMonitoringAvailable ||
        !powerManager.ready())
    {
        uartLink.sendError(
            "POWER_MONITOR_NOT_AVAILABLE"
        );

        return;
    }

    const float batteryVoltage =
        powerManager.readBatteryVoltage();

    const uint8_t batteryPercent =
        powerManager.estimate3sPercent();

    uartLink.sendLine(
        String("POWER,") +
        String(batteryVoltage, 2) +
        "V," +
        String(batteryPercent) +
        "%"
    );
}


// =====================================================
// STATE COMMANDS
// =====================================================

static bool requestAndReportState(
    RobotState requestedState
)
{
    if (failSafe.isActive())
    {
        uartLink.sendError(
            "STATE_CHANGE_BLOCKED_FAILSAFE"
        );

        return false;
    }

    if (!missionManager.requestState(
            requestedState
        ))
    {
        uartLink.sendError(
            String("STATE_TRANSITION,") +
            robotStateName(
                missionManager.currentState()
            ) +
            "," +
            robotStateName(
                requestedState
            )
        );

        return false;
    }

    uartLink.sendOk(
        String("STATE,") +
        robotStateName(requestedState)
    );

    if (oled.ready())
    {
        oled.status(
            "STATE CHANGED",
            robotStateName(requestedState)
        );
    }

    return true;
}


// =====================================================
// RECOVERY
// =====================================================

static bool canRecoverFromCurrentError()
{
    if (!errorManager.hasError())
    {
        return true;
    }

    const ErrorCode currentError =
        errorManager.getCurrentError();

    // -------------------------------------------------
    // LOW BATTERY
    // -------------------------------------------------

    if (currentError == ErrorCode::LOW_BATTERY)
    {
        if (!powerMonitoringAvailable ||
            !powerManager.ready())
        {
            return false;
        }

        const float batteryVoltage =
            powerManager.readBatteryVoltage();

        return !isnan(batteryVoltage) &&
               batteryVoltage >=
                   PowerConfig::RECOVERY_THRESHOLD_V;
    }

    // -------------------------------------------------
    // COMMUNICATION LOST
    // -------------------------------------------------

    if (
        currentError ==
        ErrorCode::COMMUNICATION_LOST
    )
    {
        /*
         * Joystick bağlantısı yeniden kurulmadan
         * FailSafe temizlenemez.
         */
        return joystick.packet().valid &&
               !joystick.timedOut();
    }

    /*
     * Diğer hatalarda pilot fiziksel sorunu giderdikten
     * sonra RECOVER komutunu kullanabilir.
     */
    return true;
}


static bool handleRecoveryCommand()
{
    if (!failSafe.isActive() ||
        missionManager.currentState() !=
            RobotState::FAIL_SAFE)
    {
        uartLink.sendError(
            "RECOVERY_NOT_REQUIRED"
        );

        return true;
    }

    if (!canRecoverFromCurrentError())
    {
        uartLink.sendError(
            String("RECOVERY_BLOCKED,") +
            errorCodeName(
                errorManager.getCurrentError()
            )
        );

        if (oled.ready())
        {
            display.failSafe(
                errorCodeName(
                    errorManager.getCurrentError()
                )
            );
        }

        return true;
    }

    if (!missionManager.recoverFromFailSafe())
    {
        uartLink.sendError(
            "RECOVERY_FAILED"
        );

        return true;
    }

    errorManager.clearError();

    consecutiveLowBatterySamples = 0;
    lowBatteryFailSafeTriggered = false;
    communicationFailSafeTriggered = false;

    uartLink.sendOk(
        String("RECOVERED,") +
        robotStateName(
            missionManager.currentState()
        )
    );

    if (oled.ready())
    {
        oled.status(
            "FAILSAFE CLEARED",
            robotStateName(
                missionManager.currentState()
            ),
            "DATA PRESERVED"
        );
    }

    return true;
}


// =====================================================
// COMMAND HANDLER
// =====================================================

static bool handleStateCommand(
    const String& line
)
{
    String command = line;

    command.trim();
    command.toUpperCase();

    if (command == "STATE,IDLE")
    {
        return requestAndReportState(
            RobotState::IDLE
        );
    }

    if (command == "STATE,MANUAL")
    {
        return requestAndReportState(
            RobotState::MANUAL
        );
    }

    if (command == "STATE,PICKUP")
    {
        return requestAndReportState(
            RobotState::PICKUP
        );
    }

    if (command == "RECOVER")
    {
        return handleRecoveryCommand();
    }

    if (command == "STATE?")
    {
        reportCurrentState();
        return true;
    }

    if (command == "ERROR?")
    {
        reportCurrentError();
        return true;
    }

    if (command == "POWER?")
    {
        reportPowerStatus();
        return true;
    }

    return false;
}


// =====================================================
// FAILSAFE TRIGGERS
// =====================================================

static void triggerCommunicationLostFailSafe()
{
    /*
     * Aynı bağlantı kaybı için FailSafe'in her loop
     * turunda yeniden tetiklenmesini engeller.
     */
    if (communicationFailSafeTriggered)
    {
        return;
    }

    communicationFailSafeTriggered = true;

    /*
     * Durum değişikliği beklenmeden motorlar
     * hemen durdurulur.
     */
    motionController.stop();

    errorManager.setError(
        ErrorCode::COMMUNICATION_LOST
    );

    Serial.println(
        "ERROR: Joystick communication lost."
    );

    uartLink.sendError(
        "COMMUNICATION_LOST"
    );

    missionManager.activateFailSafe();

    if (oled.ready())
    {
        display.failSafe(
            "COMMUNICATION LOST"
        );
    }
}


static void triggerLowBatteryFailSafe(
    float batteryVoltage
)
{
    if (lowBatteryFailSafeTriggered)
    {
        return;
    }

    lowBatteryFailSafeTriggered = true;

    motionController.stop();

    errorManager.setError(
        ErrorCode::LOW_BATTERY
    );

    Serial.print(
        "ERROR: Low battery. Voltage: "
    );

    Serial.print(
        batteryVoltage,
        2
    );

    Serial.println(" V");

    uartLink.sendError(
        String("LOW_BATTERY,") +
        String(batteryVoltage, 2) +
        "V"
    );

    missionManager.activateFailSafe();

    if (oled.ready())
    {
        oled.status(
            "FAIL SAFE",
            "LOW BATTERY",
            String(batteryVoltage, 2) +
            " V"
        );
    }
}


// =====================================================
// SCHEDULED TASK: POWER MONITORING
// =====================================================

/*
 * Bu fonksiyon artık loop() içinde doğrudan çağrılmaz.
 *
 * TaskScheduler tarafından
 * PowerConfig::CHECK_INTERVAL_MS aralığında çalıştırılır.
 *
 * Bu nedenle fonksiyonun içinde ayrıca millis()
 * kontrolü yapılmasına gerek yoktur.
 */
static void updatePowerMonitoring()
{
    if (!RobotConfig::USE_POWER_MONITORING ||
        !powerMonitoringAvailable ||
        !powerManager.ready())
    {
        return;
    }

    const float batteryVoltage =
        powerManager.readBatteryVoltage();

    if (isnan(batteryVoltage))
    {
        return;
    }

    if (
        batteryVoltage <=
        PowerConfig::LOW_BATTERY_THRESHOLD_V
    )
    {
        if (
            consecutiveLowBatterySamples <
            PowerConfig::REQUIRED_LOW_SAMPLES
        )
        {
            ++consecutiveLowBatterySamples;
        }

        if (
            consecutiveLowBatterySamples >=
            PowerConfig::REQUIRED_LOW_SAMPLES
        )
        {
            triggerLowBatteryFailSafe(
                batteryVoltage
            );
        }

        return;
    }

    /*
     * Voltaj normale döndüğünde yalnızca düşük ölçüm
     * sayacı sıfırlanır.
     *
     * Aktif bir FailSafe otomatik olarak kaldırılmaz.
     * Pilotun RECOVER komutu göndermesi gerekir.
     */
    consecutiveLowBatterySamples = 0;
}


// =====================================================
// COMMUNICATION
// =====================================================

static void handleCommunication()
{
    String line;

    while (uartLink.readLine(line))
    {
        /*
         * Gelen satır önce joystick paketi olarak
         * yorumlanır.
         */
        if (joystick.parseLine(line))
        {
            const RobotState currentState =
                missionManager.currentState();

            /*
             * FailSafe sırasında joystick paketleri alınmaya
             * devam eder. Böylece bağlantının geri geldiği
             * anlaşılabilir.
             *
             * RECOVER komutu verilmeden motor çalıştırılmaz.
             */
            if (
                failSafe.isActive() ||
                currentState ==
                    RobotState::FAIL_SAFE
            )
            {
                motionController.stop();
                continue;
            }

            RobotState state = currentState;

            /*
             * İlk geçerli joystick paketi robotu
             * IDLE durumundan MANUAL durumuna geçirir.
             */
            if (state == RobotState::IDLE)
            {
                if (!missionManager.requestState(
                        RobotState::MANUAL
                    ))
                {
                    uartLink.sendError(
                        "AUTO_MANUAL_TRANSITION_FAILED"
                    );

                    motionController.stop();
                    continue;
                }

                state = RobotState::MANUAL;

                uartLink.sendOk(
                    "STATE,MANUAL"
                );
            }

            if (isManualDriveState(state))
            {
                if (!motionController.driveFromJoystick(
                        joystick
                    ))
                {
                    motionController.stop();

                    uartLink.sendError(
                        "JOYSTICK_DRIVE_REJECTED"
                    );
                }
            }
            else
            {
                /*
                 * ZIPLINE ve FINISH durumlarında joystick
                 * sürüş komutları kabul edilmez.
                 */
                motionController.stop();
            }

            continue;
        }

        if (handleStateCommand(line))
        {
            continue;
        }

        uartLink.sendError(
            String("UNKNOWN_COMMAND,") +
            line
        );
    }

    /*
     * FailSafe zaten aktifse yeniden bağlantı kaybı
     * kontrolü yapılmaz.
     */
    if (failSafe.isActive())
    {
        return;
    }

    /*
     * Robot sürüş durumundayken joystick zaman aşımına
     * uğrarsa iletişim kaynaklı FailSafe oluşturulur.
     *
     * Bu kontrol hızlı tepki vermesi gerektiği için
     * TaskScheduler'a bağlanmadı ve her loop turunda
     * çalışmaya devam ediyor.
     */
    if (
        isManualDriveState(
            missionManager.currentState()
        ) &&
        joystick.packet().valid &&
        joystick.timedOut()
    )
    {
        triggerCommunicationLostFailSafe();
    }
}


// =====================================================
// SCHEDULED TASK: DISPLAY
// =====================================================

/*
 * Bu fonksiyon TaskScheduler tarafından
 * DISPLAY_UPDATE_INTERVAL_MS aralığında çağrılır.
 *
 * Fonksiyonun içinde ayrıca millis() kontrolü yoktur.
 */
static void updateDisplay()
{
    if (!oled.ready())
    {
        return;
    }

    const RobotState state =
        missionManager.currentState();

    /*
     * FailSafe ekranının normal durum ekranı tarafından
     * ezilmesini önler.
     */
    if (state == RobotState::FAIL_SAFE)
    {
        if (errorManager.hasError())
        {
            display.failSafe(
                errorCodeName(
                    errorManager.getCurrentError()
                )
            );
        }
        else
        {
            display.failSafe(
                "UNKNOWN ERROR"
            );
        }

        return;
    }

    /*
     * Robot manuel sürüş durumundaysa joystick ve
     * motor PWM bilgileri ekranda gösterilir.
     */
    if (
        isManualDriveState(state) &&
        joystick.packet().valid
    )
    {
        display.joystick(
            joystick.packet(),
            motionController.currentLeftPwm(),
            motionController.currentRightPwm()
        );

        return;
    }

    /*
     * Batarya ölçümü kullanılabiliyorsa robot durumu
     * ile birlikte voltaj da gösterilir.
     */
    if (
        powerMonitoringAvailable &&
        powerManager.ready()
    )
    {
        const float batteryVoltage =
            powerManager.readBatteryVoltage();

        oled.status(
            "ROBOT STATE",
            robotStateName(state),
            String(batteryVoltage, 2) +
            " V"
        );

        return;
    }

    oled.status(
        "ROBOT STATE",
        robotStateName(state),
        "SYSTEM READY"
    );
}


// =====================================================
// TASK SCHEDULER INITIALIZATION
// =====================================================

/*
 * Periyodik görevleri TaskScheduler'a kaydeder.
 *
 * Şimdilik iki görev bulunur:
 *
 * 1. Batarya kontrolü
 * 2. OLED ekran güncellemesi
 *
 * Başarılı olursa true döndürür.
 */
static bool initializeScheduledTasks()
{
    taskScheduler.begin();

    // -------------------------------------------------
    // Power monitoring task
    // -------------------------------------------------

    const int8_t powerTaskId =
        taskScheduler.addTask(
            updatePowerMonitoring,
            PowerConfig::CHECK_INTERVAL_MS,

            /*
             * true:
             * Scheduler'ın ilk update() çağrısında
             * batarya kontrolü hemen yapılır.
             */
            true
        );

    if (
        powerTaskId ==
        TaskScheduler::INVALID_TASK_ID
    )
    {
        Serial.println(
            "ERROR: Power monitoring task could not be added."
        );

        return false;
    }

    Serial.print(
        "Power monitoring task added. Task ID: "
    );

    Serial.println(powerTaskId);

    // -------------------------------------------------
    // Display update task
    // -------------------------------------------------

    const int8_t displayTaskId =
        taskScheduler.addTask(
            updateDisplay,
            DISPLAY_UPDATE_INTERVAL_MS,

            /*
             * true:
             * OLED ilk scheduler güncellemesinde
             * hemen yenilenir.
             */
            true
        );

    if (
        displayTaskId ==
        TaskScheduler::INVALID_TASK_ID
    )
    {
        Serial.println(
            "ERROR: Display task could not be added."
        );

        /*
         * İkinci görev eklenemezse daha önce eklenen
         * batarya görevi de kaldırılır.
         */
        taskScheduler.removeTask(
            static_cast<uint8_t>(powerTaskId)
        );

        return false;
    }

    Serial.print(
        "Display task added. Task ID: "
    );

    Serial.println(displayTaskId);

    Serial.print(
        "TaskScheduler started. Task count: "
    );

    Serial.println(
        taskScheduler.getTaskCount()
    );

    return true;
}


// =====================================================
// ARDUINO SETUP
// =====================================================

void setup()
{
    uartLink.begin(115200);
    delay(500);

    printProjectInformation();

    watchdog.begin();

    Serial.println(
        "Watchdog started."
    );

    /*
     * Robotun donanım ve görev modülleri başlatılır.
     */
    bool systemReady =
        initializeRobotModules();

    missionManager.begin();

    Serial.println(
        "MissionManager started."
    );

    /*
     * Periyodik görevler TaskScheduler'a eklenir.
     */
    const bool schedulerReady =
        initializeScheduledTasks();

    if (!schedulerReady)
    {
        Serial.println(
            "ERROR: TaskScheduler initialization failed."
        );

        errorManager.setError(
            ErrorCode::UNKNOWN_ERROR
        );

        systemReady = false;
    }

    if (!systemReady)
    {
        Serial.println(
            "ERROR: One or more modules could not start."
        );

        if (!errorManager.hasError())
        {
            errorManager.setError(
                ErrorCode::UNKNOWN_ERROR
            );
        }

        missionManager.activateFailSafe();

        if (oled.ready())
        {
            display.failSafe(
                errorCodeName(
                    errorManager.getCurrentError()
                )
            );
        }
    }
    else
    {
        Serial.println(
            "Robot initialized successfully."
        );

        if (oled.ready())
        {
            if (
                powerMonitoringAvailable &&
                powerManager.ready()
            )
            {
                oled.status(
                    "METEBOT READY",
                    "STATE: IDLE",
                    String(
                        powerManager
                            .readBatteryVoltage(),
                        2
                    ) +
                    " V"
                );
            }
            else
            {
                oled.status(
                    "METEBOT READY",
                    "STATE: IDLE",
                    "POWER MONITOR OFF"
                );
            }
        }
    }

    Serial.println("Commands:");

    Serial.println(
        "  JOY,x,y,buttons"
    );

    Serial.println(
        "  JOY,x,y,twist,throttle,hat,buttons"
    );

    Serial.println(
        "  STATE,IDLE | STATE,MANUAL | STATE,PICKUP"
    );

    Serial.println(
        "  STATE?"
    );

    Serial.println(
        "  ERROR?"
    );

    Serial.println(
        "  POWER?"
    );

    Serial.println(
        "  RECOVER"
    );

    Serial.println(
        "================================"
    );
}


// =====================================================
// ARDUINO LOOP
// =====================================================

void loop()
{
    // -------------------------------------------------
    // Watchdog
    // -------------------------------------------------

    if (watchdog.hasTimedOut())
    {
        Serial.println(
            "ERROR: Watchdog timeout!"
        );

        if (!errorManager.hasError())
        {
            errorManager.setError(
                ErrorCode::UNKNOWN_ERROR
            );
        }

        missionManager.activateFailSafe();
    }
    else
    {
        watchdog.feed();
    }

    /*
     * UART ve joystick haberleşmesi mümkün olduğunca
     * hızlı çalışmalıdır. Bu nedenle scheduler'a bağlı
     * değildir ve her loop turunda çağrılır.
     */
    handleCommunication();

    /*
     * Batarya ve ekran gibi zamanlanmış görevleri
     * gereken aralıklarla çalıştırır.
     */
    taskScheduler.update();

    /*
     * MissionManager durum makinesinin gecikmeden
     * ilerlemesi için her loop turunda güncellenir.
     */
    missionManager.update();
}