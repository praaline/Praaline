# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

TARGET = praaline-diff

HEADERS += \
    variables.h \
    ses.h \
    sequence.h \
    lcs.h \
    functors.h \
    dtl.h \
    diffintervals.h \
    diff3.h \
    diff.h

SOURCES += \
    diffintervals.cpp

