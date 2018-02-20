#include <QDebug>
#include <QString>
#include <QPointer>
#include <QFile>
#include <QTextStream>
#include <QProcess>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "MBROLAResynthesiser.h"

struct MBROLAResynthesiserData {
    QString tiernamePhone;
    QString tiernameSyll;
    QString attributePhoneLabels;
};

MBROLAResynthesiser::MBROLAResynthesiser(QObject *parent) : QObject(parent)
{

}

QString MBROLAResynthesiser::createPhoFile(const QString &filenamePho, QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;

    QStringList vowels;
    vowels << "i" << "e" << "E" << "a" << "A" << "o" << "O" << "y" << "2" << "9" << "e~" << "a~" << "o~" << "9~";

    if (!com) return "Error";
    ret = com->ID();
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "Error";
    QString annotationID = rec->ID();
    QString speakerID = com->property("SubjectID").toString();
    // Get tiers
    IntervalTier *tier_phone = qobject_cast<IntervalTier *>
            (com->repository()->annotations()->getTier(annotationID, speakerID, "phone"));
    if (!tier_phone) return ret + " No tier phone";
    IntervalTier *tier_syll = qobject_cast<IntervalTier *>
            (com->repository()->annotations()->getTier(annotationID, speakerID, "syll"));
    if (!tier_syll) return ret + " No tier syll";
    // Crete Pho file
    QFile fileOut(filenamePho);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return ret + " Error writing file";
    QTextStream out(&fileOut);
    out.setCodec("ISO 8859-1");
    // Write out phones
    QString attributePhoneLabels;
    foreach(Interval *phone, tier_phone->intervals()) {
        // Phone label
        QString p = (attributePhoneLabels.isEmpty()) ? phone->text() : phone->attribute(attributePhoneLabels).toString();
        if (p.trimmed().isEmpty()) p = "_";
        out << p << "\t";
        // Phone duration
        out << qRound(phone->duration().toDouble() * 1000.0);
        // Pitch targets (percentage of duration - pitch target in Hz)
        if (vowels.contains(p)) {
            Interval *syll = tier_syll->intervalAtTime(phone->tCenter());
            if (syll && ((!syll->isPauseSilent() && syll->attribute("f0_start").toInt() > 0 &&
                          syll->attribute("f0_end").toInt() > 0))) {
                out << "\t";
                out << "0\t" << syll->attribute("f0_start").toInt() << "\t";
                out << "100\t" << syll->attribute("f0_end").toInt();
            }
        }
        out << "\n";
    }
    fileOut.close();
    return ret;
}

QString MBROLAResynthesiser::resynthesise(const QString &directory, QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    ret = com->ID();
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "Error";
    QString speakerID = com->property("SubjectID").toString();
    QPointer<CorpusSpeaker> spk = com->corpus()->speaker(speakerID);
    QString sex = spk->property("Sex").toString();
    QString voice = (sex == "F") ? "/home/george/Downloads/mbrola/fr2/fr2" : "/home/george/Downloads/mbrola/fr1/fr1";

    // Filenames
    QString filenamePho = directory + "/" + rec->ID() + ".pho";
    QString filenameWav = directory + "/" + rec->ID() + "_resynth.wav";
    ret = ret + "\t" + createPhoFile(filenamePho, com);

    // Execute MBROLA
    QProcess *processMBROLA = new QProcess();
    processMBROLA->setReadChannel(QProcess::StandardOutput);
    QString commandMBROLA;
#ifdef Q_OS_WIN
    QString appPath = QCoreApplication::applicationDirPath();
    commandMBROLA = appPath + "/tools/mbrola.exe";
#else
#ifdef Q_OS_MAC
    commandMBROLA = "/Users/george/mbrola";
#else Q_OS_WIN
    commandMBROLA = "/home/george/Downloads/mbrola/mbrola";
#endif
#endif
    QStringList args;
    args << voice << filenamePho << filenameWav;
    processMBROLA->start(commandMBROLA, args);
    if (!processMBROLA->waitForStarted(-1)) {
        ret.append("Error: " + processMBROLA->errorString());
        return ret;
    }
    if (!processMBROLA->waitForFinished(-1)) {
        qDebug() << processMBROLA->errorString();
        ret.append("Error: " + processMBROLA->errorString());
        return ret;
    }
    QByteArray result = processMBROLA->readAllStandardOutput();
    qDebug() << result;
    if (processMBROLA) {
        processMBROLA->close();
        delete processMBROLA;
    }
    ret.append("Resynthesised: " + filenameWav);
    // ret = commandMBROLA + " " + args.join(" ");
    return ret;
}
