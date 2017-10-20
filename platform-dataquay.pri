
exists(config.pri) {
    include(./config.pri)
}

!exists(config.pri) {
    CONFIG += release
    win32-g++ {
        INCLUDEPATH += $$PWD/dependency-builds/sv/win32-mingw/include
        LIBS += -L$$PWD/dependency-builds/sv/win32-mingw/lib
    }
    win32-msvc* {
        INCLUDEPATH += $$PWD/dependency-builds/sv/win32-msvc/include
        LIBS += -L$$PWD/dependency-builds/sv/win32-msvc/lib
    }
    macx* {
        INCLUDEPATH += $$PWD/dependency-builds/sv/osx/include
        LIBS += -L$$PWD/dependency-builds/sv/osx/lib
    }
}

CONFIG += staticlib c++11

DEFINES += USE_SORD
# Libraries and paths should be added by config.pri

