# Praaline Plugin
# DisMo Annotator
# (c) George Christodoulides 2012-2014

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_DISMO_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = DisMod
} else {
    TARGET = DisMo
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

unix {
    DEFINES += HAVE_CRFPP_CONFIG_H
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
LIBS +=  \
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    crfpp/common.h \
    crfpp/config.h \
    crfpp/crfpp.h \
    crfpp/darts.h \
    crfpp/encoder.h \
    crfpp/feature_cache.h \
    crfpp/feature_index.h \
    crfpp/freelist.h \
    crfpp/lbfgs.h \
    crfpp/mmap.h \
    crfpp/node.h \
    crfpp/param.h \
    crfpp/path.h \
    crfpp/scoped_ptr.h \
    crfpp/stream_wrapper.h \
    crfpp/tagger.h \
    crfpp/thread.h \
    crfpp/timer.h \
    crfpp/winmain.h \
    dismo/CRFFeatureSet.h \
    dismo/BoundaryDetector.h \
    dismo/CRFAnnotator.h \
    dismo/DictionaryEntry.h \
    dismo/DictionaryFST.h \
    dismo/DictionaryQuery.h \
    dismo/DictionarySQL.h \
    dismo/DiscourseTagger.h \
    dismo/DisfluencyDetector.h \
    dismo/DisMoAnnotator.h \
    dismo/DisMoEvaluator.h \
    dismo/DisMoTrainer.h \
    dismo/POSTagger.h \
    dismo/PostProcessor.h \
    dismo/PreProcessor.h \
    dismo/Token.h \
    dismo/Tokenizer.h \
    dismo/TokenList.h \
    dismo/TokenUnit.h \
    serialisers/DisMoSerialiserSQL.h \
    serialisers/DisMoSerialiserTiers.h \
    PluginDisMo.h \
    PluginDisMo_global.h \
    dismo/DisMoConfiguration.h

SOURCES += \
    plugindismo.cpp \
    crfpp/encoder.cpp \
    crfpp/feature.cpp \
    crfpp/feature_cache.cpp \
    crfpp/feature_index.cpp \
    crfpp/lbfgs.cpp \
    crfpp/libcrfpp.cpp \
    crfpp/node.cpp \
    crfpp/param.cpp \
    crfpp/path.cpp \
    crfpp/tagger.cpp \
    dismo/CRFFeatureSet.cpp \
    dismo/BoundaryDetector.cpp \
    dismo/CRFAnnotator.cpp \
    dismo/DictionaryFST.cpp \
    dismo/DictionarySQL.cpp \
    dismo/DiscourseTagger.cpp \
    dismo/DisfluencyDetector.cpp \
    dismo/DisMoAnnotator.cpp \
    dismo/DisMoEvaluator.cpp \
    dismo/DisMoTrainer.cpp \
    dismo/POSTagger.cpp \
    dismo/PostProcessor.cpp \
    dismo/PreProcessor.cpp \
    dismo/Token.cpp \
    dismo/Tokenizer.cpp \
    dismo/TokenList.cpp \
    dismo/TokenUnit.cpp \
    serialisers/DisMoSerialiserSQL.cpp \
    serialisers/DisMoSerialiserTiers.cpp \
    dismo/DisMoConfiguration.cpp
