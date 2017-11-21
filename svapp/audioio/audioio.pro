TEMPLATE = lib

SV_UNIT_PACKAGES = fftw3f samplerate jack portaudio-2.0 libpulse rubberband
load(../prf/sv.prf)

CONFIG += sv staticlib qt thread warn_on stl rtti exceptions
QT -= gui

TARGET = svaudioio

DEPENDPATH += ..
INCLUDEPATH += . ..
OBJECTS_DIR = tmp_obj
MOC_DIR = tmp_moc

HEADERS += AudioCallbackPlaySource.h \
           AudioCallbackPlayTarget.h \
           AudioCoreAudioTarget.h \
           AudioGenerator.h \
           AudioJACKTarget.h \
           AudioPortAudioTarget.h \
           AudioPulseAudioTarget.h \
           AudioTargetFactory.h \
           PlaySpeedRangeMapper.h
SOURCES += AudioCallbackPlaySource.cpp \
           AudioCallbackPlayTarget.cpp \
           AudioCoreAudioTarget.cpp \
           AudioGenerator.cpp \
           AudioJACKTarget.cpp \
           AudioPortAudioTarget.cpp \
           AudioPulseAudioTarget.cpp \
           AudioTargetFactory.cpp \
           PlaySpeedRangeMapper.cpp
