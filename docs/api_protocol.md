# 🌐 前后端通信协议 (API Protocol)

本系统采用 B/S 架构，前端通过 AJAX/Fetch 异步调用后端提供的 RESTful API 接口。

## 1. 基础配置
* **基地址 (Base URL)**: `http://[填写开发板分配到的局域网 IP]`
* **数据格式**: `application/json`

## 2. 状态拉取接口 (前端 -> 后端)
* **请求说明**: `GET /api/status`
* **功能描述**: 前端定时拉取当前环境数据与设备状态。
* **返回 JSON 结构约定**:
  ```json
  {
    "temperature": "[类型：浮点数，说明：当前温度]",
    "humidity": "[类型：浮点数，说明：当前湿度]",
    "mode": "[类型：字符串，可选值：AUTO / MANUAL_ON / MANUAL_OFF]",
    "fan_state": "[类型：布尔值，说明：风扇当前是否在转]",
    "intrusion_alert": "[类型：布尔值，说明：是否检测到闯入]"
  }
