! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = gram

INCLUDEPATH += ../num ../kar ../sys ../dwsys ../stat ../dwtools ../fon

SOURCES = Network.cpp \
   OTGrammar.cpp OTGrammarEditor.cpp manual_gram.cpp praat_gram.cpp OTMulti.cpp OTMultiEditor.cpp \
   OTGrammar_ex_metrics.cpp OTGrammar_ex_NoCoda.cpp OTGrammar_ex_NPA.cpp OTGrammar_ex_tongueRoot.cpp \
   OTMulti_ex_metrics.cpp

HEADERS += \
    Network.h \
    Network_def.h \
    Network_enums.h \
    OTGrammar.h \
    OTGrammar_def.h \
    OTGrammar_enums.h \
    OTGrammarEditor.h \
    OTMulti.h \
    OTMulti_def.h \
    OTMultiEditor.h

