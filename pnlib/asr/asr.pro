# Praaline
# (c) George Christodoulides 2012-2016

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib qt thread warn_on stl rtti exceptions c++11

INCLUDEPATH += . .. ../.. ../../pncore
DEPENDPATH += . .. ../.. ../../pncore

TARGET = praaline-asr

# Linking dynamically with PocketSphinx
win32-g++ {
    POCKETSPHINX_BASE_PATH = $$PWD/../../dependency-builds/pn/win32-mingw
}
win32-msvc* {
    POCKETSPHINX_BASE_PATH = $$PWD/../../dependency-builds/pn/win32-msvc
}
unix {
    POCKETSPHINX_BASE_PATH = /usr/local
}

INCLUDEPATH += $${POCKETSPHINX_BASE_PATH}/include/pocketsphinx \
               $${POCKETSPHINX_BASE_PATH}/include/sphinxbase \
               $${POCKETSPHINX_BASE_PATH}/include/
win32 {
    LIBS += -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase -liconv
}
unix {
    LIBS += -L$${POCKETSPHINX_BASE_PATH}/lib -lpocketsphinx -lsphinxbase
}

LIBS +=  \
        -L../media/$${COMPONENTSPATH} -lpraaline-media \
        $$LIBS
PRE_TARGETDEPS += \
        ../media/$${COMPONENTSPATH}/libpraaline-media.a

HEADERS += \
    SpeechToken.h \
    SpeechRecognitionRecipes.h \
    sphinx/SphinxSegmentation.h \
    sphinx/SphinxRecogniser.h \
    sphinx/SphinxPronunciationDictionary.h \
    sphinx/SphinxFeatureExtractor.h \
    sphinx/SphinxAcousticModelAdapter.h \
    htk/HTKForcedAligner.h \
    htk/HTKAcousticModelTrainer.h \
    htk/HTKAcousticModelAdapter.h \
    phonetiser/ExternalPhonetiser.h \
    sphinx/SphinxConfiguration.h \
    sphinx/SphinxOfflineRecogniser.h \
    sphinx/SphinxAcousticModelTrainer.h \
    AcousticModelAdapter.h \
    AcousticModelTrainer.h \
    SpeechRecogniser.h \
    LanguageModelTrainer.h \
    ForcedAligner.h \
    Phonetiser.h \
    AcousticFeatureExtractor.h \
    Syllabifier.h \
    sphinx/SphinxLanguageModelBuilder.h \
    sphinx/SphinxContinuousRecogniser.h \
    sphinx/SphinxLongSoundAligner.h \
    sphinx/SphinxAutoTranscriber.h \
    syllabifier/SyllabifierEasy.h \
    phonetiser/DictionaryPhonetiser.h \
    phonetiser/PhonemeTranslation.h \
    phonetiser/PhoneticDictionary.h \
    phonetiser/RuleBasedPhonetiser.h \
    kaldi/KaldiForcedAligner.h \
    kaldi/KaldiConfiguration.h \
    kaldi/MFAExternal.h \
    dummy/DummyForcedAligner.h \
    dummy/SyllableDetectionForcedAligner.h

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
    sphinx/SphinxConfiguration.cpp \
    sphinx/SphinxOfflineRecogniser.cpp \
    sphinx/SphinxAcousticModelTrainer.cpp \
    sphinx/SphinxLanguageModelBuilder.cpp \
    sphinx/SphinxContinuousRecogniser.cpp \
    sphinx/SphinxLongSoundAligner.cpp \
    sphinx/SphinxAutoTranscriber.cpp \
    syllabifier/SyllabifierEasy.cpp \
    phonetiser/DictionaryPhonetiser.cpp \
    phonetiser/PhonemeTranslation.cpp \
    phonetiser/PhoneticDictionary.cpp \
    kaldi/KaldiForcedAligner.cpp \
    kaldi/KaldiConfiguration.cpp \
    kaldi/MFAExternal.cpp \
    dummy/DummyForcedAligner.cpp \
    dummy/SyllableDetectionForcedAligner.cpp \
    ForcedAligner.cpp

SUBDIRS += \
    phonetiser/phonetiser.pro

