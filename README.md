# 强制门户（ESP32）

可以用在ESP32上的强制门户。

> 什么是强制门户？查看：[Captive portal - Wikipedia](https://en.wikipedia.org/wiki/Captive_portal)

## 项目原理

已发布到本人的个人博客：[使用ESP32C3开发板实现一个强制门户（Captive Portal） | 薛定谔的第四旋臂 (direct5dom.github.io)](https://direct5dom.github.io/2023/02/06/使用ESP32C3开发板实现一个强制门户（Captive-Portal）/)

## 🛠️从源码构建

首先，确保您已经安装了[ESP-IDF](https://docs.espressif.com/projects/esp-idf/zh_CN/stable/esp32/get-started/index.html)。

然后克隆此项目到本地，并进入项目目录：

```bash
git clone https://github.com/Direct5dom/ESP32CaptivePortal.git
cd ESP32CaptivePortal
```

现在您需要将项目设定为您所使用的芯片，本项目适用于大多数ESP32芯片，我本人所使用的ESP32C3。

```bash
idf.py set-target esp32c3
```

> 输入这条命令后应该会自动`build`，如果没有，请运行：
>
> ```bash
> idf.py build
> ```

烧录此项目到设备：

```bash
idf.py -p PORT [-b BAUD] flash
```

- `PORT` - ESP32开发板的串口名称。
- `[-b BAUD]` - 可以将`BAUD`替换为其他烧录波特率。此项可以为空，默认波特率为 `460800`。

## ⚖️License

[MIT License](https://github.com/Direct5dom/ESP32CaptivePortal/blob/master/LICENSE)
