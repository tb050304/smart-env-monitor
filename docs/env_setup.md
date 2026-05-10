# 🛠️ 开发环境与依赖库规范 (Environment Setup)

为了防止编译报错与版本冲突，所有参与 C++ 后端编译烧录的同学必须统一以下环境。

## 1. 开发工具链
* **推荐 IDE**: `[填写推荐的编辑器，如 Arduino IDE 2.x 或 VS Code + PlatformIO]`
* **开发板支持包 (Board Manager)**: `[填写 ESP 核心包名称及版本，例如 esp32 by Espressif Systems v2.0.11]`

## 2. 第三方依赖库列表
| 库名称 | 作者/来源 | 必须使用的版本号 | 用途说明 |
| :--- | :--- | :--- | :--- |
| **DHT sensor library** | Adafruit | `[填写具体版本号]` | 驱动 DHT11 读取温湿度 |
| **Adafruit Unified Sensor** | Adafruit | `[填写具体版本号]` | DHT 库的基础依赖 |
| **ArduinoJson** | Benoit Blanchon | `[填写具体版本号，注意区分 V6/V7]` | 解析和生成 API 的 JSON 数据 |

## 3. 全局代码约定
* **波特率 (Baud Rate)**: `[填写串口打印速率，如 115200]`
* **非阻塞约定**: 主 `loop()` 中绝对禁止使用 `delay()` 函数，传感器轮询必须使用基于 `millis()` 的定时器。
