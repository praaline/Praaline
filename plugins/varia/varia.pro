# Praaline Plugin
# Temporal variables for prosodic analysis
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_VARIA_LIBRARY

QT += gui sql
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
include(../../libs/qtilities/src/Qtilities.pri)
INCLUDEPATH += ../../libs/qtilities/include


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
        -L../../svcore/$${COMPONENTSPATH} -lsvcore \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginvaria_global.h \
    pluginvaria.h \
    crfannotator.h \
    chunkannotator.h \
    svbridge.h \
    prosodicboundaries.h \
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
    SyllabifierEasy.h \
    ForcedAlignerDummy.h \
    SequencerProsodicUnits.h \
    SequencerCombineUnits.h \
    CPROMDISS.h


SOURCES += \ 
    pluginvaria.cpp \
    crfannotator.cpp \
    chunkannotator.cpp \
    svbridge.cpp \
    prosodicboundaries.cpp \
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
    SyllabifierEasy.cpp \
    ForcedAlignerDummy.cpp \
    SequencerProsodicUnits.cpp \
    SequencerCombineUnits.cpp \
    CPROMDISS.cpp
