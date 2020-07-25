TEMPLATE = app
QT += qml quick widgets
TARGET = twamp-responder

CONFIG += qtquickcompiler

# QMAKE_LFLAGS += -static-libgcc -static-libstdc++

SOURCES += \
    main.cpp \
    twamp_responder.cpp \
    twamp_responder_worker.cpp

RESOURCES += qml/qml.qrc

HEADERS += \
    twamp_responder.h \
    twamp_responder_worker.h \
    log_model_data.h

INCLUDEPATH += "$$PWD/../common"

CONFIG(debug, debug|release){
    LIBS += -L"$$PWD/../common/debug" -lcommon
}
CONFIG(release, debug|release){
    LIBS += -L"$$PWD/../common" -lcommon
}

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
