! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TEMPLATE = lib

CONFIG(debug, debug|release) {
    TARGET = qpraatd
} else {
    TARGET = qpraat
}
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += ../sys ../num

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}

QT += core
QT -= gui

CONFIG += no_lflags_merge

#win32:QMAKE_LIBS_QT_ENTRY -= -lqtmain
#win32-g++:DEFINES -= QT_NEEDS_QMAIN
#QMAKE_CFLAGS += main=wingwmain

# Libraries
win32 {
    LIBS += -lwinmm -lwsock32 -lcomctl32 -lole32 -lgdi32 -lgdiplus -lcomdlg32 -mwindows
}
unix {
    LIBS += `pkg-config --libs gtk+-2.0` -lm -lasound -lpthread
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

SOURCES += \
    PraatPitch.cpp \
    PraatSession.cpp
    # ../main/main_Praat.cpp

HEADERS += \
    PraatPitch.h \
    PraatSession.h

