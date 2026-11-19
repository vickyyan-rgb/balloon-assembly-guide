# 气球搭子 · 组装手册

这是一份中文图解手册，带你从桌面接线开始，装好腕带电机盒和气球声舱，再按下腕带按钮，检查电机、灯环和声音能否一起响应。

**[打开在线装配手册](https://vickyyan-rgb.github.io/balloon-assembly-guide/)**

## 跟着手册组装

1. 点击「零件与下载」，把两套电子零件分开放好。
2. 回到「装配步骤」，点击「开始第一步」。31 个步骤覆盖接线、桌面测试、装盒和两盒联测。
3. 每接好一根线就勾选一行。点击结构图可放大，点击「我卡在这一步了」可查看排查方法。进度保存在当前浏览器。

腕带使用 Feather ESP32 V2、DRV8833、N20 电机和外接瞬时按钮，V8 腕盒采用齐平底穿式表带。气球端使用 XIAO ESP32-S3、MAX98357A 和 8 Ω 喇叭。紧凑声舱底面有三个出声孔，LED 灯环独立悬挂在盒外。

## 打印件与程序

| 下载 | 用途 |
| --- | --- |
| [腕盒 V8 打印包](downloads/Wristband-V8-STL.zip) | 一体主壳、齐平底板、侧检修盖和装配图 |
| [紧凑声舱打印包](downloads/Balloon-Pod-STL.zip) | 出声外壳、三层抽屉和装配图 |
| [WristBench.ino](downloads/WristBench.ino) | 测试腕带按钮、短时电机点动和无线呼叫 |
| [BalloonBench.ino](downloads/BalloonBench.ino) | 测试灯环、提示音和无线接收 |

STL 使用毫米，按 100% 比例导入切片软件。先用实物核对电池、喇叭、电容和插头尺寸。两份程序用于桌面原型联测；儿童试戴前需另行验证受阻停机、绳端脱开和佩戴安全。

## 离线打开

在仓库页面点击 **Code → Download ZIP**，完整解压后，用浏览器打开根目录的 `index.html`。图片、接线步骤、固件和打印包均随仓库提供。访问制造商资料链接时需要联网。

## 维护与发布

这是一个无需构建的静态网站。`wrist-data.js` 和 `balloon-data.js` 分别存放两套模块的装配步骤，`guide-data.js` 存放零件清单与下载入口。`assets/` 存放结构图，`downloads/` 存放可下载文件。

GitHub Pages 从 `main` 分支的根目录发布。修改后提交到 `main`，即可在仓库的 **Actions** 或 **Settings → Pages** 查看发布状态。
