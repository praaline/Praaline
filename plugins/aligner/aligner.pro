# Praaline Plugin
# Forced Alignment
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_ALIGNER_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Alignerd
} else {
    TARGET = Aligner
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
DESTDIR += ../../../../../../app/build/plugins/

INCLUDEPATH += . .. ../.. ../../pncore ../../app/annotation
DEPENDPATH += . .. ../.. ../../pncore

# Praaline core has a dependency on VAMP SDK (only for RealTime conversion)
win32-g++ {
    INCLUDEPATH += ../../sv-dependency-builds/win32-mingw/include
    LIBS += -L../../sv-dependency-builds/win32-mingw/lib
}
win32-msvc* {
    INCLUDEPATH += ../../sv-dependency-builds/win32-msvc/include
    LIBS += -L../../sv-dependency-builds/win32-msvc/lib
}
macx* {
    INCLUDEPATH += ../../sv-dependency-builds/osx/include
    LIBS += -L../../sv-dependency-builds/osx/lib
}

# Qtilities configuration
QTILITIES += extension_system
include(../../external/qtilities/src/Qtilities.pri)
INCLUDEPATH += ../../external/qtilities/include


# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}
# Application components
LIBS += -L../../pnlib/$${COMPONENTSPATH} -lpnlib \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \ 
    pluginaligner_global.h \
    pluginaligner.h \
    alignertoken.h \
    phonemedatabase.h \
    easyalignbasic.h \
    annotationpluginpraatscript.h \
    abstractphonetiser.h \
    abstractacousticmodeltrainer.h \
    abstractacousticmodeladapter.h \
    abstractlanguagemodeltrainer.h \
    abstractfeatureextractor.h \
    abstractforcedaligner.h \
    abstractrecogniser.h \
    htk/htkacousticmodeladapter.h \
    sphinx/sphinxacousticmodeladapter.h \
    htk/htkacousticmodeltrainer.h \
    sphinx/sphinxfeatureextractor.h \
    sphinx/sphinxrecogniser.h \
    abstractsyllabifier.h \
    phonetisers/rulebasedphonetiser.h \
    phonetisers/externalphonetiser.h \
    sphinx/sphinxsegmentation.h \
    htk/htkforcedaligner.h
	
SOURCES += \ 
    pluginaligner.cpp \
    phonemedatabase.cpp \
    easyalignbasic.cpp \
    annotationpluginpraatscript.cpp \
    htk/htkacousticmodeladapter.cpp \
    sphinx/sphinxacousticmodeladapter.cpp \
    htk/htkacousticmodeltrainer.cpp \
    sphinx/sphinxfeatureextractor.cpp \
    sphinx/sphinxrecogniser.cpp \
    phonetisers/rulebasedphonetiser.cpp \
    phonetisers/externalphonetiser.cpp \
    sphinx/sphinxsegmentation.cpp \
    htk/htkforcedaligner.cpp
