! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = LPC

INCLUDEPATH += ../num ../dwtools ../fon ../sys ../dwsys ../stat

SOURCES = Cepstrum.cpp Cepstrumc.cpp Cepstrum_and_Spectrum.cpp \
        Cepstrogram.cpp \
        Formant_extensions.cpp \
        LPC.cpp LPC_and_Cepstrumc.cpp LPC_and_Formant.cpp LPC_and_LFCC.cpp \
        LPC_and_Polynomial.cpp \
        LPC_to_Spectrum.cpp  LPC_to_Spectrogram.cpp \
        LPC_and_Tube.cpp \
        Sound_and_LPC.cpp  Sound_and_LPC_robust.cpp \
        Sound_and_Cepstrum.cpp Tube.cpp \
        VocalTractTier.cpp \
        praat_LPC_init.cpp manual_LPC.cpp

HEADERS += \
    Cepstrogram.h \
    Cepstrum.h \
    Cepstrum_and_Spectrum.h \
    Cepstrumc.h \
    Cepstrumc_def.h \
    Formant_extensions.h \
    LPC.h \
    LPC_and_Cepstrumc.h \
    LPC_and_Formant.h \
    LPC_and_LFCC.h \
    LPC_and_Polynomial.h \
    LPC_and_Tube.h \
    LPC_def.h \
    LPC_to_Spectrogram.h \
    LPC_to_Spectrum.h \
    Sound_and_Cepstrum.h \
    Sound_and_LPC.h \
    Sound_and_LPC_robust.h \
    Tube.h \
    Tube_def.h \
    VocalTractTier.h \
    VocalTractTier_def.h
