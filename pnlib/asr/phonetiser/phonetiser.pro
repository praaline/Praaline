# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

HEADERS += \
    RuleBasedPhonetiser.h \
    ExternalPhonetiser.h \
    DictionaryPhonetiser.h \
    AbstractPhonetiser.h

SOURCES += \
    RuleBasedPhonetiser.cpp \
    ExternalPhonetiser.cpp \
    DictionaryPhonetiser.cpp
