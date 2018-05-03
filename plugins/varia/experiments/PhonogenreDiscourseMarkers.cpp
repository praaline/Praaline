#include <QDebug>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QMapIterator>
#include <QFile>
#include <QTextStream>
#include <math.h>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/statistics/Measures.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "PhonogenreDiscourseMarkers.h"

PhonogenreDiscourseMarkers::PhonogenreDiscourseMarkers()
{

}

// static
QString PhonogenreDiscourseMarkers::readBackAnnotations(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    if (!com->corpus()) return ret;
    QString path = QDir::homePath() + "/Dropbox/CORPORA/Phonogenre/DiscourseMarkerAnnotation/";
    QStringList filenames; filenames << "phonogenre_et.txt";
    foreach (QString filename, filenames) {
        QString line;
        QFile file(path + filename);
        if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            ret.append("Error reading file ").append(filename); continue;
        }
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        do {
            line = stream.readLine();
            if (line.startsWith("#")) continue;
            QStringList fields = line.split("\t");
            QString annotationID = fields.at(2);
            QString speakerID = fields.at(3);
            IntervalTier *tier_tok_min = qobject_cast<IntervalTier *>(com->corpus()->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
            if (!tier_tok_min) { ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n"); continue; }
            int intervalNo = fields.at(6).toInt();
            if  (intervalNo <= 0 || intervalNo > tier_tok_min->count()) {
                ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n");
                continue;
            }
            Interval *tok_min = tier_tok_min->interval(intervalNo - 1);
            QString text = fields.at(8);
            if (tok_min->text() != text) {
                ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n");
                continue;
            }
            tok_min->setAttribute("DM", fields.at(10));
            tok_min->setAttribute("Non_DM", fields.at(11));
            com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            delete tier_tok_min;
        } while (!stream.atEnd());
        file.close();
        ret.append("Read filename ").append(filename).append("\n");
    }
    return ret;
}

QString PhonogenreDiscourseMarkers::statistics(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    if (!com->corpus()) return ret;

    // Output
    QString path = QDir::homePath() + "/Dropbox/CORPORA/Phonogenre/DiscourseMarkerAnnotation/";
    QFile file(path + "prosodic_dms.txt");
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return "Error writing output file";
    QTextStream out(&file);
    out.setCodec("UTF-8");

    out << "Genre\tAnnotationID\tSpeakerID\tDiscourseMarker\tDMCategory\tToken\tTime\tDuration\tPitch\t";
    out << "PauseBefore\tPauseAfter\n";

}
