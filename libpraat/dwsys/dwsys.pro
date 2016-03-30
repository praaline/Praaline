! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += staticlib
TARGET = dwsys

INCLUDEPATH += ../stat ../num ../sys ../external/gsl ../kar


SOURCES = Collection_extensions.cpp Command.cpp \
        DLL.cpp Eigen.cpp FileInMemory.cpp Graphics_extensions.cpp Index.cpp \
        NUM2.cpp NUMhuber.cpp NUMlapack.cpp NUMmachar.cpp \
        NUMf2c.cpp NUMcblas.cpp NUMclapack.cpp NUMfft_d.cpp NUMsort2.cpp \
        NUMmathlib.cpp NUMstring.cpp \
        Permutation.cpp Permutation_and_Index.cpp \
        regularExp.cpp SimpleVector.cpp Simple_extensions.cpp \
        SVD.cpp

HEADERS += \
    Collection_extensions.h \
    Command.h \
    DLL.h \
    Eigen.h \
    Eigen_def.h \
    FileInMemory.h \
    Graphics_extensions.h \
    Index.h \
    Index_def.h \
    NUM2.h \
    NUMcblas.h \
    NUMclapack.h \
    NUMf2c.h \
    NUMfft_core.h \
    NUMlapack.h \
    NUMmachar.h \
    Permutation.h \
    Permutation_and_Index.h \
    Permutation_def.h \
    regularExp.h \
    Simple_extensions.h \
    SimpleVector.h \
    SimpleVector_def.h \
    SVD.h \
    SVD_def.h
