# Praaline Plugin
# Temporal variables for prosodic analysis
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_VARIA_LIBRARY

QT += gui sql xml
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

INCLUDEPATH += . .. ../.. ../../pncore ../../app/interfaces ../../svcore
DEPENDPATH += . .. ../.. ../../pncore ../../app/interfaces ../../svcore

# Qtilities configuration
QTILITIES += extension_system
include(../../dependencies/qtilities/src/Qtilities.pri)
INCLUDEPATH += ../../dependencies/qtilities/include

# Application components
LIBS +=  \
        -L../../svcore/$${COMPONENTSPATH} -lsvcore \
        -L../../pnlib/asr/$${COMPONENTSPATH} -lpraaline-asr \
        -L../../pnlib/media/$${COMPONENTSPATH} -lpraaline-media \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../svcore/$${COMPONENTSPATH}/libsvcore.a \
        ../../pnlib/asr/$${COMPONENTSPATH}/libpraaline-asr.a \
        ../../pnlib/media/$${COMPONENTSPATH}/libpraaline-media.a \
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginvaria_global.h \
    pluginvaria.h \
    crfannotator.h \
    chunkannotator.h \
    svbridge.h \
    InterraterAgreement.h \
    ProsodyCourse.h \
    MyExperiments.h \
    DisfluenciesExperiments.h \
    SpeechRateExperiments.h \
    TappingAnnotatorExperiment.h \
    XMLTranscription.h \
    ProsodicBoundariesExperimentPreparation.h \
    ProsodicBoundariesExperimentAnalysis.h \
    StutteredSpeechAlignmentExperiments.h \
    MelissaExperiment.h \
    ForcedAligner.h \
    MacroprosodyExperiment.h \
    BratAnnotationExporter.h \
    PhonetiserExternal.h \
    SequencerSyntax.h \
    SequencerDisfluencies.h \
    BratSyntaxAndDisfluencies.h \
    ForcedAlignerDummy.h \
    SequencerProsodicUnits.h \
    SequencerCombineUnits.h \
    SilentPauseManipulator.h \
    ExperimentUtterances.h \
    ProsodicUnits.h \
    AggregateProsody.h \
    LanguageDefinition.h \
    corpus-specific/CPROMDISS.h \
    corpus-specific/PFCAlignment.h \
    corpus-specific/Rhapsodie.h \
    corpus-specific/LOCASF.h \
    ProsodicBoundaries.h


SOURCES += \ 
    pluginvaria.cpp \
    crfannotator.cpp \
    chunkannotator.cpp \
    svbridge.cpp \
    InterraterAgreement.cpp \
    ProsodyCourse.cpp \
    MyExperiments.cpp \
    DisfluenciesExperiments.cpp \
    SpeechRateExperiments.cpp \
    TappingAnnotatorExperiment.cpp \
    XMLTranscription.cpp \
    ProsodicBoundariesExperimentPreparation.cpp \
    ProsodicBoundariesExperimentAnalysis.cpp \
    StutteredSpeechAlignmentExperiments.cpp \
    MelissaExperiment.cpp \
    ForcedAligner.cpp \
    MacroprosodyExperiment.cpp \
    BratAnnotationExporter.cpp \
    PhonetiserExternal.cpp \
    SequencerSyntax.cpp \
    SequencerDisfluencies.cpp \
    BratSyntaxAndDisfluencies.cpp \
    ForcedAlignerDummy.cpp \
    SequencerProsodicUnits.cpp \
    SequencerCombineUnits.cpp \
    SilentPauseManipulator.cpp \
    ExperimentUtterances.cpp \
    ProsodicUnits.cpp \
    AggregateProsody.cpp \
    LanguageDefinition.cpp \
    corpus-specific/CPROMDISS.cpp \
    corpus-specific/PFCAlignment.cpp \
    corpus-specific/Rhapsodie.cpp \
    corpus-specific/LOCASF.cpp \
    ProsodicBoundaries.cpp
