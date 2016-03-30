! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = artsynth

INCLUDEPATH += ../num ../sys ../fon ../stat

SOURCES = Speaker.cpp Articulation.cpp Artword.cpp \
     Art_Speaker.cpp Art_Speaker_to_VocalTract.cpp Artword_Speaker.cpp Artword_Speaker_Sound.cpp \
     Artword_Speaker_to_Sound.cpp Artword_to_Art.cpp \
     Delta.cpp Speaker_to_Delta.cpp Art_Speaker_Delta.cpp \
     ArtwordEditor.cpp praat_Artsynth.cpp manual_Artsynth.cpp

HEADERS += \
    Art_Speaker.h \
    Art_Speaker_Delta.h \
    Art_Speaker_to_VocalTract.h \
    Articulation.h \
    Articulation_def.h \
    Articulation_enums.h \
    Artword.h \
    Artword_def.h \
    Artword_Speaker.h \
    Artword_Speaker_Sound.h \
    Artword_Speaker_to_Sound.h \
    Artword_to_Art.h \
    ArtwordEditor.h \
    Delta.h \
    Speaker.h \
    Speaker_def.h \
    Speaker_to_Delta.h
