#pragma once

#include <cstdint>

namespace app::config {

// Replace these placeholders before flashing a real device.
inline constexpr const char* WIFI_SSID = "Your_SSID";
inline constexpr const char* WIFI_PASSWORD = "Your_PASSWORD";

inline constexpr uint8_t PIN_DHT11 = 4;
inline constexpr uint8_t PIN_PIR = 14;
inline constexpr uint8_t PIN_RELAY = 5;
inline constexpr uint8_t PIN_BUZZER = 12;

inline constexpr float AUTO_FAN_ON_TEMP_C = 30.0f;
inline constexpr float AUTO_FAN_ON_HUMI_PCT = 70.0f;
inline constexpr float AUTO_FAN_OFF_TEMP_C = 28.0f;
inline constexpr float AUTO_FAN_OFF_HUMI_PCT = 65.0f;

inline constexpr unsigned long SENSOR_READ_INTERVAL_MS = 2000UL;
inline constexpr unsigned long PIR_ALARM_DURATION_MS = 5000UL;
inline constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 5000UL;

inline constexpr bool RELAY_ACTIVE_LOW = true;
inline constexpr bool BUZZER_ACTIVE_HIGH = true;

}  // namespace app::config
