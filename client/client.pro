TEMPLATE = app
QT += qml quick widgets
TARGET = twamp-client

SOURCES += \
    main.cpp \
    twamp_client.cpp \
    twamp_test_worker.cpp

RESOURCES += qml/qml.qrc

HEADERS += \
    twamp_client.h \
    log_model_data.h \
    twamp_test_worker.h

INCLUDEPATH += ../common
LIBS += -L../common -lcommon

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
