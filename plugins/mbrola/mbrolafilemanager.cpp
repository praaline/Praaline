#include <QMap>
#include <QFile>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "mbrolafilemanager.h"

MBROLAFileManager::MBROLAFileManager()
{
}

bool MBROLAFileManager::createPhoFile(const QString &filenamePho, IntervalTier *tier_phones, const QString &attributeID)
{
    return createPhoFile(filenamePho, tier_phones->intervals(), attributeID);
}

bool MBROLAFileManager::createPhoFile(const QString &filenamePho, QList<Interval *> phones, const QString &attributeID)
{
    QFile fileOut(filenamePho);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream out(&fileOut);
    out.setCodec("ISO 8859-1");
    foreach(Interval *phone, phones) {
        QString p = (attributeID.isEmpty()) ? phone->text() : phone->attribute(attributeID).toString();
        if (p.trimmed().isEmpty()) p = "_";
        out << p << "\t";
        out << qRound(phone->duration().toDouble() * 1000.0) << "\t";
        for (int i = 1; i <= 20; ++i) {
            out << i * 5 << "\t";
            out << phone->attribute(QString("pitch_target_%1").arg(i * 5)).toString() << "\t";
        }
        out << "\n";
    }
    fileOut.close();
    return true;
}




bool MBROLAFileManager::updatePhoneTierFromPhoFile(const QString &filenamePho, IntervalTier *tier,
                                                   const QString &attributeForPhoneme, bool updatePhone, bool updatePitch)
{
    if (!tier) return false;
    // Read PHO file
    QList<QList<QVariant> > data;
    QFile file(filenamePho);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);
    do {
        QString line = stream.readLine();
        QList<QVariant> record;
        foreach (QString value, line.split("\t")) {
            record.append(QVariant(value));
        }
        data.append(record);
    } while (!stream.atEnd());
    file.close();
    // Update attributes from annotation table
    if (data.count() < tier->count()) return false;

    for (int i = 0; i < tier->count(); i++) {
        Interval *intv = tier->interval(i);
        if (!intv) continue;
        QList<QVariant> record = data.at(i);
        // Update phoneme
        if (updatePhone) {
            QString phoneme = record.at(0).toString();
            if (attributeForPhoneme.isEmpty())
                intv->setText(phoneme);
            else
                intv->setAttribute(attributeForPhoneme, phoneme);
        }
        // Update pitch targets
        if (updatePitch) {
            for (int j = 1; j <= 20; ++j) {
                int target = 5 * j;
                // column 3, 5, 7...
                QVariant pitch = record.at(1 + 2 * j);
                intv->setAttribute(QString("pitch_target_%1").arg(target), pitch);
            }
        }
    }
    return true;
}


void MBROLAFileManager::extractPhoParameters(QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot,
                                             const QString &attributeForPhoneme)
{
    if (!rec) return;
    if (!annot) return;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = annot->repository()->annotations()->getTiersAllSpeakers(annot->ID());
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);

        IntervalTier *tier_phones = tiers->getIntervalTierByName("phone");
        if (!tier_phones) continue;

        // Find speaker's sex
        QPointer<CorpusSpeaker> spk = annot->corpus()->speaker(speakerID);
        QString sex = "Male";
        if (spk) {
            QString spkSex = spk->property("sex").toString();
            if (spkSex == "F" || spkSex == "Female") sex = "Female";
        }

        // Create a temporary directory
        QTemporaryDir dirTemp;
        if (!dirTemp.isValid()) return;
        QString tempDirectory = dirTemp.path(); // returns the unique directory path
        if (!tempDirectory.endsWith("/")) tempDirectory.append("/");
        // Copy the recording file into temp+.wav
        QString filenameTempRec = QString("%1_%2.wav").arg(rec->ID()).arg(speakerID);
        QFile::copy(rec->filePath(), tempDirectory + filenameTempRec);
        // Prepare the annotation (phones only)
        QString filenameTempAnnot = QString("%1_%2.TextGrid").arg(rec->ID()).arg(speakerID);
        QPointer<AnnotationTierGroup> tiersOnlyPhones = new AnnotationTierGroup();
        tiersOnlyPhones->addTier(tier_phones);
        PraatTextGrid::save(tempDirectory + filenameTempAnnot, tiersOnlyPhones);
        // Call script
        QString appPath = QCoreApplication::applicationDirPath();
        QString script = appPath + "/plugins/mbrola/praaline_mbrolise.praat";
        QStringList scriptArguments;
        scriptArguments << tempDirectory + filenameTempRec << tempDirectory + filenameTempAnnot <<
                           sex << "1" << rec->basePath();
        executePraatScript(script, scriptArguments);

        QString filenamePho = rec->basePath() + "/" + QString(filenameTempAnnot).replace(".TextGrid", "m.pho");
        updatePhoneTierFromPhoFile(filenamePho, tier_phones, attributeForPhoneme);
        if (!annot->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_phones)) {
            qDebug() << "Error in saving " << annot->ID() << " speaker " << speakerID << " phone count " << tier_phones->count();
        }
    }
    qDeleteAll(tiersAll);
}

