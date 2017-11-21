
TEMPLATE = app

LIBS += -L../../.. -L../../../../dataquay -L../../../release -L../../../../dataquay/release -lsvcore -ldataquay

win32-g++ {
    INCLUDEPATH += ../../../../sv-dependency-builds/win32-mingw/include
    LIBS += -L../../../../sv-dependency-builds/win32-mingw/lib
}
win32-msvc* {
    INCLUDEPATH += ../../../../sv-dependency-builds/win32-msvc/include
    LIBS += -L../../../../sv-dependency-builds/win32-msvc/lib
}
mac* {
    INCLUDEPATH += ../../../../sv-dependency-builds/osx/include
    LIBS += -L../../../../sv-dependency-builds/osx/lib
}

exists(../../../config.pri) {
    include(../../../config.pri)
}

!exists(../../../config.pri) {

    CONFIG += release
    DEFINES += NDEBUG BUILD_RELEASE NO_TIMING

    DEFINES += HAVE_BZ2 HAVE_FFTW3 HAVE_FFTW3F HAVE_SNDFILE HAVE_SAMPLERATE HAVE_VAMP HAVE_VAMPHOSTSDK HAVE_RUBBERBAND HAVE_DATAQUAY HAVE_LIBLO HAVE_MAD HAVE_ID3TAG HAVE_PORTAUDIO_2_0

    LIBS += -lbz2 -lrubberband -lvamp-hostsdk -lfftw3 -lfftw3f -lsndfile -lFLAC -logg -lvorbis -lvorbisenc -lvorbisfile -logg -lmad -lid3tag -lportaudio -lsamplerate -lz -lsord-0 -lserd-0

    win* {
        LIBS += -llo -lwinmm -lws2_32
    }
    macx* {
        DEFINES += HAVE_COREAUDIO
        LIBS += -framework CoreAudio -framework CoreMidi -framework AudioUnit -framework AudioToolbox -framework CoreFoundation -framework CoreServices -framework Accelerate
    }
}

CONFIG += qt thread warn_on stl rtti exceptions console c++11
QT += network xml testlib
QT -= gui

TARGET = svcore-data-fileio-test

DEPENDPATH += ../../..
INCLUDEPATH += ../../..
OBJECTS_DIR = o
MOC_DIR = o

HEADERS += AudioFileReaderTest.h \
           AudioTestData.h
SOURCES += main.cpp

win* {
//PRE_TARGETDEPS += ../../../svcore.lib
}
!win* {
PRE_TARGETDEPS += ../../../libsvcore.a
}

!win32 {
    !macx* {
        QMAKE_POST_LINK=./$${TARGET}
    }
    macx* {
        QMAKE_POST_LINK=./$${TARGET}.app/Contents/MacOS/$${TARGET}
    }
}

win32:QMAKE_POST_LINK=./release/$${TARGET}.exe

