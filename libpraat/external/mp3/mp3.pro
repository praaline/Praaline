! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = mp3

INCLUDEPATH += ../../sys

SOURCES = mp3.cpp \
        mad_bit.c \
        mad_decoder.c \
        mad_fixed.c \
        mad_frame.c \
        mad_huffman.c \
        mad_layer12.c \
        mad_layer3.c \
        mad_stream.c \
        mad_synth.c \
        mad_timer.c \
        mad_version.c

HEADERS += \
    mad_bit.h \
    mad_config.h \
    mad_decoder.h \
    mad_fixed.h \
    mad_frame.h \
    mad_global.h \
    mad_huffman.h \
    mad_layer3.h \
    mad_layer12.h \
    mad_stream.h \
    mad_synth.h \
    mad_timer.h \
    mad_version.h \
    mp3.h
