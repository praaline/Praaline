# Praaline Plugin
# Temporal variables for prosodic analysis
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_VARIA_LIBRARY

QT += gui sql xml concurrent
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Variad
} else {
    TARGET = Varia
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../app/interfaces ../../svcore
DEPENDPATH += . .. ../.. ../../app/interfaces ../../svcore

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

# Application components
LIBS +=  \
        -L../../svcore/$${COMPONENTSPATH} -lsvcore \
        -L../../praaline-asr/$${COMPONENTSPATH} -lpraaline-asr$${PRAALINE_LIB_POSTFIX} \
        -L../../praaline-media/$${COMPONENTSPATH} -lpraaline-media$${PRAALINE_LIB_POSTFIX} \
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../svcore/$${COMPONENTSPATH}/libsvcore.a \
        ../../praaline-asr/$${COMPONENTSPATH}/libpraaline-asr$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-media/$${COMPONENTSPATH}/libpraaline-media$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    ExperimentCL.h \
    ExperimentCL_ImportFiles.h \
    SophieScripts.h \
    YizhiScripts.h \
    corpus-specific/ESLOMD.h \
    pluginvaria_global.h \
    pluginvaria.h \
    crfannotator.h \
    chunkannotator.h \
    svbridge.h \
    InterraterAgreement.h \
    XMLTranscription.h \
    BratAnnotationExporter.h \
    PhonetiserExternal.h \
    SequencerSyntax.h \
    SequencerDisfluencies.h \
    BratSyntaxAndDisfluencies.h \
    ForcedAlignerDummy.h \
    SequencerProsodicUnits.h \
    SequencerCombineUnits.h \
    SilentPauseManipulator.h \
    ProsodicUnits.h \
    LanguageDefinition.h \
    corpus-specific/CPROMDISS.h \
    corpus-specific/Rhapsodie.h \
    ProsodicBoundaries.h \
    CorpusCommunicationSplicer.h \
    corpus-specific/NCCFR.h \
    PhonoSeesaw.h \
    JsonAlignedTranscription.h \
    IntervalTierCombinations.h \
    corpus-specific/ORFEO.h


SOURCES += \
    ExperimentCL.cpp \
    ExperimentCL_ImportFiles.cpp \
    SophieScripts.cpp \
    YizhiScripts.cpp \
    corpus-specific/ESLOMD.cpp \
    pluginvaria.cpp \
    crfannotator.cpp \
    chunkannotator.cpp \
    svbridge.cpp \
    InterraterAgreement.cpp \
    XMLTranscription.cpp \
    BratAnnotationExporter.cpp \
    PhonetiserExternal.cpp \
    SequencerSyntax.cpp \
    SequencerDisfluencies.cpp \
    BratSyntaxAndDisfluencies.cpp \
    ForcedAlignerDummy.cpp \
    SequencerProsodicUnits.cpp \
    SequencerCombineUnits.cpp \
    SilentPauseManipulator.cpp \
    ProsodicUnits.cpp \
    LanguageDefinition.cpp \
    corpus-specific/CPROMDISS.cpp \
    corpus-specific/Rhapsodie.cpp \
    ProsodicBoundaries.cpp \
    CorpusCommunicationSplicer.cpp \
    corpus-specific/NCCFR.cpp \
    PhonoSeesaw.cpp \
    JsonAlignedTranscription.cpp \
    IntervalTierCombinations.cpp \
    corpus-specific/ORFEO.cpp
