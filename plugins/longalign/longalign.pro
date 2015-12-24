# Praaline Plugin
# Long Sound Alignment
# (c) George Christodoulides 2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_LONGALIGN_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = LongAlignd
} else {
    TARGET = LongAlign
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../pncore ../../app/annotation
DEPENDPATH += . .. ../.. ../../pncore

# Settings for PocketSphinx
win32 {
    POCKETSPHINX_BASE_PATH = C:/Qt/mingw-4.9.2-x32
}
unix {
    POCKETSPHINX_BASE_PATH = /usr/local
}
INCLUDEPATH += $${POCKETSPHINX_BASE_PATH}/include/pocketsphinx \
               $${POCKETSPHINX_BASE_PATH}/include/sphinxbase \
               $${POCKETSPHINX_BASE_PATH}/include/
win32 {
    LIBS += -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    LIBS += -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase
}

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
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginlongalign.h \
    pluginlongalign_global.h \
    speechrecogniser.h \
    languagemodel.h \
    recognitionresult.h \
    anchorsfinder.h
	
SOURCES += \ 
    pluginlongalign.cpp \
    speechrecogniser.cpp \
    languagemodel.cpp \
    recognitionresult.cpp \
    anchorsfinder.cpp
