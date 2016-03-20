# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

TARGET = praaline-crf

DEFINES += HAVE_CRFPP_CONFIG_H

HEADERS += \
    winmain.h \
    timer.h \
    thread.h \
    tagger.h \
    stream_wrapper.h \
    scoped_ptr.h \
    path.h \
    param.h \
    node.h \
    mmap.h \
    lbfgs.h \
    freelist.h \
    feature_index.h \
    feature_cache.h \
    encoder.h \
    darts.h \
    crfpp.h \
    config.h \
    common.h

SOURCES += \
    tagger.cpp \
    path.cpp \
    param.cpp \
    node.cpp \
    libcrfpp.cpp \
    lbfgs.cpp \
    feature_index.cpp \
    feature_cache.cpp \
    feature.cpp \
    encoder.cpp

