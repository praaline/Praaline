# Praaline Plugin
# DisMo Annotator
# (c) George Christodoulides 2012-2014

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_DISMO_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = DisMod
} else {
    TARGET = DisMo
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

unix {
    DEFINES += HAVE_CRFPP_CONFIG_H
}

# Build folder organisation
CONFIG( debug, debug|release ) {
    # debug
    COMPONENTSPATH = build/debug
} else {
    # release
    COMPONENTSPATH = build/release
}
# Application components
LIBS +=  \
        -L../../pncore/$${COMPONENTSPATH} -lpncore$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../pncore/$${COMPONENTSPATH}/libpncore$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    plugindismo.h \
    plugindismo_global.h \
    crfpp/common.h \
    crfpp/config.h \
    crfpp/crfpp.h \
    crfpp/darts.h \
    crfpp/encoder.h \
    crfpp/feature_cache.h \
    crfpp/feature_index.h \
    crfpp/freelist.h \
    crfpp/lbfgs.h \
    crfpp/mmap.h \
    crfpp/node.h \
    crfpp/param.h \
    crfpp/path.h \
    crfpp/scoped_ptr.h \
    crfpp/stream_wrapper.h \
    crfpp/tagger.h \
    crfpp/thread.h \
    crfpp/timer.h \
    crfpp/winmain.h \
    dismo/boundarydetector.h \
    dismo/crfannotator.h \
    dismo/dictionaryentry.h \
    dismo/dictionaryfst.h \
    dismo/dictionaryquery.h \
    dismo/dictionarysql.h \
    dismo/discoursetagger.h \
    dismo/disfluencydetector.h \
    dismo/dismoannotator.h \
    dismo/dismoevaluator.h \
    dismo/dismotrainer.h \
    dismo/postagger.h \
    dismo/postprocessor.h \
    dismo/preprocessor.h \
    dismo/token.h \
    dismo/tokenizer.h \
    dismo/tokenlist.h \
    dismo/tokenunit.h \
    serialisers/dismoserialisersql.h \
    serialisers/dismoserialisertiers.h

SOURCES += \
    plugindismo.cpp \
    crfpp/encoder.cpp \
    crfpp/feature.cpp \
    crfpp/feature_cache.cpp \
    crfpp/feature_index.cpp \
    crfpp/lbfgs.cpp \
    crfpp/libcrfpp.cpp \
    crfpp/node.cpp \
    crfpp/param.cpp \
    crfpp/path.cpp \
    crfpp/tagger.cpp \
    dismo/boundarydetector.cpp \
    dismo/crfannotator.cpp \
    dismo/dictionaryfst.cpp \
    dismo/dictionarysql.cpp \
    dismo/discoursetagger.cpp \
    dismo/disfluencydetector.cpp \
    dismo/dismoannotator.cpp \
    dismo/dismoevaluator.cpp \
    dismo/dismotrainer.cpp \
    dismo/postagger.cpp \
    dismo/postprocessor.cpp \
    dismo/preprocessor.cpp \
    dismo/token.cpp \
    dismo/tokenizer.cpp \
    dismo/tokenlist.cpp \
    dismo/tokenunit.cpp \
    serialisers/dismoserialisersql.cpp \
    serialisers/dismoserialisertiers.cpp
