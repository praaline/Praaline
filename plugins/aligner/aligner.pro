# Praaline Plugin
# Forced Alignment
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_ALIGNER_LIBRARY

QT += gui sql concurrent
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Alignerd
} else {
    TARGET = Aligner
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../pncore ../../pnlib ../../app/annotation
DEPENDPATH += . .. ../.. ../../pncore ../../pnlib

# Qtilities configuration
QTILITIES += extension_system
include(../../external/qtilities/src/Qtilities.pri)
INCLUDEPATH += ../../external/qtilities/include


# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}
# Application components
LIBS += -L../../pnlib/$${COMPONENTSPATH} -lpnlib \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../pnlib/$${COMPONENTSPATH}/libpnlib.a
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginaligner_global.h \
    pluginaligner.h \
    phonemedatabase.h \
    easyalignbasic.h \
    annotationpluginpraatscript.h \
    LongSoundAligner.h
	
SOURCES += \ 
    pluginaligner.cpp \
    phonemedatabase.cpp \
    easyalignbasic.cpp \
    annotationpluginpraatscript.cpp \
    LongSoundAligner.cpp
