#include <QDebug>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>

#include "SpeechToken.h"
#include "ExternalPhonetiser.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

namespace Praaline {
namespace ASR {

class PUtterance {
public:
    PUtterance(QList<int> &ptokens, QString orthographic) : ptokens(ptokens), orthographic(orthographic) {}
    QList<int> ptokens;
    QString orthographic;
    QString phonetisation;
};


struct ExternalPhonetiserData {

};

ExternalPhonetiser::ExternalPhonetiser(QObject *parent) :
    Phonetiser(parent), d(new ExternalPhonetiserData)
{

}

ExternalPhonetiser::~ExternalPhonetiser()
{
    delete d;
}

bool callPhonetiserCitationForms(QHash<QString, QString> &citationForms)
{
    if (citationForms.isEmpty()) return false;
    // Get a temporary file and write out the forms
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;

    QList<QString> forms;

    if (!fileIn.open()) return false;
    filenameIn = fileIn.fileName();
    QTextStream streamIn(&fileIn);
    streamIn.setCodec("ISO 8859-1");
    foreach (QString form, citationForms.keys()) {
        if (form.isEmpty()) continue;
        forms << form;
        streamIn << form << " .\n";
    }
    fileIn.close();

    // Touch the output file, and get its filename
    if (!fileOut.open()) return false;
    filenameOut = fileOut.fileName();
    fileOut.close();

    // Pass to external phonetiser
    // DIRECTORY:
    QString phonPath = QDir::homePath() + "/Praaline/plugins/aligner/phonetiser/";
    QProcess phonetiser;
    phonetiser.setWorkingDirectory(phonPath);
    phonetiser.start(phonPath + "lang/fra/phon250.exe" , QStringList() <<
                     phonPath + "lang/fra/phon250.fra.ini" << filenameIn << filenameOut);
    if (!phonetiser.waitForStarted(-1)) return false;
    if (!phonetiser.waitForFinished(-1)) return false;
    // Read responses into hash
    if (!fileOut.open()) return false;
    QTextStream streamOut(&fileOut);
    streamOut.setCodec("ISO 8859-1");
    int i = 0;
    while (!streamOut.atEnd() && i < forms.count()) {
        QString phonetisation = streamOut.readLine();
        if (phonetisation.isEmpty()) continue;
        phonetisation = phonetisation.replace("| _", "").trimmed();
        citationForms[forms[i]] = phonetisation;
        i++;
    }
    fileOut.close();
    return true;
}

bool callPhonetiser(QList<PUtterance> &utterances)
{
    if (utterances.isEmpty()) return false;

    // Get a temporary file and write out the utterances
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;

    if (!fileIn.open()) return false;
    filenameIn = fileIn.fileName();
    QTextStream streamIn(&fileIn);
    streamIn.setCodec("ISO 8859-1");
    foreach (PUtterance utterance, utterances) {
        if (utterance.orthographic.isEmpty()) continue;
        streamIn << utterance.orthographic << " .\n";
    }
    fileIn.close();

    // Touch the output file, and get its filename
    if (!fileOut.open()) return false;
    filenameOut = fileOut.fileName();
    fileOut.close();

    // Pass to external phonetiser
    // DIRECTORY:
    QString phonPath = QDir::homePath() + "/Praaline/plugins/aligner/phonetiser/";
    QProcess phonetiser;
    phonetiser.setWorkingDirectory(phonPath);
    phonetiser.start(phonPath + "lang/fra/phon250.exe" , QStringList() <<
                     phonPath + "lang/fra/phon250.fra.ini" << filenameIn << filenameOut);
    if (!phonetiser.waitForStarted(-1)) return false;
    if (!phonetiser.waitForFinished(-1)) return false;
    // Read responses into token list
    if (!fileOut.open()) return false;
    QTextStream streamOut(&fileOut);
    streamOut.setCodec("ISO 8859-1");
    int i = 0;
    while (!streamOut.atEnd() && i < utterances.count()) {
        QString phonetisation = streamOut.readLine();
        if (phonetisation.isEmpty()) continue;
        phonetisation = phonetisation.replace("| _", "");
        utterances[i].phonetisation = phonetisation.trimmed();
        i++;
    }
    fileOut.close();
    return true;
}

QList<SpeechToken> mergePhonetiserTokens(const QList<SpeechToken> &ptokens)
{
    QList<SpeechToken> merged;
    int i = 0, j = 0;
    while (i < ptokens.count() && j < ptokens.count()) {
        j = i + 1;
        if ( (ptokens[i].orthographic.endsWith("'")) &&
             (ptokens[i].orthographic.length() == 2 || ptokens[i].orthographic == "qu'") &&
             (j < ptokens.count()))
        {
            SpeechToken merge = SpeechToken(ptokens[i].start, ptokens[j].end,
                                            ptokens[i].orthographic + ptokens[j].orthographic);
            merged << merge;
            // qDebug() << i << j << merge.start << merge.end << merge.orthographic;
            i = j + 1;
        }
        else {
            merged << ptokens[i];
            // qDebug() << i << j << ptokens[i].start << ptokens[i].end << ptokens[i].orthographic;
            i++;
        }
    }
    return merged;
}

// static
QList<SpeechToken> ExternalPhonetiser::phonetiseList(QList<Interval *> tokens, const QString &attributeID)
{
    QList<SpeechToken> ptokens;
    int i = 0;
    foreach (Interval *token, tokens) {
        QString orthographic = ((attributeID.isEmpty()) ? token->text() : token->attribute(attributeID).toString()).trimmed();
        // characters to ignore
        orthographic = orthographic.replace("{", "").replace("}", "").replace("-", "").replace("(", "")
                .replace(")", "").replace("/", "").trimmed();
        orthographic = orthographic.simplified();
        if (orthographic.isEmpty()) orthographic = "_";
        foreach (QString ortho, orthographic.split(" ", QString::SkipEmptyParts)) {
            SpeechToken ptoken(i, i, ortho);
            ptokens << ptoken;
        }
        i++;
    }
    ptokens = mergePhonetiserTokens(ptokens);

    QList<PUtterance> putterances;
    QList<int> putterance_tokens;
    QString putterance_orthographic;
    for (int i = 0; i < ptokens.count(); ++i) {
        if (ptokens[i].orthographic == "_" && !putterance_orthographic.isEmpty()) {
            putterances << PUtterance(putterance_tokens, putterance_orthographic);
            putterance_orthographic.clear();
            putterance_tokens.clear();
            ptokens[i].phonetisations << "_";
        }
        else if (ptokens[i].orthographic != "_") {
            putterance_orthographic.append(ptokens[i].orthographic).append(" ");
            putterance_tokens << i;
        }
        else {
            ptokens[i].phonetisations << "_";
        }
    }
    // last
    putterances << PUtterance(putterance_tokens, putterance_orthographic);

    callPhonetiser(putterances);
    foreach (PUtterance putterance, putterances) {
        QStringList phonetisation_split = putterance.phonetisation.split("|", QString::SkipEmptyParts);
        if (phonetisation_split.count() != putterance.ptokens.count()) {
            // qDebug() << "mismatched tokens-phonotokens";
            // qDebug() << putterance.phonetisation << putterance.orthographic;
            QHash<QString, QString> citationForms;
            foreach (int ptoken_id, putterance.ptokens) {
                citationForms.insert(ptokens[ptoken_id].orthographic, "");
            }
            callPhonetiserCitationForms(citationForms);
            foreach (int ptoken_id, putterance.ptokens) {
                ptokens[ptoken_id].phonetisations << citationForms.value(ptokens[ptoken_id].orthographic, "@");
            }
            continue;
        }
        for (int i = 0; i < phonetisation_split.count(); ++i) {
            int ptoken_id = putterance.ptokens[i];
            ptokens[ptoken_id].phonetisations << phonetisation_split[i].trimmed();
        }
    }
    return ptokens;
}

// static
QList<SpeechToken> ExternalPhonetiser::phonetiseTier(IntervalTier *tier_tokens, const QString &attributeID)
{
    return phonetiseList(tier_tokens->intervals(), attributeID);
}

// static
void ExternalPhonetiser::addPhonetisationToTokens(IntervalTier *tier_tokens, const QString &attributeIDOrthographic,
                                                  const QString &attributeIDPhonetisation)
{
    QList<SpeechToken> atokens = phonetiseTier(tier_tokens, attributeIDOrthographic);
    foreach (SpeechToken atoken, atokens) {
        bool alreadyAddedPhonetisation = false;
        for (int i = atoken.start; i <= atoken.end; ++i) {
            if (!alreadyAddedPhonetisation) {
                tier_tokens->interval(i)->setAttribute(attributeIDPhonetisation, atoken.phonetisations.first());
                alreadyAddedPhonetisation = true;
            } else {
                tier_tokens->interval(i)->setAttribute(attributeIDPhonetisation, "^^^");
            }
        }
    }
}

} // namespace ASR
} // namespace Praaline

