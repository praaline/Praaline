! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = kar

HEADERS += \
    longchar.h \
    UnicodeData.h

SOURCES += \
    ipaSerifRegularPS.cpp \
    longchar.cpp

