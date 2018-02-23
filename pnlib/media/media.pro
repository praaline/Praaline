# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

TARGET = praaline-media

HEADERS += \
    SoundInfo.h \
    AudioSegmenter.h \
    SoxExternal.h \
    SoxExecutable.h

SOURCES += \
    SoundInfo.cpp \
    AudioSegmenter.cpp \
    SoxExternal.cpp \
    SoxExecutable.cpp
