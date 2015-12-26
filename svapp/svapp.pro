
! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TEMPLATE = lib

CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
    DEFINES += NDEBUG BUILD_RELEASE NO_TIMING
}

exists(config.pri) {
    include(config.pri)
}
!exists(config.pri) {
    win32-g++ {
        INCLUDEPATH += ../sv-dependency-builds/win32-mingw/include
        LIBS += -L../sv-dependency-builds/win32-mingw/lib
    }
    win32-msvc* {
        INCLUDEPATH += ../sv-dependency-builds/win32-msvc/include
        LIBS += -L../sv-dependency-builds/win32-msvc/lib
    }
    macx* {
        INCLUDEPATH += ../sv-dependency-builds/osx/include
        LIBS += -L../sv-dependency-builds/osx/lib
    }

    win* {
        DEFINES += HAVE_PORTAUDIO_2_0
    }
    macx* {
        DEFINES += HAVE_COREAUDIO HAVE_PORTAUDIO_2_0
    }
    unix*:!macx* {
        DEFINES += HAVE_LIBPULSE
    }
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11
QT += network xml gui widgets

TARGET = svapp

DEPENDPATH += . ../pncore ../svcore ../svgui
INCLUDEPATH += . ../pncore ../svcore ../svgui
OBJECTS_DIR = o
MOC_DIR = o

HEADERS += audioio/AudioCallbackPlaySource.h \
           audioio/AudioCallbackPlayTarget.h \
           audioio/AudioGenerator.h \
           audioio/AudioJACKTarget.h \
           audioio/AudioPortAudioTarget.h \
           audioio/AudioPulseAudioTarget.h \
           audioio/AudioTargetFactory.h \
           audioio/ClipMixer.h \
           audioio/ContinuousSynth.h \
           audioio/PlaySpeedRangeMapper.h \
    framework/VisualiserWindowBase.h

SOURCES += audioio/AudioCallbackPlaySource.cpp \
           audioio/AudioCallbackPlayTarget.cpp \
           audioio/AudioGenerator.cpp \
           audioio/AudioJACKTarget.cpp \
           audioio/AudioPortAudioTarget.cpp \
           audioio/AudioPulseAudioTarget.cpp \
           audioio/AudioTargetFactory.cpp \
           audioio/ClipMixer.cpp \
           audioio/ContinuousSynth.cpp \
           audioio/PlaySpeedRangeMapper.cpp \
    framework/VisualiserWindowBase.cpp

HEADERS += framework/Document.h \
           framework/SVFileReader.h \
           framework/TransformUserConfigurator.h \
           framework/VersionTester.h

SOURCES += framework/Document.cpp \
           framework/SVFileReader.cpp \
           framework/TransformUserConfigurator.cpp \
           framework/VersionTester.cpp

