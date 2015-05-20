TEMPLATE = lib
CONFIG += staticlib
SOURCES = twamp_common.cpp
HEADERS = twamp_common.h

CONFIG(debug, debug|release){
    DESTDIR = ./debug
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = debug/.ui
}

CONFIG(release, debug|release){
    DESTDIR = ./
    OBJECTS_DIR = .obj
    MOC_DIR = .moc
    RCC_DIR = .rcc
    UI_DIR = .ui
}
