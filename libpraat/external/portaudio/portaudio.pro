! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = portaudio

win32|unix {
    SOURCES = \
        pa_unix_hostapis.c pa_unix_util.c pa_linux_alsa.c \
        pa_win_hostapis.c pa_win_util.c pa_win_wmme.c pa_win_waveformat.c \
        pa_front.c pa_debugprint.c pa_cpuload.c \
        pa_allocation.c pa_process.c pa_converters.c pa_dither.c \
        pa_stream.c
}
mac {
    SOURCES = \
        pa_mac_hostapis.c pa_mac_util.c pa_mac_core.c pa_mac_core_blocking.c pa_mac_core_utilities.c \
        pa_ringbuffer.c pa_trace.c \
        pa_front.c pa_cpuload.c \
        pa_allocation.c pa_process.c pa_converters.c pa_dither.c \
        pa_stream.c
}

HEADERS += \
    pa_allocation.h \
    pa_asio.h \
    pa_converters.h \
    pa_cpuload.h \
    pa_debugprint.h \
    pa_dither.h \
    pa_endianness.h \
    pa_hostapi.h \
    pa_jack.h \
    pa_linux_alsa.h \
    pa_mac_core.h \
    pa_mac_core_blocking.h \
    pa_mac_core_internal.h \
    pa_mac_core_utilities.h \
    pa_mac_util.h \
    pa_memorybarrier.h \
    pa_process.h \
    pa_ringbuffer.h \
    pa_stream.h \
    pa_trace.h \
    pa_types.h \
    pa_unix_util.h \
    pa_util.h \
    pa_win_coinitialize.h \
    pa_win_ds.h \
    pa_win_ds_dynlink.h \
    pa_win_wasapi.h \
    pa_win_waveformat.h \
    pa_win_wdmks.h \
    pa_win_wdmks_utils.h \
    pa_win_wmme.h \
    pa_x86_plain_converters.h \
    portaudio.h
