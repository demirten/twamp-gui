TEMPLATE = app
QT += qml quick widgets
TARGET = twamp-client

CONFIG += qtquickcompiler

SOURCES += \
    main.cpp \
    twamp_client.cpp \
    twamp_test_worker.cpp

RESOURCES += qml/qml.qrc
RC_ICONS = ../msi/assets/TwampClient.ico

HEADERS += \
    twamp_client.h \
    log_model_data.h \
    twamp_test_worker.h

INCLUDEPATH += "$$PWD/../common"

CONFIG(debug, debug|release){
    LIBS += -L"$$OUT_PWD/../common/debug" -lcommon
}
CONFIG(release, debug|release){
    LIBS += -L"$$OUT_PWD/../common" -lcommon
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
