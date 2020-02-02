# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

DEFINES += USE_NAMESPACE_PRAALINE_CORE

INCLUDEPATH += . .. ../.. ../../praaline-core/include
DEPENDPATH += . .. ../.. ../../praaline-core

TARGET = praaline-featextract

HEADERS += \
    OpenSmileVAD.h

SOURCES += \
    OpenSmileVAD.cpp
