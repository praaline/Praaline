
TEMPLATE = app
CONFIG += debug console
QT += testlib
QT -= gui
TARGET = test-dataquay

exists(../config.pri) {
	include(../config.pri)
}

INCLUDEPATH += . ..
DEPENDPATH += . ..
QMAKE_LIBDIR += ..

OBJECTS_DIR = o
MOC_DIR = o

!win32: LIBS += -Wl,-rpath,..

LIBS += -L.. -ldataquay	$${EXTRALIBS}

HEADERS += TestBasicStore.h TestDatatypes.h TestTransactionalStore.h TestImportOptions.h TestObjectMapper.h
SOURCES += TestDatatypes.cpp main.cpp

exists(../../platform-dataquay.pri) {
	include(../../platform-dataquay.pri)
}

exists(./platform.pri) {
    include(./platform.pri)
}
!exists(./platform.pri) {
    exists(../platform.pri) {
	include(../platform.pri)
    }
}

!win32 {
    !macx* {
        QMAKE_POST_LINK=./$${TARGET}
    }
    macx* {
        QMAKE_POST_LINK=./$${TARGET}.app/Contents/MacOS/$${TARGET}
    }
}

win32:QMAKE_POST_LINK=$${TARGET}.exe
