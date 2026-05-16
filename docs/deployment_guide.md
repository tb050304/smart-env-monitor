# 🚀 系统部署与落地指南 (Deployment Guide)

## 1. 硬件准备

### 1.1 接线确认（参考 hardware_interface.md）

| 模块 | GPIO 引脚 | 连接说明 |
|------|----------|----------|
| DHT11 | GPIO 4 | 数据引脚，需要 10K 上拉电阻 |
| HC-SR501 | GPIO 14 | 中断输入，上升沿触发 |
| 继电器 | GPIO 5 | 输出控制，低电平触发 |
| 蜂鸣器 | GPIO 12 | 输出控制，高电平触发 |

### 1.2 硬件检查清单
- [ ] ESP32 开发板已焊接排针
- [ ] 传感器模块已正确连接
- [ ] 继电器模块使用独立 5V 供电
- [ ] 所有地线共地连接

---

## 2. 软件配置

### 2.1 修改 Wi-Fi 配置

打开 `backend_cpp/include/app_config.h`，修改以下内容：

```cpp
// 修改前
inline constexpr const char* WIFI_SSID = "Your_SSID";
inline constexpr const char* WIFI_PASSWORD = "Your_PASSWORD";

// 修改后（替换为实际的 Wi-Fi 信息）
inline constexpr const char* WIFI_SSID = "MyHomeWiFi";
inline constexpr const char* WIFI_PASSWORD = "MyWiFiPassword123";
```

### 2.2 确认阈值参数（可根据需求调整）

```cpp
inline constexpr float AUTO_FAN_ON_TEMP_C = 30.0f;    // 风扇开启温度阈值
inline constexpr float AUTO_FAN_ON_HUMI_PCT = 70.0f;   // 风扇开启湿度阈值
inline constexpr float AUTO_FAN_OFF_TEMP_C = 28.0f;   // 风扇关闭温度阈值
inline constexpr float AUTO_FAN_OFF_HUMI_PCT = 65.0f;  // 风扇关闭湿度阈值
```

---

## 3. 固件烧录

### 3.1 环境要求
- 已安装 PlatformIO（VS Code 插件）
- ESP32 开发板连接电脑
- USB 驱动已安装

### 3.2 烧录步骤
1. 打开 `backend_cpp/platformio.ini` 配置文件
2. 确保 `board = nodemcu-32s` 与实际硬件匹配
3. 点击 PlatformIO 工具栏的「Upload」按钮
4. 等待烧录完成

### 3.3 上传前端文件到 SPIFFS

#### 方法一：使用 PlatformIO（推荐）
1. 在 PlatformIO 工具栏点击「Upload File System Image」
2. 确保 `frontend_web/index.html` 已放入 `data/` 目录

#### 方法二：手动上传
```bash
# 安装 esptool
pip install esptool

# 生成 SPIFFS 镜像
mkspiffs -c data -b 4096 -s 0x100000 spiffs.bin

# 上传到 ESP32
esptool.py --port COM3 write_flash 0x290000 spiffs.bin
```

---

## 4. 局域网配置

### 4.1 连接网络
- ESP32 上电后会自动连接配置的 Wi-Fi
- 通过串口监视器查看 ESP32 的 IP 地址：
  ```
  [INFO] ip: 192.168.1.100
  ```

### 4.2 访问监控页面
1. 确保电脑/手机与 ESP32 连接到**同一局域网**
2. 在浏览器中输入：`http://<ESP32-IP>/`
3. 例如：`http://192.168.1.100/`

### 4.3 网络故障排查
- 检查 ESP32 是否已正确连接 Wi-Fi
- 确认电脑与 ESP32 在同一网段
- 尝试使用手机热点测试网络连接

---

## 5. 功能验证

### 5.1 基础功能测试
| 测试项 | 操作方法 | 预期结果 |
|--------|----------|----------|
| 温度显示 | 观察页面 | 温度数值实时更新 |
| 湿度显示 | 观察页面 | 湿度数值实时更新 |
| 风扇自动控制 | 加热环境 | 温度≥30°C 时风扇自动开启 |
| 手动控制 | 点击按钮 | 风扇状态立即改变 |
| 报警功能 | 触发 PIR 传感器 | 蜂鸣器鸣叫，页面显示警报 |

### 5.2 状态机验证
- **AUTO 模式**：风扇根据温湿度自动启停
- **MANUAL_ON**：风扇强制开启，忽略温湿度
- **MANUAL_OFF**：风扇强制关闭，忽略温湿度

---

## 6. 常见问题

### 6.1 页面无法访问
- 检查 ESP32 是否正常上电
- 确认 IP 地址是否正确
- 检查防火墙是否阻止访问

### 6.2 温湿度显示异常
- 检查 DHT11 接线是否正确
- 确认上拉电阻已正确连接
- 查看串口日志是否有错误信息

### 6.3 风扇不工作
- 检查继电器接线是否正确
- 确认继电器使用独立 5V 供电
- 测试继电器控制引脚是否有信号输出

---

## 7. 维护与更新

### 7.1 固件更新
1. 修改代码后重新编译
2. 使用 PlatformIO 重新上传固件

### 7.2 前端更新
1. 修改 `frontend_web/index.html`
2. 重新上传 SPIFFS 镜像

### 7.3 日常维护
- 定期检查传感器接线是否松动
- 保持设备通风良好
- 避免设备长时间工作在高温环境

---

## 8. API 接口参考

### 接口列表
- `GET /api/status` - 获取当前环境状态
- `POST /api/control` - 发送控制命令（auto/on/off）

### 返回格式
```json
{
  "temperature": 26.5,
  "humidity": 60.0,
  "mode": "AUTO",
  "fan_state": false,
  "intrusion_alert": false
}
```

---

## 9. 项目目录结构

```
smart-env-monitor/
├── backend_cpp/                    # 后端 C++ 代码
│   ├── .vscode/                    # VS Code 配置
│   ├── include/                    # 头文件
│   │   └── app_config.h            # 应用配置（Wi-Fi、引脚、阈值）
│   ├── src/                        # 源文件
│   │   └── main.cpp                # 主程序入口
│   ├── README.md                   # 后端说明文档
│   └── platformio.ini              # PlatformIO 配置
├── frontend_web/                   # 前端 Web 代码
│   ├── index.html                  # 生产版本（用于烧录）
│   └── index_mock.html             # Mock 版本（用于开发测试）
├── docs/                           # 项目文档
│   ├── api_protocol.md             # 前后端通信协议
│   ├── hardware_interface.md       # 硬件接口与接线协议
│   ├── logic_fsm.md                # 状态机与核心逻辑规范
│   ├── env_setup.md                # 开发环境与依赖库规范
│   └── deployment_guide.md         # 部署与落地指南
├── hardware_docs/                  # 硬件资料（接线图、手册等）
├── .gitignore                      # Git 忽略配置
└── README.md                       # 项目根目录说明
```

### 目录说明
| 目录/文件 | 说明 |
|-----------|------|
| `backend_cpp/` | ESP32 后端固件代码，包含状态机、API、传感器驱动 |
| `frontend_web/index.html` | 干净版前端页面，用于最终烧录到 ESP32 |
| `frontend_web/index_mock.html` | 含 Mock 数据的前端页面，用于开发测试 |
| `docs/` | 项目核心文档，包含协议规范和部署指南 |
| `hardware_docs/` | 硬件接线照片、电路图等实物资料存放处 |

---

**文档版本**: 1.0  
**最后更新**: 2026-05-16