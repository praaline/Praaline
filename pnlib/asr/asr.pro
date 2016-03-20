# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

TARGET = praaline-asr

HEADERS += \
    SpeechToken.h \
    SpeechRecognitionRecipes.h \
    AbstractSpeechRecogniser.h \
    AbstractLanguageModelTrainer.h \
    AbstractForcedAligner.h \
    AbstractFeatureExtractor.h \
    AbstractAcousticModelTrainer.h \
    AbstractAcousticModelAdapter.h \
    sphinx/SphinxSegmentation.h \
    sphinx/SphinxRecogniser.h \
    sphinx/SphinxPronunciationDictionary.h \
    sphinx/SphinxFeatureExtractor.h \
    sphinx/SphinxAcousticModelAdapter.h \
    htk/HTKForcedAligner.h \
    htk/HTKAcousticModelTrainer.h \
    htk/HTKAcousticModelAdapter.h \
    phonetiser/RuleBasedPhonetiser.h \
    phonetiser/ExternalPhonetiser.h \
    phonetiser/DictionaryPhonetiser.h \
    phonetiser/AbstractPhonetiser.h \
    syllabifier/AbstractSyllabifier.h

SOURCES += \
    SpeechRecognitionRecipes.cpp \
    sphinx/SphinxSegmentation.cpp \
    sphinx/SphinxRecogniser.cpp \
    sphinx/SphinxPronunciationDictionary.cpp \
    sphinx/SphinxFeatureExtractor.cpp \
    sphinx/SphinxAcousticModelAdapter.cpp \
    htk/HTKForcedAligner.cpp \
    htk/HTKAcousticModelTrainer.cpp \
    htk/HTKAcousticModelAdapter.cpp \
    phonetiser/RuleBasedPhonetiser.cpp \
    phonetiser/ExternalPhonetiser.cpp \
    phonetiser/DictionaryPhonetiser.cpp

SUBDIRS += \
    phonetiser/phonetiser.pro

