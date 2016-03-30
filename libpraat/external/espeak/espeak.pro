! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = espeak

INCLUDEPATH += ../../num ../../kar ../../stat ../../sys ../../dwsys

SOURCES = \
    compiledict.cpp \
	dictionary.cpp \
	intonation.cpp \
	klatt.cpp  \
    numbers.cpp \
	phonemelist.cpp \
	readclause.cpp \
	setlengths.cpp  \
    sonic.cpp \
	speak_lib.cpp \
	synthdata.cpp \
	synthesize.cpp  \
    synth_mbrola.cpp \
	tr_languages.cpp \
	translate.cpp \
	voices.cpp \
	wavegen.cpp \
    espeakdata_phons.cpp \
	espeakdata_dicts.cpp \
	espeakdata_voices.cpp \
	espeakdata_variants.cpp \
    espeakdata_FileInMemory.cpp
	
	

HEADERS += \
    debug.h \
    espeak_command.h \
    espeakdata_FileInMemory.h \
    event.h \
    fifo.h \
    klatt.h \
    phoneme.h \
    sintab.h \
    sonic.h \
    speak_lib.h \
    speech.h \
    StdAfx.h \
    synthesize.h \
    translate.h \
    voice.h \
    wave.h