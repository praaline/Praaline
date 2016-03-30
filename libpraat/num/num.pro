! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = num

INCLUDEPATH += ../external/gsl ../external/glpk ../sys ../dwsys

HEADERS += \
    NUM.h

SOURCES += \
    NUM.cpp \
    NUMarrays.cpp \
    NUMear.cpp \
    NUMlinprog.cpp \
    NUMrandom.cpp \
    NUMsort.cpp


