# Praaline Plugin
# DisMo Annotator
# (c) George Christodoulides 2012-2014

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_DISMO2_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = DisMo2d
} else {
    TARGET = DisMo2
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
    plugindismo2.h \
    plugindismo2_global.h \
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
    dismo/tokenbase.h
	
SOURCES += \
    plugindismo2.cpp \
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
    dismo/tokenbase.cpp
