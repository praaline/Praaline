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
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../pncore ../../app/interfaces
DEPENDPATH += . .. ../.. ../../pncore

# Qtilities include for plugin interfaces
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesCore
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesCoreGui
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesLogging
INCLUDEPATH += ../../dependencies/qtilities/include/QtilitiesExtensionSystem


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
    annotationpluginpraatscript.h \
    pluginprosobox5.h \
    attributenametranslation.h \
    ProsodicBoundariesAnnotator.h \
    SyllableProminenceAnnotator.h \
    SpeechRateEstimator.h \
    PluginPromise.h \
    PluginPromise_global.h
	
SOURCES += \ 
    annotationpluginpraatscript.cpp \
    pluginprosobox5.cpp \
    attributenametranslation.cpp \
    ProsodicBoundariesAnnotator.cpp \
    SyllableProminenceAnnotator.cpp \
    SpeechRateEstimator.cpp \
    PluginPromise.cpp
	
