#include <Arduino.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WebServer.h>
#include <WiFi.h>

#include "app_config.h"

namespace {

// =========================
// 基础状态与全局对象
// =========================
enum class FanMode {
  Auto,
  ManualOn,
  ManualOff,
};

// 运行时状态快照，供接口返回和控制逻辑共用。
struct RuntimeState {
  float temperature_c = 0.0f;
  float humidity_pct = 0.0f;
  FanMode mode = FanMode::Auto;
  bool fan_state = false;
  bool intrusion_alert = false;
  unsigned long intrusion_alert_until_ms = 0;
};

constexpr int kDhtType = DHT11;

WebServer server(80);
DHT dht(app::config::PIN_DHT11, kDhtType);
RuntimeState state;

volatile bool pir_interrupt_flag = false;
unsigned long last_sensor_read_ms = 0;
unsigned long last_wifi_retry_ms = 0;

// =========================
// 状态转换与底层控制工具
// =========================
String modeToString(FanMode mode) {
  switch (mode) {
    case FanMode::Auto:
      return "AUTO";
    case FanMode::ManualOn:
      return "MANUAL_ON";
    case FanMode::ManualOff:
      return "MANUAL_OFF";
  }
  return "AUTO";
}

FanMode modeFromCommand(const String& cmd) {
  if (cmd == "on") {
    return FanMode::ManualOn;
  }
  if (cmd == "off") {
    return FanMode::ManualOff;
  }
  return FanMode::Auto;
}

bool timeReached(unsigned long now, unsigned long deadline) {
  return static_cast<long>(now - deadline) >= 0;
}

void setRelay(bool on) {
  state.fan_state = on;
  const int level = on ? (app::config::RELAY_ACTIVE_LOW ? LOW : HIGH)
                       : (app::config::RELAY_ACTIVE_LOW ? HIGH : LOW);
  digitalWrite(app::config::PIN_RELAY, level);
}

void setBuzzer(bool on) {
  const int level = on ? (app::config::BUZZER_ACTIVE_HIGH ? HIGH : LOW)
                       : (app::config::BUZZER_ACTIVE_HIGH ? LOW : HIGH);
  digitalWrite(app::config::PIN_BUZZER, level);
}

void updateFanLogic() {
  // 手动模式优先，AUTO 按阈值回差逻辑控制。
  switch (state.mode) {
    case FanMode::Auto: {
      const bool autoShouldRun =
          state.temperature_c >= app::config::AUTO_FAN_ON_TEMP_C ||
          state.humidity_pct >= app::config::AUTO_FAN_ON_HUMI_PCT;
      const bool autoShouldStop =
          state.temperature_c <= app::config::AUTO_FAN_OFF_TEMP_C &&
          state.humidity_pct <= app::config::AUTO_FAN_OFF_HUMI_PCT;

      if (autoShouldRun) {
        setRelay(true);
      } else if (autoShouldStop) {
        setRelay(false);
      }
      break;
    }
    case FanMode::ManualOn:
      setRelay(true);
      break;
    case FanMode::ManualOff:
      setRelay(false);
      break;
  }
}

void updateAlarmLogic() {
  const unsigned long now = millis();
  // 报警按时间关闭，避免主循环阻塞。
  if (state.intrusion_alert && timeReached(now, state.intrusion_alert_until_ms)) {
    state.intrusion_alert = false;
    setBuzzer(false);
  } else if (state.intrusion_alert) {
    setBuzzer(true);
  } else {
    setBuzzer(false);
  }
}

void triggerIntrusionAlarm() {
  // 外部中断里只置标志位，真正处理放到主循环里做。
  state.intrusion_alert = true;
  state.intrusion_alert_until_ms = millis() + app::config::PIR_ALARM_DURATION_MS;
  setBuzzer(true);
}

bool readSensors() {
  const float humidity = dht.readHumidity();
  const float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("[WARN] DHT11 read failed");
    return false;
  }
  state.temperature_c = temperature;
  state.humidity_pct = humidity;
  return true;
}

// =========================
// HTTP 响应与接口处理
// =========================
void sendCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  sendCorsHeaders();
  server.send(204);
}

void handleStatus() {
  sendCorsHeaders();
  // 保持返回结构与 docs/api_protocol.md 一致。
  DynamicJsonDocument doc(256);
  doc["temperature"] = state.temperature_c;
  doc["humidity"] = state.humidity_pct;
  doc["mode"] = modeToString(state.mode);
  doc["fan_state"] = state.fan_state;
  doc["intrusion_alert"] = state.intrusion_alert;

  String payload;
  serializeJson(doc, payload);
  server.send(200, "application/json; charset=utf-8", payload);
}

void handleControl() {
  sendCorsHeaders();
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", R"({"success":false,"message":"missing body"})");
    return;
  }

  DynamicJsonDocument doc(256);
  const DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", R"({"success":false,"message":"invalid json"})");
    return;
  }

  const String cmd = doc["cmd"] | "";
  // 只接受约定的三种命令：auto / on / off。
  if (cmd != "auto" && cmd != "on" && cmd != "off") {
    server.send(400, "application/json", R"({"success":false,"message":"invalid cmd"})");
    return;
  }

  state.mode = modeFromCommand(cmd);
  updateFanLogic();

  DynamicJsonDocument resp(256);
  resp["success"] = true;
  resp["message"] = "mode updated";
  resp["mode"] = modeToString(state.mode);

  String payload;
  serializeJson(resp, payload);
  server.send(200, "application/json; charset=utf-8", payload);
}

// =========================
// 路由注册与硬件初始化
// =========================
void setupRoutes() {
  server.on("/api/status", HTTP_OPTIONS, handleOptions);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/control", HTTP_OPTIONS, handleOptions);
  server.on("/api/control", HTTP_POST, handleControl);
  server.onNotFound([]() {
    sendCorsHeaders();
    server.send(404, "application/json", R"({"success":false,"message":"not found"})");
  });
}

void setupPins() {
  // 上电默认输出安全：风扇关闭，蜂鸣器静音。
  pinMode(app::config::PIN_RELAY, OUTPUT);
  pinMode(app::config::PIN_BUZZER, OUTPUT);
  pinMode(app::config::PIN_PIR, INPUT_PULLDOWN);

  setRelay(false);
  setBuzzer(false);
}

// =========================
// 网络连接与中断处理
// =========================
void setupWiFi() {
  // 使用 STA 模式连接局域网路由器并提供 HTTP 接口。
  WiFi.mode(WIFI_STA);
  WiFi.begin(app::config::WIFI_SSID, app::config::WIFI_PASSWORD);
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  // 定时重连，避免每次循环都疯狂重试。
  const unsigned long now = millis();
  if (now - last_wifi_retry_ms < app::config::WIFI_RETRY_INTERVAL_MS) {
    return;
  }

  last_wifi_retry_ms = now;
  Serial.println("[INFO] retrying Wi-Fi connection");
  WiFi.disconnect();
  WiFi.begin(app::config::WIFI_SSID, app::config::WIFI_PASSWORD);
}

void IRAM_ATTR handlePirInterrupt() {
  // 让中断服务函数尽量短，只设置标志位即可。
  pir_interrupt_flag = true;
}

void printBootInfo() {
  Serial.println();
  Serial.println("[INFO] backend booted");
  Serial.print("[INFO] ip: ");
  Serial.println(WiFi.localIP());
}

}  // namespace

void setup() {
  Serial.begin(115200);

  setupPins();
  dht.begin();
  setupRoutes();
  attachInterrupt(digitalPinToInterrupt(app::config::PIN_PIR), handlePirInterrupt, RISING);
  setupWiFi();
  server.begin();

  Serial.println("[INFO] http server started");
}

// =========================
// 主循环：轮询传感器、刷新状态、处理报警
// =========================
void loop() {
  server.handleClient();
  ensureWiFi();

  const unsigned long now = millis();
  if (pir_interrupt_flag) {
    pir_interrupt_flag = false;
    triggerIntrusionAlarm();
  }

  if (now - last_sensor_read_ms >= app::config::SENSOR_READ_INTERVAL_MS) {
    last_sensor_read_ms = now;
    if (readSensors()) {
      updateFanLogic();
    }
  }

  updateAlarmLogic();

  static bool boot_info_printed = false;
  if (!boot_info_printed && WiFi.status() == WL_CONNECTED) {
    boot_info_printed = true;
    printBootInfo();
  }
}
