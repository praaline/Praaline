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

# Qtilities include for plugin interfaces
QTILITIES += extension_system
QTILITIES_SOURCE_BASE  = $$PWD/../../dependencies/qtilities
QTILITIES_BUILD_BASE   = $$OUT_PWD/../../dependencies/qtilities
QTILITIES_DEPENDENCIES = $$OUT_PWD/../../dependencies
include(../../dependencies/qtilities/src/Qtilities.pri)

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
    corpus-specific/LOCASF.h \
    experiments/SpeechRateExperiments.h \
    experiments/TappingAnnotatorExperiment.h \
    experiments/MyExperiments.h \
    experiments/ProsodicBoundariesExperimentPreparation.h \
    experiments/ProsodicBoundariesExperimentAnalysis.h \
    experiments/DisfluenciesExperiments.h \
    experiments/StutteredSpeechAlignmentExperiments.h \
    experiments/MelissaExperiment.h \
    experiments/MacroprosodyExperiment.h \
    experiments/ExperimentUtterances.h \
    experiments/AggregateProsody.h \
    ProsodicBoundaries.h \
    CorpusCommunicationSplicer.h \
    corpus-specific/NCCFR.h \
    experiments/PhonogenreDiscourseMarkers.h \
    PhonoSeesaw.h \
    JsonAlignedTranscription.h \
    IntervalTierCombinations.h \
    corpus-specific/ORFEO.h


SOURCES += \ 
    pluginvaria.cpp \
    crfannotator.cpp \
    chunkannotator.cpp \
    svbridge.cpp \
    InterraterAgreement.cpp \
    ProsodyCourse.cpp \
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
    corpus-specific/LOCASF.cpp \
    experiments/MyExperiments.cpp \
    experiments/DisfluenciesExperiments.cpp \
    experiments/SpeechRateExperiments.cpp \
    experiments/TappingAnnotatorExperiment.cpp \
    experiments/StutteredSpeechAlignmentExperiments.cpp \
    experiments/ProsodicBoundariesExperimentPreparation.cpp \
    experiments/ProsodicBoundariesExperimentAnalysis.cpp \
    experiments/MelissaExperiment.cpp \
    experiments/MacroprosodyExperiment.cpp \
    experiments/ExperimentUtterances.cpp \
    experiments/AggregateProsody.cpp \
    ProsodicBoundaries.cpp \
    CorpusCommunicationSplicer.cpp \
    corpus-specific/NCCFR.cpp \
    experiments/PhonogenreDiscourseMarkers.cpp \
    PhonoSeesaw.cpp \
    JsonAlignedTranscription.cpp \
    IntervalTierCombinations.cpp \
    corpus-specific/ORFEO.cpp
