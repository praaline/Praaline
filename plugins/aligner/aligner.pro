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

# Linking dynamically with PocketSphinx
win32 {
    POCKETSPHINX_BASE_PATH = C:/Qt/mingw-4.9.2-x32
}
unix {
    POCKETSPHINX_BASE_PATH = /usr/local
}
# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}
# Application components
win32 {
    LIBS += -L../../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    LIBS += -L../../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase
}
LIBS += -L../../pnlib/mediautil/$${COMPONENTSPATH} -lpraaline-mediautil \
        -L../../pnlib/featextract/$${COMPONENTSPATH} -lpraaline-featextract \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX}
PRE_TARGETDEPS += \
        ../../pnlib/asr/$${COMPONENTSPATH}/libpraaline-asr.a
        ../../pnlib/mediautil/$${COMPONENTSPATH}/libpraaline-mediautil.a
        ../../pnlib/featextract/$${COMPONENTSPATH}/libpraaline-featextract.a
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginaligner_global.h \
    pluginaligner.h \
    phonemedatabase.h \
    annotationpluginpraatscript.h \
    LongSoundAligner.h \
    BroadClassAligner.h \
    EasyAlignBasic.h \
    EasyAlignBasic.h
	
SOURCES += \ 
    pluginaligner.cpp \
    phonemedatabase.cpp \
    annotationpluginpraatscript.cpp \
    LongSoundAligner.cpp \
    BroadClassAligner.cpp \
    EasyAlignBasic.cpp \
    EasyAlignBasic.cpp
