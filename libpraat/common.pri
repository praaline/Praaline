#Includes common configuration for all subdirectory .pro files.
INCLUDEPATH += . ..
#WARNINGS += -Wall

TEMPLATE = lib

CONFIG += thread

QMAKE_CFLAGS += -std=gnu99 -O1

win* {
    QMAKE_CXXFLAGS += -std=c++0x -Wshadow
    DEFINES += PRAAT_LIB "WINVER=0x0600" "_WIN32_WINNT=0x0600" "_WIN32_IE=0x0700" UNICODE "_FILE_OFFSET_BITS=64" #"main=wingwmain"
}
linux* {
    QMAKE_CFLAGS += -Werror=missing-prototypes -Werror=implicit -Wreturn-type -Wunused -Wunused-parameter -Wuninitialized -g1 -pthread
    CONFIG += link_pkgconfig
    PKGCONFIG += gtk+-2.0
    QMAKE_CXXFLAGS += -std=c++11 -Wshadow
    DEFINES += PRAAT_LIB UNIX linux ALSA HAVE_PULSEAUDIO "_FILE_OFFSET_BITS=64"
}
macx* {
    DEFINES += PRAAT_LIB macintosh "_FILE_OFFSET_BITS=64"
    QMAKE_CXXFLAGS += -std=c++11 -Wshadow
}
# For a shared libary:
DEFINES += PRAAT_LIB_SHARED_LIBRARY

# The following keeps the generated files at least somewhat separate
# from the source files.
release: DESTDIR = build/release
debug:   DESTDIR = build/debug

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

win32 {
    CONFIG(debug, debug|release) {
        PRAATLIB_POSTFIX = d$${PRAATLIB_FILE_VER}
    } else {
        PRAATLIB_POSTFIX = $${PRAATLIB_FILE_VER}
    }
    LIB_SUFFIX = a
}
unix {
    CONFIG(debug, debug|release) {
        PRAATLIB_POSTFIX = d$${PRAATLIB_FILE_VER}
    } else {
        PRAATLIB_POSTFIX = $${PRAATLIB_FILE_VER}
    }
    LIB_SUFFIX = so
}
mac {
    LIB_SUFFIX = dylib
}
