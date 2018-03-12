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

INCLUDEPATH += . .. ../.. ../../pncore ../../pnlib ../../app/interfaces
DEPENDPATH += . .. ../.. ../../pncore ../../pnlib

# Qtilities include for plugin interfaces
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesCore
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesCoreGui
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesLogging
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesExtensionSystem

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
    LIBS += -L../../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    LIBS += -L../../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase
}
LIBS += -L../../pnlib/media/$${COMPONENTSPATH} -lpraaline-media \
        -L../../pnlib/featextract/$${COMPONENTSPATH} -lpraaline-featextract \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX}
PRE_TARGETDEPS += \
        ../../pnlib/asr/$${COMPONENTSPATH}/libpraaline-asr.a
        ../../pnlib/mediautil/$${COMPONENTSPATH}/libpraaline-media.a
        ../../pnlib/featextract/$${COMPONENTSPATH}/libpraaline-featextract.a
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

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
