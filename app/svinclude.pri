
exists(svconfig.pri) {
    include(svconfig.pri)
}

!exists(svconfig.pri) {
    win32-g++ {
        INCLUDEPATH += $$PWD/../dependency-builds/sv/win32-mingw/include
        LIBS += -L$$OUT_PWD/../dependency-builds/sv/win32-mingw/lib
    }
    win32-msvc* {
        INCLUDEPATH += $$PWD/../dependency-builds/sv/win32-msvc/include
        LIBS += -L$$OUT_PWD/dependency-builds/sv/win32-msvc/lib
    }
    macx* {
        INCLUDEPATH += $$PWD/../dependency-builds/sv/osx/include
        LIBS += -L$$OUT_PWD/../dependency-builds/sv/osx/lib
    }

    DEFINES += HAVE_BZ2 HAVE_FFTW3 HAVE_FFTW3F HAVE_SNDFILE HAVE_SAMPLERATE HAVE_VAMP HAVE_VAMPHOSTSDK HAVE_RUBBERBAND HAVE_LIBLO HAVE_MAD HAVE_ID3TAG

    LIBS += -lbz2 -lrubberband -lvamp-hostsdk -lfftw3 -lfftw3f -lsndfile -lFLAC -logg -lvorbis -lvorbisenc -lvorbisfile -logg -lmad -lid3tag -lsamplerate -llo -lz -lsord-0 -lserd-0

    win* {
        DEFINES += HAVE_PORTAUDIO_2_0
        LIBS += -lportaudio -lwinmm -lws2_32
    }
    macx* {
        DEFINES += HAVE_COREAUDIO HAVE_PORTAUDIO_2_0
        LIBS += -lportaudio -framework CoreAudio -framework CoreMidi -framework AudioUnit -framework AudioToolbox -framework CoreFoundation -framework CoreServices -framework Accelerate
    }
    linux* {
        DEFINES += HAVE_LIBPULSE
        LIBS += -lpulse -lasound -ldl
    }
}

CONFIG(release, debug|release) {
    DEFINES += NDEBUG BUILD_RELEASE NO_TIMING
}

DEPENDPATH += . ../svcore ../svgui ../svapp
INCLUDEPATH += . ../svcore ../svgui ../svapp

contains(DEFINES, BUILD_STATIC):LIBS -= -ljack

linux* {
SV_LIBS = -lsvapp -lsvgui -lsvcore -ldataquay
SV_LIBS = -L../svapp/$${COMPONENTSPATH} -L../svgui/$${COMPONENTSPATH} -L../svcore/$${COMPONENTSPATH} -L../dataquay $$SV_LIBS
SV_LIBS = -Wl,-Bstatic $$SV_LIBS -Wl,-Bdynamic -L../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX}
}

win* {
SV_LIBS = -lsvapp -lsvgui -lsvcore -ldataquay -lpncore$${PRAALINE_LIB_POSTFIX}
SV_LIBS = -L../svapp/$${COMPONENTSPATH} -L../svgui/$${COMPONENTSPATH} -L../svcore/$${COMPONENTSPATH} -L../dataquay/release -L../pncore/$${COMPONENTSPATH} $$SV_LIBS
}

macx* {
SV_LIBS = -lsvapp -lsvgui -lsvcore -ldataquay -lpncore$${PRAALINE_LIB_POSTFIX}
SV_LIBS = -L../svapp/$${COMPONENTSPATH} -L../svgui/$${COMPONENTSPATH} -L../svcore/$${COMPONENTSPATH} -L../dataquay -L../pncore/$${COMPONENTSPATH} $$SV_LIBS
}

LIBS = $$SV_LIBS $$LIBS

win32-msvc* {
PRE_TARGETDEPS += ../svapp/svapp.lib \
                  ../svgui/svgui.lib \
                  ../svcore/svcore.lib \
                  ../dataquay/release/dataquay.lib
}

win32-g++ {
PRE_TARGETDEPS += ../svapp/$${COMPONENTSPATH}/libsvapp.a \
                  ../svgui/$${COMPONENTSPATH}/libsvgui.a \
                  ../svcore/$${COMPONENTSPATH}/libsvcore.a \
                  ../dataquay/release/libdataquay.a
}

!win* {
PRE_TARGETDEPS += ../svapp/$${COMPONENTSPATH}/libsvapp.a \
                  ../svgui/$${COMPONENTSPATH}/libsvgui.a \
                  ../svcore/$${COMPONENTSPATH}/libsvcore.a \
                  ../dataquay/libdataquay.a
}
