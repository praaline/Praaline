# Praaline Plugin
# Prosodic prominence
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_PROMISE_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Promised
} else {
    TARGET = Promise
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
!mac*:DESTDIR += ../../../../../../app/build/plugins/
mac*:DESTDIR = ../../app/build/plugins/

INCLUDEPATH += . .. ../.. ../../pncore ../../app/annotation
DEPENDPATH += . .. ../.. ../../pncore

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
    pluginpromise_global.h \
    pluginpromise.h \ 
    pluginpromisefeatures.h \
    annotationpluginpraatscript.h \
    pluginprosobox5.h \
    attributenametranslation.h
	
SOURCES += \ 
    pluginpromise.cpp \ 
    pluginpromisefeatures.cpp \
    annotationpluginpraatscript.cpp \
    pluginprosobox5.cpp \
    attributenametranslation.cpp
	
