# Praaline Automatic Speech Recognition (ASR) Library
# (c) George Christodoulides 2012-2020

! include( ../common.pri ) {
    ! include( ./praaline-asr-common.pri ) {
        error( Could not find the common.pri file! )
    }
}

TEMPLATE = lib

CONFIG(debug, debug|release) {
    TARGET = praaline-asrd
} else {
    TARGET = praaline-asr
}

CONFIG += qt thread warn_on stl rtti exceptions c++11

QT -= gui

DEFINES += LIBRARY_PRAALINE_ASR
DEFINES += USE_NAMESPACE_PRAALINE_ASR

INCLUDEPATH += . external src include
DEPENDPATH += . external src

DEFINES += USE_NAMESPACE_PRAALINE_CORE
INCLUDEPATH += ../praaline-core/include

DEFINES += USE_NAMESPACE_PRAALINE_MEDIA
INCLUDEPATH += ../praaline-media/include

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

# The Praaline ASR library depends on Praaline Core and on Praaline Media
LIBS +=  \
        -L../praaline-media/$${COMPONENTSPATH} -lpraaline-media$${PRAALINE_LIB_POSTFIX} \
        -L../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../praaline-media/$${COMPONENTSPATH}/libpraaline-media$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}
        ../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    include/PraalineASR/PraalineASR_Global.h \
    include/PraalineASR/SpeechToken.h \
    include/PraalineASR/AcousticModelAdapter.h \
    include/PraalineASR/AcousticModelTrainer.h \
    include/PraalineASR/SpeechRecogniser.h \
    include/PraalineASR/LanguageModelTrainer.h \
    include/PraalineASR/ForcedAligner.h \
    include/PraalineASR/Phonetiser.h \
    include/PraalineASR/AcousticFeatureExtractor.h \
    include/PraalineASR/Syllabifier.h \
    include/PraalineASR/SpeechRecognitionRecipes.h \
    include/PraalineASR/Dummy/DummyForcedAligner.h \
    include/PraalineASR/Dummy/SyllableDetectionForcedAligner.h \
    include/PraalineASR/HTK/HTKForcedAligner.h \
    include/PraalineASR/HTK/HTKAcousticModelTrainer.h \
    include/PraalineASR/HTK/HTKAcousticModelAdapter.h \
    include/PraalineASR/Kaldi/KaldiForcedAligner.h \
    include/PraalineASR/Kaldi/KaldiConfiguration.h \
    include/PraalineASR/Kaldi/MFAExternal.h \
    include/PraalineASR/Sphinx/SphinxSegmentation.h \
    include/PraalineASR/Sphinx/SphinxRecogniser.h \
    include/PraalineASR/Sphinx/SphinxPronunciationDictionary.h \
    include/PraalineASR/Sphinx/SphinxFeatureExtractor.h \
    include/PraalineASR/Sphinx/SphinxAcousticModelAdapter.h \
    include/PraalineASR/Sphinx/SphinxConfiguration.h \
    include/PraalineASR/Sphinx/SphinxOfflineRecogniser.h \
    include/PraalineASR/Sphinx/SphinxAcousticModelTrainer.h \
    include/PraalineASR/Sphinx/SphinxLanguageModelBuilder.h \
    include/PraalineASR/Sphinx/SphinxContinuousRecogniser.h \
    include/PraalineASR/Sphinx/SphinxLongSoundAligner.h \
    include/PraalineASR/Sphinx/SphinxAutoTranscriber.h \
    include/PraalineASR/Syllabifier/SyllabifierEasy.h \
    include/PraalineASR/Phonetiser/ExternalPhonetiser.h \
    include/PraalineASR/Phonetiser/DictionaryPhonetiser.h \
    include/PraalineASR/Phonetiser/PhonemeTranslation.h \
    include/PraalineASR/Phonetiser/PhoneticDictionary.h \
    include/PraalineASR/Phonetiser/RuleBasedPhonetiser.h

SOURCES += \
    src/ForcedAligner.cpp \
    src/SpeechRecognitionRecipes.cpp \
    src/Dummy/DummyForcedAligner.cpp \
    src/Dummy/SyllableDetectionForcedAligner.cpp \
    src/HTK/HTKForcedAligner.cpp \
    src/HTK/HTKAcousticModelTrainer.cpp \
    src/HTK/HTKAcousticModelAdapter.cpp \
    src/Kaldi/KaldiForcedAligner.cpp \
    src/Kaldi/KaldiConfiguration.cpp \
    src/Kaldi/MFAExternal.cpp \
    src/Sphinx/SphinxSegmentation.cpp \
    src/Sphinx/SphinxRecogniser.cpp \
    src/Sphinx/SphinxPronunciationDictionary.cpp \
    src/Sphinx/SphinxFeatureExtractor.cpp \
    src/Sphinx/SphinxAcousticModelAdapter.cpp \
    src/Sphinx/SphinxConfiguration.cpp \
    src/Sphinx/SphinxOfflineRecogniser.cpp \
    src/Sphinx/SphinxAcousticModelTrainer.cpp \
    src/Sphinx/SphinxLanguageModelBuilder.cpp \
    src/Sphinx/SphinxContinuousRecogniser.cpp \
    src/Sphinx/SphinxLongSoundAligner.cpp \
    src/Sphinx/SphinxAutoTranscriber.cpp \
    src/Phonetiser/RuleBasedPhonetiser.cpp \
    src/Phonetiser/ExternalPhonetiser.cpp \
    src/Phonetiser/DictionaryPhonetiser.cpp \
    src/Phonetiser/PhonemeTranslation.cpp \
    src/Phonetiser/PhoneticDictionary.cpp \
    src/Syllabifier/SyllabifierEasy.cpp

