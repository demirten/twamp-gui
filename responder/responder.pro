TEMPLATE = app
QT += qml quick widgets
TARGET = twamp-responder

SOURCES += \
    main.cpp \
    twamp_responder.cpp

RESOURCES += qml/qml.qrc

HEADERS += \
    twamp_responder.h \
    log_model_data.h

INCLUDEPATH += ../common
LIBS += -L../common -lcommon

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
