# Praaline Plugin
# Web interface with Django
# (c) George Christodoulides 2020

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_WEBDJANGO_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = WebDjangod
} else {
    TARGET = WebDjango
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../app/interfaces
DEPENDPATH += . .. ../..

# Dependency: PraalineCore
DEFINES += USE_NAMESPACE_PRAALINE_CORE
INCLUDEPATH += ../../praaline-core/include
DEPENDPATH += ../../praaline-core

# Qtilities include for plugin interfaces
QTILITIES += extension_system
QTILITIES_SOURCE_BASE  = $$PWD/../../dependencies/qtilities
QTILITIES_BUILD_BASE   = $$OUT_PWD/../../dependencies/qtilities
QTILITIES_DEPENDENCIES = $$OUT_PWD/../../dependencies
include(../../dependencies/qtilities/src/Qtilities.pri)


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
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    MetadataDjangoModels.h \
    pluginwebdjango_global.h \
    pluginwebdjango.h
	
SOURCES += \
    MetadataDjangoModels.cpp \
    pluginwebdjango.cpp