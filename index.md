## Twamp
Two-Way Active Measurement Protocol (TWAMP) provides standard way to make two way delay, availability and jitter tests between two nodes.

Before TWAMP, there are proprietary solutions to make these type of tests. Because of the there is no standard about how test flow must be done, you need to use same product or software both of the nodes.

After TWAMP, it is possible to make tests between vendors and products. Most of the network routers and switches start to support TWAMP internally so it is possible to make tests against the those network devices without a need to put a custom probe.

## Twamp Gui Tools
TWAMP is relatively new protocol and there is not much tools around it. I tried to implement a reference for both of the Twamp Client and Responder which you can clone the repository from:

```
$ git clone https://github.com/demirten/twamp-gui.git
```

## Cross Platform
I tried to make twamp gui runs natively on Linux, Windows and MacOSX platforms so Qt5 and QML technologies used to achieve this. You can build the executables from the codes if you have a running Qt5 development setup for your system. It is also possible to download prebuilt binaries for your platform.

## Downloads
* Debian Package:
[v1.0.5](https://github.com/demirten/twamp-gui/releases/download/v1.0.5/twamp-gui_1.0.5-1_amd64.deb)

* Windows Installer:
[v1.0.5 (win32/64)](https://github.com/demirten/twamp-gui/releases/download/v1.0.5/TwampGuiInstaller-1.0.5.exe)

* MacOSX Installer:
[v1.0.5](https://github.com/demirten/twamp-gui/releases/download/v1.0.5/Twamp-1.0.5-Installer.dmg)

* Android APK:
[v1.0.5](https://github.com/demirten/twamp-gui/releases/download/v1.0.5/twamp-gui-1.0.5.apk)

## Screenshots
Twamp Client
![twamp-client](https://cloud.githubusercontent.com/assets/43532/7880410/c5da98e2-0601-11e5-9a46-ec7392442c3a.png)

Packet Details
![twamp-detail](https://cloud.githubusercontent.com/assets/43532/7880411/c5de8a4c-0601-11e5-8e25-44d36dc5d1da.png)

Twamp Responder
![twamp-responder](https://cloud.githubusercontent.com/assets/43532/7767624/bf388ede-007c-11e5-9304-38d56c6d6693.png)

Android Client
![android](https://cloud.githubusercontent.com/assets/43532/7848319/5ed1dcfc-04d3-11e5-9158-e1882d752413.png)

## Authors and Contributors
Twamp Gui Tools written by the Murat Demirten (@demirten).

## Support / Contact
If you found a bug you can send email to mdemirten@yh.com.tr with tcpdump capture attached or create an issue on https://github.com/demirten/twamp-gui/issues

You can also get professional support from us for your TWAMP test network.

## HTML5 / Websocket Application
We built a HTML5 & Websocket Twamp client application for testing: (https://ping.as/twamp) 

