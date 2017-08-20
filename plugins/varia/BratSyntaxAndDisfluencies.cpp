#include <QString>
#include <QList>
#include <QMap>
#include <QPair>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "BratAnnotationExporter.h"
#include "BratSyntaxAndDisfluencies.h"

struct BratSyntaxAndDisfluenciesData
{
    QString sentenceTier;
};

BratSyntaxAndDisfluencies::BratSyntaxAndDisfluencies() :
    d(new BratSyntaxAndDisfluenciesData())
{
}

BratSyntaxAndDisfluencies::~BratSyntaxAndDisfluencies()
{
    delete d;
}

QString BratSyntaxAndDisfluencies::sentenceTier() const
{
    return d->sentenceTier;
}

void BratSyntaxAndDisfluencies::setSentenceTier(const QString &tiername)
{
    d->sentenceTier = tiername;
}

QString htmlHead(const QString &pageTitle)
{
    QString html;
    html.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
    html.append("<html lang=\"fr-FR\" xml:lang=\"fr-FR\" xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    html.append("<head>\n");
    html.append("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n");
    html.append(QString("<title>%1</title>\n").arg(pageTitle));
    html.append("<link rel=\"stylesheet\" type=\"text/css\" href=\"style-vis.css\"/>\n");
    html.append("<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n");
    html.append("<script type=\"text/javascript\" src=\"client/lib/head.load.min.js\"></script>\n");
    html.append("</head>\n");
    return html;
}

QString htmlStyle()
{
    QString html;
    html.append("<style type=\"text/css\">\n");
    html.append("text { font-size: 15px; }\n");
    html.append(".span text { font-size: 10px; }\n");
    html.append(".arcs text { font-size: 9px; }\n");
    html.append("</style>\n");
    return html;
}

QString scriptInit()
{
    QString s;
    s.append("var bratLocation = '';\n");
    s.append("head.js(");
    // External libraries
    s.append("    bratLocation + 'client/lib/jquery.min.js',\n");
    s.append("    bratLocation + 'client/lib/jquery.svg.min.js',\n");
    s.append("    bratLocation + 'client/lib/jquery.svgdom.min.js',\n");
    // brat helper modules
    s.append("    bratLocation + 'client/src/configuration.js',\n");
    s.append("    bratLocation + 'client/src/util.js',\n");
    s.append("    bratLocation + 'client/src/annotation_log.js',\n");
    s.append("    bratLocation + 'client/lib/webfont.js',\n");
    // brat modules
    s.append("    bratLocation + 'client/src/dispatcher.js',\n");
    s.append("    bratLocation + 'client/src/url_monitor.js',\n");
    s.append("    bratLocation + 'client/src/visualizer.js'\n");
    s.append(");\n");
    s.append("var webFontURLs = [\n");
    s.append("    bratLocation + 'static/fonts/Astloch-Bold.ttf',\n");
    s.append("    bratLocation + 'static/fonts/PT_Sans-Caption-Web-Regular.ttf',\n");
    s.append("    bratLocation + 'static/fonts/Liberation_Sans-Regular.ttf'\n");
    s.append("];\n");
    return s;
}

QString scriptHeadReadyFunction(const BratAnnotationExporter::BratCollData &collData,
                                QList<BratAnnotationExporter::BratDocData> docDataList)
{
    QString s;
    BratAnnotationExporter brat;
    s.append("head.ready(function() {\n");
    s.append("var collData = {};\n");
    s.append(brat.scriptCollData(collData)).append("\n");
    s.append("var docData = {};\n");
    foreach (BratAnnotationExporter::BratDocData docData, docDataList) {
        s.append(brat.scriptDocData(docData)).append("\n");
        s.append(QString("Util.embed('%1', $.extend({}, collData), $.extend({}, docData), webFontURLs)\n\n").arg(docData.divID));
    }
    s.append("});\n");
    return s;
}

void addBratEntitiesForDisfluencies(BratAnnotationExporter::BratCollData &collData)
{
    collData.entityTypes << BratAnnotationExporter::BratEntityType("FIL", "Filled Pause",       "FIL", "#7fa2ff", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("FST", "False Start",        "FST", "#7fa2ff", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("LEN", "Lengthening",        "LEN", "#7fa2ff", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("WDP", "Pause Inside Word",  "WDP", "#7fa2ff", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("MSP", "Mispronunciation",   "MSP", "#7fa2ff", "darken");

    collData.entityTypes << BratAnnotationExporter::BratEntityType("REP", "Repetition",         "REP", "#7fa65f", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("RED", "Repetition (edit)",  "RED", "#7fa65f", "darken");

    collData.entityTypes << BratAnnotationExporter::BratEntityType("DEL", "Deletion",           "DEL", "#48a2af", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("SUB", "Substitution",       "SUB", "#48a2af", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("INS", "Insertion",          "INS", "#48a2af", "darken");
    collData.entityTypes << BratAnnotationExporter::BratEntityType("COM", "Complex disfluency", "COM", "#48a2af", "darken");
}

QString BratSyntaxAndDisfluencies::getHTML(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    QString html;
    BratAnnotationExporter::BratCollData collData;
    addBratEntitiesForDisfluencies(collData);

    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;

            // Structure to hold brat annotations for this file
            QList<BratAnnotationExporter::BratDocData> docDataList;

            // Begin HTML
            html.clear();
            html.append(htmlHead(annotationID)).append("\n");
            html.append("<body>\n");

            // Major grouping tier
            IntervalTier *tier_sentence = tiers->getIntervalTierByName(d->sentenceTier);
            if (!tier_sentence) continue;
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            SequenceTier *tier_disfluencies = tiers->getSequenceTierByName("disfluencies");
            if (!tier_disfluencies) continue;

            // tok_mwu - tok_min correspondance;
            QMap<int, QList<int> > tok_mwu_to_min;
            int i_min(0);
            for (int i_mwu = 0; i_mwu < tier_tok_mwu->count(); ++i_mwu) {
                QList<int> indices_min;
                while (i_min < tier_tok_min->count() && tier_tok_min->at(i_min)->tMax() <= tier_tok_mwu->at(i_mwu)->tMax()) {
                    indices_min << i_min;
                    i_min++;
                }
                tok_mwu_to_min.insert(i_mwu, indices_min);
            }

            // In-text sequence processing
            QMap<int, QString> tok_mwu_textOpening;
            QMap<int, QString> tok_mwu_textClosing;
            SequenceTier *tier_bdu = tiers->getSequenceTierByName("bdu");
            SequenceTier *tier_syntax = tiers->getSequenceTierByName("syntactic_units");
            SequenceTier *tier_prosody = tiers->getSequenceTierByName("prosodic_units");
            foreach (Sequence *seq, tier_bdu->sequences()) {
                tok_mwu_textOpening.insert(seq->indexFrom(), " [");
                tok_mwu_textClosing.insert(seq->indexTo(), QString("]%1 ").arg(seq->text().trimmed()));
            }
            foreach (Sequence *seq, tier_syntax->sequences()) {
                QString textOpen = " (";
                QString textClose = QString(")%1 ").arg(seq->text().trimmed());
                if (tok_mwu_textOpening.contains(seq->indexFrom()))
                    tok_mwu_textOpening[seq->indexFrom()] = QString(tok_mwu_textOpening[seq->indexFrom()]).append(textOpen);
                else
                    tok_mwu_textOpening.insert(seq->indexFrom(), textOpen);

                if (tok_mwu_textClosing.contains(seq->indexTo()))
                    tok_mwu_textClosing[seq->indexTo()] = QString(tok_mwu_textClosing[seq->indexTo()]).prepend(textClose);
                else
                    tok_mwu_textClosing.insert(seq->indexTo(), textClose);
            }
            foreach (Sequence *seq, tier_prosody->sequences()) {
                if (tok_mwu_textClosing.contains(seq->indexTo()))
                    tok_mwu_textClosing[seq->indexTo()] = QString(tok_mwu_textClosing[seq->indexTo()]).prepend("| ");
                else
                    tok_mwu_textClosing.insert(seq->indexTo(), "| ");
            }

            foreach (Interval *sentence, tier_sentence->intervals()) {
                // Structures to keep track of character positions
                QMap<int, int> tok_min_charStart;
                QMap<int, int> tok_min_charEnd;

                BratAnnotationExporter::BratDocData docData;
                QString divID = sentence->text();
                html.append(QString("<div><b>%1</b><br />\n").arg(divID));
                html.append(QString("<div id=\"%1\"></div>\n").arg(divID));
                html.append("<br /></div>\n");
                docData.divID = divID;
                QString text;
                int cursor(0);

                QPair<int, int> indices_mwu = tier_tok_mwu->getIntervalIndexesContainedIn(sentence);
                for (int i_mwu = indices_mwu.first; i_mwu <= indices_mwu.second; ++i_mwu) {
                    // Opening (grouping text)
                    if (tok_mwu_textOpening.contains(i_mwu)) {
                        if (text.endsWith(" ")) { text.chop(1); cursor--; }
                        text.append(tok_mwu_textOpening.value(i_mwu));
                        cursor = cursor + tok_mwu_textOpening.value(i_mwu).length();
                    }
                    // Tokens
                    foreach (int i_min, tok_mwu_to_min.value(i_mwu)) {
                        QString tokmin = tier_tok_min->at(i_min)->text().trimmed();
                        tok_min_charStart.insert(i_min, cursor);
                        tok_min_charEnd.insert(i_min, cursor + tokmin.length());
                        text.append(tokmin).append(" ");
                        cursor = cursor + tokmin.length() + 1;
                    }
                    // Closing (grouping text)
                    if (tok_mwu_textClosing.contains(i_mwu)) {
                        text.append(tok_mwu_textClosing.value(i_mwu));
                        cursor = cursor + tok_mwu_textClosing.value(i_mwu).length();
                    }
                }
                docData.text = text;

                // Annotate disfluencies
                int i(0);
                foreach (Sequence *seq, tier_disfluencies->sequences()) {
                    if ((!tok_min_charStart.contains(seq->indexFrom())) || (!tok_min_charEnd.contains(seq->indexTo()))) continue;
                    docData.entities << BratAnnotationExporter::BratEntity(
                                            QString("D%1").arg(i), seq->text(),
                                            tok_min_charStart.value(seq->indexFrom()), tok_min_charEnd.value(seq->indexTo()));
                    i++;
                }

                docDataList << docData;
                ret.append(QString("Exporting %1\t%2\n").arg(annotationID).arg(divID));
            }
            html.append("</body>\n\n");
            html.append(htmlStyle()).append("\n");
            html.append("<script type=\"text/javascript\">\n");
            html.append(scriptInit()).append("\n");
            html.append(scriptHeadReadyFunction(collData, docDataList)).append("\n");
            html.append("</script>\n");
            html.append("</html>\n");

            QFile file(QString("/home/george/test_brat/%1.html").arg(annotationID));
            if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) continue;
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << html;
            file.close();
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
