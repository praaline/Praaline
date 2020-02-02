# Praaline Media Library
# (c) George Christodoulides 2012-2020

! include( ../common.pri ) {
    ! include( ./praaline-media-common.pri ) {
        error( Could not find the common.pri file! )
    }
}

TEMPLATE = lib

CONFIG(debug, debug|release) {
    TARGET = praaline-mediad
} else {
    TARGET = praaline-media
}

CONFIG += qt thread warn_on stl rtti exceptions c++11

QT -= gui

DEFINES += LIBRARY_PRAALINE_MEDIA
DEFINES += USE_NAMESPACE_PRAALINE_MEDIA

INCLUDEPATH += . external src include
DEPENDPATH += . external src

DEFINES += USE_NAMESPACE_PRAALINE_CORE
INCLUDEPATH += ../praaline-core/include

# The Praaline Media library depends on Praaline Core
LIBS +=  \
        -L../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    include/PraalineMedia/PraalineMedia_Global.h \
    include/PraalineMedia/SoundInfo.h \
    include/PraalineMedia/AudioSegmenter.h \
    include/PraalineMedia/SoxExternal.h

SOURCES += \
    src/SoundInfo.cpp \
    src/AudioSegmenter.cpp \
    src/SoxExternal.cpp
