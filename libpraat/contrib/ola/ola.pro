! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = ola

INCLUDEPATH += ../../dwtools ../../fon ../../sys ../../dwsys ../../stat ../../num ../../external/gsl
DEFINES += _DEBUG _REENTRANT

SOURCES = KNN.cpp \
   KNN_threads.cpp Pattern_to_Categories_cluster.cpp KNN_prune.cpp FeatureWeights.cpp praat_contrib_Ola_KNN.cpp manual_KNN.cpp

HEADERS += \
    FeatureWeights.h \
    FeatureWeights_def.h \
    KNN.h \
    KNN_def.h \
    KNN_prune.h \
    KNN_threads.h \
    OlaP.h \
    Pattern_to_Categories_cluster.h
