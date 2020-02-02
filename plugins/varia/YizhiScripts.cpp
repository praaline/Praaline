#include <QString>
#include <QHash>
#include <QPair>
#include <QFile>
#include <QTextStream>
#include <QSharedPointer>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "PraalineASR/HTK/HTKForcedAligner.h"
using namespace Praaline::ASR;

#include "YizhiScripts.h"

struct PhonetisedSyllable {
    QString syll;
    QList<QString> phonemes; // of the syllable
};

struct YizhiScriptsData {
    QHash<QString, QList<PhonetisedSyllable> > phonetisation;
};

YizhiScripts::YizhiScripts() :
    d(new YizhiScriptsData())
{
}

YizhiScripts::~YizhiScripts() {
    delete d;
}

QString YizhiScripts::readPhonetisationFile()
{
    QString ret;
    QString filename = "/mnt/hgfs/DATA/yizhi/phonetisation.txt";
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error opening phonetisation file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        QString line = stream.readLine();
        line = line.trimmed();
        if (line.startsWith("#")) continue;
        if (line.isEmpty()) continue;
        QStringList fields = line.split("\t");
        if (fields.count() != 3) {
            ret.append("Incorrect number of fields: ").append(line).append("\n");
            continue;
        }
        QString pinyin = fields.at(0);
        QString syllables = fields.at(1);
        QString phonemes = fields.at(2);
        if (syllables.split(" ").count() != phonemes.split("|").count()) {
            ret.append("Mismatch: ").append(line).append("\n");
            continue;
        }
        QList<PhonetisedSyllable> syll_list;
        for (int i = 0; i < syllables.split(" ").count(); ++i) {
            PhonetisedSyllable syll;
            syll.syll = syllables.split(" ").at(i);
            syll.phonemes = phonemes.split("|").at(i).split(" ");
            syll_list << syll;
        }
        d->phonetisation.insert(pinyin, syll_list);
    } while (!stream.atEnd());
    file.close();
    ret.append("Reading phonetisation file... OK");
    return ret;
}

QString YizhiScripts::createAnnotation(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    QString pinyin = com->property("pinyin").toString();
    QList<PhonetisedSyllable> syllables = d->phonetisation.value(pinyin);
    // Find recording
    CorpusRecording *rec = com->recordings().first();
    if (!rec) return "Error: No recording";
    // Find annotation, otherwise create one
    QString annotationID = com->ID();
    if (com->annotationsCount() == 0) {
        com->addAnnotation(new CorpusAnnotation(annotationID));
        com->save();
    }


    IntervalTier *tier_tokens = new IntervalTier("tokens");
    IntervalTier *tier_syll = new IntervalTier("syll");
    IntervalTier *tier_phone = new IntervalTier("phone");


    // QSharedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());


    ret.append(pinyin).append("\t");
    foreach (PhonetisedSyllable syll, syllables) {
        ret.append(syll.syll).append("{").append(syll.phonemes.join(" ")).append("} ");
    }
    return ret;
}

