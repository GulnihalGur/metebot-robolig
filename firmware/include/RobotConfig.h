#ifndef ROBOTCONFIG_H
#define ROBOTCONFIG_H

namespace RobotConfig
{
    // Hardware modules
    constexpr bool USE_OLED = true;
    constexpr bool USE_RFID = true;
    constexpr bool USE_JOYSTICK = true;
    constexpr bool USE_WIFI_JOYSTICK = true;

    // PC, Deneyap'in olusturdugu bu Wi-Fi agina baglanir.
    constexpr const char* WIFI_AP_SSID = "METEBOT";
    constexpr const char* WIFI_AP_PASSWORD = "metebot123";
    constexpr unsigned short JOYSTICK_UDP_PORT = 4210;
    constexpr bool USE_SERVO_SYSTEM = true;
    constexpr bool USE_LINEAR_ACTUATOR = true;
    constexpr bool USE_POWER_MONITORING = true;

    // Safety features
    constexpr bool ENABLE_WATCHDOG = true;
    constexpr bool ENABLE_FAILSAFE = true;
    constexpr bool ENABLE_DIAGNOSTICS = true;

    // Robot operating mode
    constexpr bool ENABLE_MANUAL_MODE = true;
    constexpr bool ENABLE_MISSION_MODE = true;

    // Drive system
    constexpr bool USE_DIFFERENTIAL_DRIVE = true;
    constexpr unsigned char DRIVE_MOTOR_COUNT = 6;

    // Mission configuration
    constexpr unsigned char CITY_COUNT = 5;
    constexpr unsigned char SLOT_COUNT = 5;

    // Debug
    constexpr bool DEBUG_MODE = true;
}

#endif