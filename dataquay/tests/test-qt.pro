
TEMPLATE = app
CONFIG += debug
TARGET = test-qt-widgets

INCLUDEPATH += . ..
DEPENDPATH += . ..
LIBPATH += ..
PRE_TARGETDEPS += ../libdataquay.a

OBJECTS_DIR = o
MOC_DIR = o

LIBS += ../libdataquay.a

SOURCES += TestQtWidgets.cpp

exists(./platform.pri) {
    include(./platform.pri)
}
!exists(./platform.pri) {
    exists(../platform.pri) {
	include(../platform.pri)
    }
}

