#include <QString>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QTemporaryDir>
#include <QCoreApplication>
#include "languagemodel.h"

LanguageModel::LanguageModel()
{
}

LanguageModel::~LanguageModel()
{
}

// static
bool LanguageModel::createSentencesAndVocabularyFromTokens(const QString &filenameBase, QList<Interval *> tokens)
{
    QList<QString> vocabulary;

    QFile fileSentences(filenameBase + ".sent");
    if ( !fileSentences.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream sentences(&fileSentences);
    sentences.setCodec("UTF-8");

    QString utterance;
    for (int i = 0; i < tokens.count(); ++i) {
        Interval *token = tokens[i];
        Interval *next = (i < tokens.count() - 1) ? tokens[i+1] : tokens[i];
        if (token->attribute("speakerID") != next->attribute("speakerID")) {
            if (!(utterance.trimmed().isEmpty())) {
                sentences << "<s> " << utterance.trimmed() << " </s>\n";
                utterance.clear();
            }
        }
        else if (token->isPauseSilent()) {
            if ( (token->duration().toDouble() >= 0.250) && (!(utterance.trimmed().isEmpty())) ) {
                sentences << "<s> " << utterance.trimmed() << " </s>\n";
                utterance.clear();
            }
        }
        else {
            QString t = token->text();
            t = t.replace(".", "").replace(",", "").replace("/", "").replace("=", "").replace("!", "").replace("?", "")
                    .replace("*", "").replace("+", "");
            t = t.replace(" ", "_").replace("|", " ").trimmed(); // parce_que il|y a
            if (t.isEmpty()) continue;
            if (t.startsWith("(") || t.endsWith(")")) continue;
            if (t.startsWith("[") || t.endsWith("]")) continue;
            if (t.startsWith("{") || t.endsWith("}")) continue;
            if (t.startsWith("<") || t.endsWith(">")) continue;
            if (t == "-") continue;
            utterance.append(t).append(" ");
            if (!vocabulary.contains(t)) vocabulary.append(t);
        }
    }
    fileSentences.close();
    qSort(vocabulary);

    QFile fileVocabulary(filenameBase + ".vocab");
    if ( !fileVocabulary.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream vocab(&fileVocabulary);
    vocab.setCodec("UTF-8");
    vocab << "<s>\n";
    vocab << "</s>\n";
    foreach (QString word, vocabulary) {
        if (word.isEmpty()) continue;
        vocab << word << "\n";
    }
    fileVocabulary.close();

    return true;
}

bool LanguageModel::createModel(const QString &filenameBase, bool createDMP)
{
    // Create n-gram file from sentences and vocabulary
    // DIRECTORY:
    QString appPath = QCoreApplication::applicationDirPath();
    QString cmulmtkPath = "D:/SPHINX/cmuclmtk-0.7-win32/"; //appPath + "/plugins/longalign/";

    QProcess text2idngram;
    text2idngram.setWorkingDirectory(cmulmtkPath);
    text2idngram.setStandardInputFile(filenameBase + ".sent");
    text2idngram.start(cmulmtkPath + "text2idngram.exe" , QStringList() <<
                       "-vocab" << filenameBase + ".vocab" <<
                       "-idngram" << filenameBase + ".idngram");
    if (!text2idngram.waitForStarted(-1)) return false;
    if (!text2idngram.waitForFinished(-1)) return false;

    QProcess idngram2lm;
    idngram2lm.setWorkingDirectory(cmulmtkPath);
    idngram2lm.start(cmulmtkPath + "idngram2lm.exe" , QStringList() <<
                     "-vocab_type" << "0" <<
                     "-vocab" << filenameBase + ".vocab" <<
                     "-idngram" << filenameBase + ".idngram" <<
                     "-arpa" << filenameBase + ".lm");
    if (!idngram2lm.waitForStarted(-1)) return false;
    if (!idngram2lm.waitForFinished(-1)) return false;

    if (createDMP) {
        QProcess idngram2lm;
        idngram2lm.setWorkingDirectory(cmulmtkPath);
        idngram2lm.start(cmulmtkPath + "sphinx_lm_convert.exe" , QStringList() <<
                         "-i" << filenameBase + ".lm" <<
                         "-o" << filenameBase + ".lm.dmp");
        if (!idngram2lm.waitForStarted(-1)) return false;
        if (!idngram2lm.waitForFinished(-1)) return false;
    }

    return true;
}


//text2wfreq < transcript.txt | wfreq2vocab >  transcript.vocab
//d'
//qu'
//parce que	parce_que
//text2idngram -vocab transcript.vocab -idngram transcript.idngram < transcript.txt
//idngram2lm -vocab_type 0 -idngram transcript.idngram -vocab transcript.vocab -arpa transcript.lm
//sphinx_lm_convert -i transcript.lm -o transcript.lm.dmp

