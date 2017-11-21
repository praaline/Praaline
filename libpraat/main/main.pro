! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TARGET = Praat
linux*:TARGET = praat
solaris*:TARGET = praat

TEMPLATE = app

INCLUDEPATH += ../sys ../num ../fon

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}

QT += core gui widgets

CONFIG += no_lflags_merge

win* {
    QMAKE_LIBS_QT_ENTRY -= -lqtmain
    DEFINES -= QT_NEEDS_QMAIN
    DEFINES += main=wingwmain
    LIBS += -lwinmm -lwsock32 -lcomctl32 -lole32 -lgdi32 -lgdiplus -lcomdlg32 -static-libgcc -static-libstdc++ -mwindows
}
linux* {
    LIBS += -lm -lpulse -lasound -lpthread
}

# Application components
PRAAT_LIBS = -lfon -lola -lartsynth -lFFNet -lgram -lEEG -lLPC -ldwtools -lfon -lstat -ldwsys -lsys -lnum -lkar -lespeak -ldwsys -lportaudio -lflac -lmp3 -lglpk -lgsl
PRAAT_LIBS = -L../contrib/ola/$${COMPONENTSPATH} \
            -L../artsynth/$${COMPONENTSPATH} \
            -L../FFNet/$${COMPONENTSPATH} \
            -L../gram/$${COMPONENTSPATH} \
            -L../EEG/$${COMPONENTSPATH} \
            -L../LPC/$${COMPONENTSPATH} \
            -L../dwtools/$${COMPONENTSPATH} \
            -L../fon/$${COMPONENTSPATH} \
            -L../stat/$${COMPONENTSPATH} \
            -L../dwsys/$${COMPONENTSPATH} \
            -L../sys/$${COMPONENTSPATH} \
            -L../num/$${COMPONENTSPATH} \
            -L../kar/$${COMPONENTSPATH} \
            -L../external/espeak/$${COMPONENTSPATH} \
            -L../external/portaudio/$${COMPONENTSPATH} \
            -L../external/flac/$${COMPONENTSPATH} \
            -L../external/mp3/$${COMPONENTSPATH} \
            -L../external/glpk/$${COMPONENTSPATH} \
            -L../external/gsl/$${COMPONENTSPATH} \
            $$PRAAT_LIBS

LIBS = $$PRAAT_LIBS $$LIBS

SOURCES += main_Praat.cpp
