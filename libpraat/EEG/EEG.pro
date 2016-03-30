! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = EEG

INCLUDEPATH += ../num ../kar ../sys ../dwsys ../stat ../dwtools ../fon

SOURCES = EEG.cpp EEGWindow.cpp ERPTier.cpp ERP.cpp ERPWindow.cpp \
   praat_EEG.cpp manual_EEG.cpp

HEADERS += \
    EEG.h \
    EEG_def.h \
    EEGWindow.h \
    EEGWindow_prefs.h \
    ERP.h \
    ERP_def.h \
    ERPTier.h \
    ERPTier_def.h \
    ERPWindow.h \
    ERPWindow_prefs.h
