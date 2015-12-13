# Praaline
# Core Library
# (c) George Christodoulides 2012-2015

! include( ../common.pri ) {
    error( Could not find the common.pri file! )
}

TEMPLATE = lib

CONFIG(debug, debug|release) {
    TARGET = pncored
} else {
    TARGET = pncore
}

CONFIG += qt thread warn_on stl rtti exceptions c++11

QT += xml sql
QT -= gui

DEFINES += PRAALINE_CORE_LIBRARY

INCLUDEPATH += . ..
DEPENDPATH += . ..

# Praaline core has a dependency on VAMP SDK (only for RealTime conversion)
win32-g++ {
    INCLUDEPATH += ../sv-dependency-builds/win32-mingw/include
    LIBS += -L../sv-dependency-builds/win32-mingw/lib
}
win32-msvc* {
    INCLUDEPATH += ../sv-dependency-builds/win32-msvc/include
    LIBS += -L../sv-dependency-builds/win32-msvc/lib
}
macx* {
    INCLUDEPATH += ../sv-dependency-builds/osx/include
    LIBS += -L../sv-dependency-builds/osx/lib
}

HEADERS += \
    base/BaseTypes.h \
    base/RealTime.h \
    base/realvaluelist.h \
    annotation/point.h \
    annotation/interval.h \
    annotation/annotationtier.h \
    annotation/intervaltier.h \
    annotation/pointtier.h \
    annotation/annotationtiergroup.h \
    corpus/corpusobject.h \
    corpus/corpuscommunication.h \
    corpus/corpusannotation.h \
    corpus/corpusrecording.h \
    corpus/corpus.h \
    corpus/corpusspeaker.h \
    corpus/corpusparticipation.h \
    corpus/corpusbookmark.h \
    structure/annotationstructure.h \
    structure/annotationstructureattribute.h \
    structure/annotationstructurelevel.h \
    structure/metadatastructureattribute.h \
    structure/metadatastructure.h \
    structure/metadatastructuresection.h \
    interfaces/praat/praattextfile.h \
    interfaces/praat/praattextgrid.h \
    interfaces/exmaralda/exmaraldabasictranscription.h \
    interfaces/exmaralda/exmaraldatranscriptionbridge.h \
    serialisers/xml/xmlserialiserannotationstructure.h \
    serialisers/xml/xmlserialisermetadatastructure.h \
    serialisers/xml/xmlserialiserbase.h \
    serialisers/xml/xmlserialisercorpus.h \
    serialisers/sql/sqlserialiserbase.h \
    serialisers/sql/sqlserialisercorpus.h \
    serialisers/sql/sqlserialisermetadatastructure.h \
    serialisers/sql/sqlserialiserannotationstructure.h \
    serialisers/sql/sqlschemaproxymetadata.h \
    serialisers/sql/sqlschemaproxyannotation.h \
    serialisers/sql/sqlserialiserannotation.h \
    query/querydefinition.h \
    serialisers/datastoreinfo.h \
    serialisers/abstractmetadatadatastore.h \
    serialisers/abstractannotationdatastore.h \
    serialisers/sql/sqlannotationdatastore.h \
    serialisers/datastorefactory.h \
    serialisers/sql/sqlmetadatadatastore.h \
    serialisers/sql/sqlschemaproxybase.h \
    serialisers/xml/xmlannotationdatastore.h \
    serialisers/xml/xmlmetadatadatastore.h \
    serialisers/xml/xmlserialiserannotation.h \
    serialisers/corpusdefinition.h \
    pncore_global.h \
    annotation/annotationdatatable.h \
    annotation/speakertimeline.h \
    serialisers/xml/xmlserialisercorpusbookmark.h \
    structure/structurebase.h \
    query/queryoccurrence.h \
    serialisers/sql/sqlqueryengineannotation.h \
    query/queryfiltergroup.h \
    query/queryfiltersequence.h \
    interfaces/subtitles/subtitlesfile.h \
    interfaces/interfacetextfile.h \
    annotation/annotationelement.h \
    annotation/relationtier.h \
    annotation/treetier.h \
    interfaces/transcriber/transcriberannotationgraph.h \
    interfaces/praat/PraatPointTierFile.h \
    interfaces/anvil/AnvilMetadataTranscript.h

SOURCES += \
    base/RealTime.cpp \
    base/realvaluelist.cpp \
    annotation/point.cpp \
    annotation/interval.cpp \
    annotation/annotationtier.cpp \
    annotation/intervaltier.cpp \
    annotation/pointtier.cpp \
    annotation/annotationtiergroup.cpp \
    corpus/corpuscommunication.cpp \
    corpus/corpusannotation.cpp \
    corpus/corpusrecording.cpp \
    corpus/corpus.cpp \
    corpus/corpusspeaker.cpp \
    corpus/corpusobject.cpp \
    corpus/corpusparticipation.cpp \
    corpus/corpusbookmark.cpp \
    structure/annotationstructure.cpp \
    structure/annotationstructureattribute.cpp \
    structure/annotationstructurelevel.cpp \
    structure/metadatastructureattribute.cpp \
    structure/metadatastructure.cpp \
    structure/metadatastructuresection.cpp \
    interfaces/praat/praattextfile.cpp \
    interfaces/praat/praattextgrid.cpp \
    interfaces/exmaralda/exmaraldabasictranscription.cpp \
    interfaces/exmaralda/exmaraldatranscriptionbridge.cpp \
    serialisers/xml/xmlserialiserannotationstructure.cpp \
    serialisers/xml/xmlserialisermetadatastructure.cpp \
    serialisers/xml/xmlserialiserbase.cpp \
    serialisers/xml/xmlserialisercorpus.cpp \
    serialisers/sql/sqlserialiserbase.cpp \
    serialisers/sql/sqlserialisercorpus.cpp \
    serialisers/sql/sqlserialisermetadatastructure.cpp \
    serialisers/sql/sqlserialiserannotationstructure.cpp \
    serialisers/sql/sqlschemaproxymetadata.cpp \
    serialisers/sql/sqlschemaproxyannotation.cpp \
    serialisers/sql/sqlserialiserannotation.cpp \
    query/querydefinition.cpp \
    serialisers/sql/sqlannotationdatastore.cpp \
    serialisers/datastorefactory.cpp \
    serialisers/sql/sqlmetadatadatastore.cpp \
    serialisers/sql/sqlschemaproxybase.cpp \
    serialisers/xml/xmlannotationdatastore.cpp \
    serialisers/xml/xmlmetadatadatastore.cpp \
    serialisers/xml/xmlserialiserannotation.cpp \
    serialisers/corpusdefinition.cpp \
    annotation/annotationdatatable.cpp \
    annotation/speakertimeline.cpp \
    serialisers/xml/xmlserialisercorpusbookmark.cpp \
    structure/structurebase.cpp \
    query/queryoccurrence.cpp \
    serialisers/sql/sqlqueryengineannotation.cpp \
    query/queryfiltergroup.cpp \
    query/queryfiltersequence.cpp \
    interfaces/subtitles/subtitlesfile.cpp \
    interfaces/interfacetextfile.cpp \
    annotation/annotationelement.cpp \
    annotation/relationtier.cpp \
    annotation/treetier.cpp \
    interfaces/transcriber/transcriberannotationgraph.cpp \
    interfaces/praat/PraatPointTierFile.cpp \
    interfaces/anvil/AnvilMetadataTranscript.cpp
