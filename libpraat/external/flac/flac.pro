! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = flac

INCLUDEPATH += ../../sys
DEFINES += FLAC__NOASM

SOURCES = \
        flac_bitmath.c \
        flac_bitreader.c \
        flac_bitwriter.c \
        flac_cpu.c \
        flac_crc.c \
        flac_fixed.c \
        flac_float.c \
        flac_format.c \
        flac_lpc.c \
        flac_md5.c \
        flac_memory.c \
        flac_metadata_iterators.c \
        flac_metadata_object.c \
        flac_stream_decoder.c \
        flac_stream_encoder.c \
        flac_stream_encoder_framing.c \
        flac_window.c

HEADERS += \
    flac_FLAC_all.h \
    flac_FLAC_assert.h \
    flac_FLAC_callback.h \
    flac_FLAC_export.h \
    flac_FLAC_format.h \
    flac_FLAC_metadata.h \
    flac_FLAC_ordinals.h \
    flac_FLAC_stream_decoder.h \
    flac_FLAC_stream_encoder.h \
    flac_private_all.h \
    flac_private_autocpu.h \
    flac_private_bitmath.h \
    flac_private_bitreader.h \
    flac_private_bitwriter.h \
    flac_private_cpu.h \
    flac_private_crc.h \
    flac_private_fixed.h \
    flac_private_float.h \
    flac_private_format.h \
    flac_private_lpc.h \
    flac_private_md5.h \
    flac_private_memory.h \
    flac_private_metadata.h \
    flac_private_stream_encoder_framing.h \
    flac_private_window.h \
    flac_protected_all.h \
    flac_protected_stream_decoder.h \
    flac_protected_stream_encoder.h \
    flac_share_alloc.h
