# 🛠️ 开发环境与依赖库规范 (Environment Setup)

为了防止编译报错与版本冲突，所有参与 C++ 后端编译烧录的同学必须统一以下环境。

## 1. 开发工具链
* **推荐 IDE**: VS Code + PlatformIO
* **开发板支持包**: `esp32 by Espressif Systems v2.0.11`

## 2. 第三方依赖库列表
| 库名称 | 作者/来源 | 必须使用的版本号 | 用途说明 |
| :--- | :--- | :--- | :--- |
| **DHT sensor library** | Adafruit | `1.4.6` | 驱动 DHT11 读取温湿度 |
| **Adafruit Unified Sensor** | Adafruit | `1.1.15` | DHT 库的基础依赖 |
| **ArduinoJson** | Benoit Blanchon | `6.21.5` | 解析和生成 API 的 JSON 数据 |

## 3. 全局代码约定
* **波特率 (Baud Rate)**: `115200`
* **非阻塞约定**: 主 `loop()` 中禁止使用 `delay()`，传感器轮询必须使用基于 `millis()` 的定时器
