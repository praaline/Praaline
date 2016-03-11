# Praaline
# (c) George Christodoulides 2012-2014

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

QT += xml sql gui
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = pnlib

INCLUDEPATH += . .. ../pncore
DEPENDPATH += . .. ../pncore

HEADERS += \ 
    diff/diff.h \
    diff/diff3.h \
    diff/dtl.h \
    diff/functors.h \
    diff/lcs.h \
    diff/sequence.h \
    diff/ses.h \
    diff/variables.h \
    diff/diffintervals.h \
    asr/sphinx/SphinxAcousticModelAdapter.h \
    asr/sphinx/SphinxFeatureExtractor.h \
    asr/sphinx/SphinxRecogniser.h \
    asr/sphinx/SphinxSegmentation.h \
    asr/htk/HTKAcousticModelAdapter.h \
    asr/htk/HTKAcousticModelTrainer.h \
    asr/htk/HTKForcedAligner.h \
    asr/AbstractAcousticModelAdapter.h \
    asr/AbstractAcousticModelTrainer.h \
    asr/AbstractFeatureExtractor.h \
    asr/AbstractForcedAligner.h \
    phonetiser/ExternalPhonetiser.h \
    phonetiser/RuleBasedPhonetiser.h \
    SpeechToken.h \
    phonetiser/AbstractPhonetiser.h \
    phonetiser/DictionaryPhonetiser.h \
    syllabifier/AbstractSyllabifier.h \
    asr/AbstractLanguageModelTrainer.h \
    asr/AbstractSpeechRecogniser.h \
    asr/SpeechRecognitionRecipes.h \
    vad/OpenSmileVAD.h \
    asr/sphinx/SphinxPronunciationDictionary.h \
    SoundInfo.h \
    AudioSegmenter.h

SOURCES += \ 
    diff/diffintervals.cpp \
    asr/sphinx/SphinxAcousticModelAdapter.cpp \
    asr/sphinx/SphinxFeatureExtractor.cpp \
    asr/sphinx/SphinxRecogniser.cpp \
    asr/sphinx/SphinxSegmentation.cpp \
    asr/htk/HTKAcousticModelAdapter.cpp \
    asr/htk/HTKAcousticModelTrainer.cpp \
    asr/htk/HTKForcedAligner.cpp \
    phonetiser/ExternalPhonetiser.cpp \
    phonetiser/RuleBasedPhonetiser.cpp \
    phonetiser/DictionaryPhonetiser.cpp \
    asr/SpeechRecognitionRecipes.cpp \
    vad/OpenSmileVAD.cpp \
    asr/sphinx/SphinxPronunciationDictionary.cpp \
    SoundInfo.cpp \
    AudioSegmenter.cpp
