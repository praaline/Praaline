#include <QString>
#include <QList>
#include <QStringList>
#include <QFile>
#include <QTextStream>

#include "DictionaryPhonetiser.h"

#include "pnlib/asr/phonetiser/PhonemeTranslation.h"
#include "pnlib/asr/sphinx/SphinxPronunciationDictionary.h"

namespace Praaline {
namespace ASR {

struct DictionaryPhonetiserData {
    SphinxPronunciationDictionary sphinxDictionary;
    PhonemeTranslation phonemeTranslation;
    QString symbolFalseStart;
    QStringList wordsFalseStarts;
    QStringList wordsOOV;
    QHash<QString, QString> additionalDictionary;
};

DictionaryPhonetiser::DictionaryPhonetiser(QObject *parent) :
    QObject(parent), d(new DictionaryPhonetiserData())
{
    // Defaults
    d->symbolFalseStart = "/";
}

DictionaryPhonetiser::~DictionaryPhonetiser()
{
    delete d;
}

bool DictionaryPhonetiser::readPhoneticDictionary(const QString &filename)
{
    return d->sphinxDictionary.readFromFile(filename);
}

bool DictionaryPhonetiser::readPhonemeTranslation(const QString &filename)
{
    return d->phonemeTranslation.read(filename);
}

bool DictionaryPhonetiser::readAdditionalPhoneticDictionary(const QString &filename)
{
    QFile fileAdditional(filename);
    if ( !fileAdditional.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream dic(&fileAdditional);
    dic.setCodec("UTF-8");
    while (!dic.atEnd()) {
        QString line = dic.readLine();
        if (!line.contains("\t")) continue;
        while (line.contains("  ")) line = line.replace("  ", " ");
        QString token = line.section("\t", 0, 0).trimmed();
        QString phonetisation = line.section("\t", 1, 1).trimmed();
        d->additionalDictionary.insert(token, phonetisation);
    }
    fileAdditional.close();
    return true;
}

QString DictionaryPhonetiser::symbolForFalseStarts() const
{
    return d->symbolFalseStart;
}

void DictionaryPhonetiser::setSymbolForFalseStarts(const QString &symbol)
{
    d->symbolFalseStart = symbol;
}

QStringList DictionaryPhonetiser::tokensOutOfVocabulary() const
{
    return  d->wordsOOV;
}

QStringList DictionaryPhonetiser::tokensFalseStarts() const
{
    return d->wordsFalseStarts;
}

void DictionaryPhonetiser::insertAdditionalPhonetisation(const QString &token, const QString &phonetisation)
{
    d->additionalDictionary.insert(token, phonetisation);
    if (d->wordsFalseStarts.contains(token)) d->wordsFalseStarts.removeOne(token);
    if (d->wordsOOV.contains(token)) d->wordsOOV.removeOne(token);
}

bool DictionaryPhonetiser::writeTokensOutOfVocabulary(const QString &filename) const
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString word, d->wordsOOV) {
        out << word << "\n";
    }
    fileOut.close();
    return true;
}

bool DictionaryPhonetiser::writeTokensFalseStarts(const QString &filename) const
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString word, d->wordsFalseStarts) {
        out << word << "\n";
    }
    fileOut.close();
    return true;
}

bool DictionaryPhonetiser::writeAdditionalPhoneticDictionary(const QString &filename) const
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString word, d->additionalDictionary.keys()) {
        out << word << "\t" << d->additionalDictionary.value(word) << "\n";
    }
    fileOut.close();
    return true;
}

void DictionaryPhonetiser::clearState()
{
    d->wordsFalseStarts.clear();
    d->wordsOOV.clear();
}

QString DictionaryPhonetiser::phonetiseToken(const QString &token) const
{
    QString phonetisationsSphinx;
    // Additional dictionary overrides base dictionary
    if (d->additionalDictionary.contains(token))
        phonetisationsSphinx = d->additionalDictionary.value(token);
    else {
        // Check in Sphinx pronunciation dictionary
        phonetisationsSphinx = d->sphinxDictionary.phonetise(QString(token).replace(" ", "_"));
    }
    // Not found: check if false start or OOV
    if (phonetisationsSphinx.isEmpty()) {
        if (token.endsWith(d->symbolFalseStart)) {
            if (!d->wordsFalseStarts.contains(token)) d->wordsFalseStarts << token;
        } else {
            if (!d->wordsOOV.contains(token)) d->wordsOOV << token;
        }
        return QString();
    }
    // Found: translate phonemes if needed
    QStringList phonetisations;
    foreach (QString phonetisationSphinx, phonetisationsSphinx.split("|")) {
        QString phonetisation;
        foreach (QString phoneSphinx, phonetisationSphinx.split(" ", QString::SkipEmptyParts)) {
            QString phone = d->phonemeTranslation.translate(phoneSphinx);
            phonetisation.append(phone).append(" ");
        }
        phonetisations << phonetisation.trimmed();
    }
    return phonetisations.join(" | ");
}

void DictionaryPhonetiser::phonetiseTokenList(QHash<QString, QString> &tokens) const
{
    foreach (QString token, tokens.keys()) {
        tokens[token] = phonetiseToken(token);
    }
}

void DictionaryPhonetiser::phonetiseTokenList(QMap<QString, QString> &tokens) const
{
    foreach (QString token, tokens.keys()) {
        tokens[token] = phonetiseToken(token);
    }
}

} // namespace ASR
} // namespace Praaline
