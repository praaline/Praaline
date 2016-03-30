! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = stat

INCLUDEPATH += ../num ../kar ../sys ../dwsys ../dwtools ../fon ../stat

SOURCES = Table.cpp TableEditor.cpp Regression.cpp manual_statistics.cpp praat_Stat.cpp \
   TableOfReal.cpp Distributions.cpp Distributions_and_Strings.cpp PairDistribution.cpp \
   LogisticRegression.cpp

HEADERS += \
    Distributions.h \
    Distributions_and_Strings.h \
    LogisticRegression.h \
    LogisticRegression_def.h \
    PairDistribution.h \
    PairDistribution_def.h \
    Regression.h \
    Regression_def.h \
    Table.h \
    Table_def.h \
    TableEditor.h \
    TableOfReal.h \
    TableOfReal_def.h
