! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = FFNet

INCLUDEPATH += ../num ../dwtools ../fon ../sys ../dwsys ../stat

SOURCES = FFNet.cpp \
        FFNet_Eigen.cpp FFNet_Matrix.cpp FFNet_Pattern.cpp \
        FFNet_Activation_Categories.cpp FFNet_Pattern_Activation.cpp \
        FFNet_Pattern_Categories.cpp \
        praat_FFNet_init.cpp manual_FFNet.cpp

HEADERS += \
    FFNet.h \
    FFNet_Activation_Categories.h \
    FFNet_def.h \
    FFNet_Eigen.h \
    FFNet_Matrix.h \
    FFNet_Pattern.h \
    FFNet_Pattern_Activation.h \
    FFNet_Pattern_Categories.h

