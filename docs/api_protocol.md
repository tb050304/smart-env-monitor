# 🌐 前后端通信协议 (API Protocol)

本系统采用 B/S 架构，前端通过 `Fetch/AJAX` 异步调用 ESP32 提供的 RESTful API。

## 1. 基础配置
* **基地址 (Base URL)**: `http://<esp32-lan-ip>`
* **数据格式**: `application/json`
* **字符编码**: `UTF-8`

## 2. 状态拉取接口
* **请求**: `GET /api/status`
* **用途**: 前端定时拉取当前环境数据与设备状态。
* **成功返回**:
  ```json
  {
    "temperature": 26.5,
    "humidity": 60.0,
    "mode": "AUTO",
    "fan_state": false,
    "intrusion_alert": false
  }
  ```
* **字段约定**:
  * `temperature`: 当前温度，单位 `°C`
  * `humidity`: 当前湿度，单位 `%`
  * `mode`: 当前工作模式，取值 `AUTO` / `MANUAL_ON` / `MANUAL_OFF`
  * `fan_state`: 风扇当前是否开启，布尔值
  * `intrusion_alert`: 是否处于闯入报警状态，布尔值

## 3. 控制接口
* **请求**: `POST /api/control`
* **用途**: 前端下发风扇模式切换指令。
* **请求体**:
  ```json
  {
    "cmd": "auto"
  }
  ```
* **`cmd` 允许值**:
  * `auto`: 恢复自动模式
  * `on`: 强制开启风扇
  * `off`: 强制关闭风扇
* **成功返回**:
  ```json
  {
    "success": true,
    "message": "mode updated",
    "mode": "AUTO"
  }
  ```
* **错误返回**:
  ```json
  {
    "success": false,
    "message": "invalid cmd"
  }
  ```

## 4. 状态码约定
* `200`: 请求成功
* `400`: 参数错误或命令非法
* `500`: 设备内部异常
