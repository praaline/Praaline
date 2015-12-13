#ifndef XMLSERIALISERCORPUS_H
#define XMLSERIALISERCORPUS_H

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "xmlserialiserbase.h"
#include "corpus/corpus.h"
#include "corpus/corpuscommunication.h"
#include "corpus/corpusspeaker.h"
#include "corpus/corpusrecording.h"
#include "corpus/corpusannotation.h"

class XMLSerialiserCorpus : XMLSerialiserBase
{
public:
    static bool saveCorpus(Corpus *corpus, QXmlStreamWriter &xml);
    static bool loadCorpus(Corpus *corpus, QXmlStreamReader &xml);

private:
    XMLSerialiserCorpus();
    static void writeCommunication(CorpusCommunication *com, MetadataStructure *mstructure, QXmlStreamWriter &xml);
    static void writeSpeaker(CorpusSpeaker *spk, MetadataStructure *mstructure, QXmlStreamWriter &xml);
    static void writeRecording(CorpusRecording *rec, MetadataStructure *mstructure, QXmlStreamWriter &xml);
    static void writeAnnotation(CorpusAnnotation *annot, MetadataStructure *mstructure, QXmlStreamWriter &xml);
    static void writeParticipation(CorpusParticipation *participation, MetadataStructure *mstructure, QXmlStreamWriter &xml);

    static CorpusCommunication *readCommunication(MetadataStructure *mstructure, QXmlStreamReader &xml);
    static CorpusSpeaker *readSpeaker(MetadataStructure *mstructure, QXmlStreamReader &xml);
    static CorpusRecording *readRecording(MetadataStructure *mstructure, QXmlStreamReader &xml);
    static CorpusAnnotation *readAnnotation(MetadataStructure *mstructure, QXmlStreamReader &xml);
    static void readParticipation(Corpus *corpus, MetadataStructure *mstructure, QXmlStreamReader &xml);

    static void readAttributes(CorpusObject *obj, MetadataStructure *mstructure, CorpusObject::Type what, QXmlStreamReader &xml);

    static QString xmlElementName_Corpus;
    static QString xmlElementName_Communication;
    static QString xmlElementName_Speaker;
    static QString xmlElementName_Recording;
    static QString xmlElementName_Annotation;
    static QString xmlElementName_Participation;
};

#endif // XMLSERIALISERCORPUS_H
