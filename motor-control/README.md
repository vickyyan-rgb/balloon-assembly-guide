# 腕带电机遥控交接

解压整个文件夹。BalloonMotorRemote/page.h 是网页源码，BalloonMotorRemote/BalloonMotorRemote.ino 提供网页服务并控制电机。上传一次固件即可同时安装网页。接线、供电及方向校准见 [接线说明](BalloonMotorRemote/README.md)。

## 用 Arduino IDE 上传

1. 安装 Arduino IDE 2，在开发板管理器安装 Espressif Systems 的 esp32 平台（本项目验证版本 4.0.0-alpha1）。
2. 打开 BalloonMotorRemote/BalloonMotorRemote.ino。用 USB 数据线连接腕带 Feather，选择 Adafruit Feather ESP32 V2 和插拔该板时随之出现的串口。
3. 点击 Upload。看到 Done uploading 后，再连接电机电源。仅插 USB 时，按 BAT 给驱动板供电的接法需要另行提供电机电源。

## 用 VS Code 上传（Windows PowerShell）

在 VS Code 打开包含 upload.ps1 的文件夹。沿用上面的开发板安装步骤，然后打开 Terminal，依次运行下列命令。最后一行用你查到的腕带串口替换 COM7。若提示串口被占用，先关闭串口监视器。

```powershell
.\upload.ps1 -ListPorts
.\upload.ps1 -VerifyOnly
.\upload.ps1 -Port COM7
```

上传命令结束后输入 `$LASTEXITCODE`，0 表示成功。

## 打开遥控网页

1. 连接腕带 Wi-Fi **Balloon-Motor**，密码 **PebbleRun27**。电脑或手机提示没有互联网时选择保持连接。
2. 在地址栏打开 **http://192.168.4.1/**。网页由手腕上的 Feather 提供。
3. 初次测试先卸下拉绳，用默认的 40%、0.5 秒点动确认方向。升起与降落已互换，时长可选 0–25 秒，步进 0.1 秒。设为 0 秒时点方向键保持停止。网页停止键和 GPIO32 实体按钮都能提前停止。
