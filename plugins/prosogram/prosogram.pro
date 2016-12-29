# Praaline Plugin
# Prosogram
# (c) 1993-2015 Piet Mertens
# adapted for Praaline by George Christodoulides

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_PROSOGRAM_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Prosogramd
} else {
    TARGET = Prosogram
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../pncore ../../svcore ../../svgui ../../app/interfaces
DEPENDPATH += . .. ../.. ../../pncore ../../svcore ../../svgui ../../app/interfaces

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
LIBS +=  \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginprosogram_global.h \
    pluginprosogram.h \ 
    annotationpluginpraatscript.h \
    prosogram.h
	
SOURCES += \ 
    pluginprosogram.cpp \ 
    annotationpluginpraatscript.cpp \
    prosogram.cpp
