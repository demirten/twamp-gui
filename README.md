# twamp-gui

Cross platform twamp client with Qt QML backend

You must have a working Qt5 - QML Development environment, it is not tested with Qt4.

## Build on Linux

It is tested on Debian Jessie and newer versions. You need to install following packages:

```
$ sudo apt-get install qt5-default qtdeclarative5-dev qml-module-qtquick-controls \
  qml-module-qtquick-dialogs qml-module-qtquick-layouts qml-module-qtquick-window2 \
  qml-module-qtquick2 
```

After that you need to follow standard build process of Qt applications:

```
$ qmake
$ make
$ ./twamp-gui
```

## Build on Windows

Install latest Qt5 development kit and build the project.

## Build on Mac

Install latest Qt5 development kit and build the project.

## Precompiled Binaries

You can download precompiled binaries for several platforms:
https://github.com/demirten/twamp-gui/releases

## Screenshots

You can look at the screenshots on project page: http://demirten.github.io/twamp-gui/#screenshots
