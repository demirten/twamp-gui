TEMPLATE = app

QT += qml quick widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

include(src/src.pri)

RESOURCES += qml/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =
