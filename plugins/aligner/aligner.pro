# Praaline Plugin
# Forced Alignment
# (c) George Christodoulides 2014-2020

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

INCLUDEPATH += . .. ../.. ../../app/interfaces ../../pnlib
DEPENDPATH += . .. ../..

# Dependency: Praaline Core
DEFINES += USE_NAMESPACE_PRAALINE_CORE
INCLUDEPATH += ../../praaline-core/include
DEPENDPATH += ../../praaline-core

# Dependency: Praaline Media
DEFINES += USE_NAMESPACE_PRAALINE_MEDIA
INCLUDEPATH += ../../praaline-media/include
DEPENDPATH += ../../praaline-media

# Dependency: Praaline ASR
DEFINES += USE_NAMESPACE_PRAALINE_ASR
INCLUDEPATH += ../../praaline-asr/include
DEPENDPATH += ../../praaline-asr

# Qtilities include for plugin interfaces
QTILITIES += extension_system
QTILITIES_SOURCE_BASE  = $$PWD/../../dependencies/qtilities
QTILITIES_BUILD_BASE   = $$OUT_PWD/../../dependencies/qtilities
QTILITIES_DEPENDENCIES = $$OUT_PWD/../../dependencies
include(../../dependencies/qtilities/src/Qtilities.pri)

# Linking dynamically with PocketSphinx
win32-g++ {
    POCKETSPHINX_BASE_PATH = $$PWD/../../dependency-builds/pn/win32-mingw
}
win32-msvc* {
    POCKETSPHINX_BASE_PATH = $$PWD/../../dependency-builds/pn/win32-msvc
}
unix {
    POCKETSPHINX_BASE_PATH = /usr/local
}

INCLUDEPATH += $${POCKETSPHINX_BASE_PATH}/include/pocketsphinx \
               $${POCKETSPHINX_BASE_PATH}/include/sphinxbase \
               $${POCKETSPHINX_BASE_PATH}/include/

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
    LIBS += -L../../praaline-asr/$${COMPONENTSPATH} -lpraaline-asr$${PRAALINE_LIB_POSTFIX} -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    LIBS += -L../../praaline-asr/$${COMPONENTSPATH} -lpraaline-asr$${PRAALINE_LIB_POSTFIX} -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase
}
LIBS += -L../../pnlib/featextract/$${COMPONENTSPATH} -lpraaline-featextract \
        -L../../praaline-media/$${COMPONENTSPATH} -lpraaline-media$${PRAALINE_LIB_POSTFIX} \
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX}
PRE_TARGETDEPS += \
        ../../pnlib/featextract/$${COMPONENTSPATH}/libpraaline-featextract.a
        ../../praaline-asr/$${COMPONENTSPATH}/libpraaline-asr$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-media/$${COMPONENTSPATH}/libpraaline-media$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginaligner_global.h \
    pluginaligner.h \
    phonemedatabase.h \
    annotationpluginpraatscript.h \
    LongSoundAligner.h \
    BroadClassAligner.h \
    EasyAlignBasic.h
	
SOURCES += \ 
    pluginaligner.cpp \
    phonemedatabase.cpp \
    annotationpluginpraatscript.cpp \
    LongSoundAligner.cpp \
    BroadClassAligner.cpp \
    EasyAlignBasic.cpp
