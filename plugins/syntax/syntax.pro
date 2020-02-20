# Praaline Plugin
# Temporal variables for prosodic analysis
# (c) George Christodoulides 2014-2015

! include( ../../common.pri ) {
    error( Could not find the common.pri file! )
}

CONFIG += plugin dll qt thread warn_on stl rtti exceptions c++11
TEMPLATE = lib
DEFINES += PLUGIN_SYNTAX_LIBRARY

QT += gui sql
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

CONFIG(debug, debug|release) {
    TARGET = Syntaxd
} else {
    TARGET = Syntax
}

# put plugin in the same directory as Praaline, inside the plugins sub-directory
win32 {
    DESTDIR += ../../../../../../app/build/plugins/
}
else {
    DESTDIR = ../../app/build/plugins/
}

INCLUDEPATH += . .. ../.. ../../app/interfaces
DEPENDPATH += . .. ../..

# Dependency: PraalineCore
DEFINES += USE_NAMESPACE_PRAALINE_CORE
INCLUDEPATH += ../../praaline-core/include
DEPENDPATH += ../../praaline-core

# Qtilities include for plugin interfaces
QTILITIES += extension_system
QTILITIES_SOURCE_BASE  = $$PWD/../../dependencies/qtilities
QTILITIES_BUILD_BASE   = $$OUT_PWD/../../dependencies/qtilities
QTILITIES_DEPENDENCIES = $$OUT_PWD/../../dependencies
include(../../dependencies/qtilities/src/Qtilities.pri)


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
        -L../../praaline-core/$${COMPONENTSPATH} -lpraaline-core$${PRAALINE_LIB_POSTFIX} \
        $$LIBS
PRE_TARGETDEPS += \
        ../../praaline-core/$${COMPONENTSPATH}/libpraaline-core$${PRAALINE_LIB_POSTFIX}.$${LIB_SUFFIX}

HEADERS += \
    pluginsyntax_global.h \
    pluginsyntax.h \
    CoNLLUReader.h \
    CorpusImporter.h \
    DependenciesToLatex.h \
    UDTokenBase.h \
    UDToken.h \
    UDTokenMultiWord.h \
    UDEmptyNode.h \
    UDSentence.h \
    SentencesSplitter.h \
    udpipelib/udpipe.h \
    DictionaryBuilder.h \
    LatexTikzDependencyExporter.h

SOURCES += \
    pluginsyntax.cpp \
    CoNLLUReader.cpp \
    CorpusImporter.cpp \
    DependenciesToLatex.cpp \
    UDTokenBase.cpp \
    UDToken.cpp \
    UDTokenMultiWord.cpp \
    UDEmptyNode.cpp \
    UDSentence.cpp \
    SentencesSplitter.cpp \
    udpipelib/udpipe.cpp \
    DictionaryBuilder.cpp \
    LatexTikzDependencyExporter.cpp

