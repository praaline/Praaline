# Praaline Plugin
# FLORAL Project - PFC corpus
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_FLORALPFC_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = FloralPFCd
} else {
    TARGET = FloralPFC
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

# Dependency: PraalineASR
DEFINES += USE_NAMESPACE_PRAALINE_ASR
INCLUDEPATH += ../../praaline-asr/include
DEPENDPATH += ../../praaline-asr

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
        -L../../praaline-asr/$${COMPONENTSPATH} -lpraaline-asr$${PRAALINE_LIB_POSTFIX} \
        -L../../praaline-media/$${COMPONENTSPATH} -lpraaline-media$${PRAALINE_LIB_POSTFIX} \
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX}
        $$LIBS
PRE_TARGETDEPS += \
        ../../praaline-asr/$${COMPONENTSPATH}/libpraaline-asr$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-media/$${COMPONENTSPATH}/libpraaline-media$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    valibelprocessor.h \
    PFCPreprocessor.h \
    PFCPhonetiser.h \
    PFCAligner.h \
    PluginFloralPFC.h \
    PluginFloralPFC_global.h \
    PFCReports.h \
    PFCAlignmentEvaluation.h
	
SOURCES += \ 
    valibelprocessor.cpp \
    PFCPreprocessor.cpp \
    PFCPhonetiser.cpp \
    PFCAligner.cpp \
    PluginFloralPFC.cpp \
    PFCReports.cpp \
    PFCAlignmentEvaluation.cpp
