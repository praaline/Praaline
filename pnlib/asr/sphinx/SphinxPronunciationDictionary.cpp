#include <QString>
#include <QList>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QTextStream>

#include "SphinxPronunciationDictionary.h"

struct SphinxPronunciationDictionaryData {
    QMap<QString, QString> dictionary;
    QSet<QString> phonemes;
};

SphinxPronunciationDictionary::SphinxPronunciationDictionary(QObject *parent) :
    QObject(parent), d(new SphinxPronunciationDictionaryData)
{
}

SphinxPronunciationDictionary::~SphinxPronunciationDictionary()
{
    delete d;
}

bool SphinxPronunciationDictionary::readFromFile(const QString &filename)
{
    QFile fileDictIn(filename);
    if ( !fileDictIn.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream dic(&fileDictIn);
    dic.setCodec("UTF-8");

    d->dictionary.clear();
    d->phonemes.clear();

    while (!dic.atEnd()) {
        QString line = dic.readLine();
        if (!line.contains(" ")) continue;
        QString word = line.section(" ", 0, 1);
        QStringList phonetisation = line.section(" ", 1, -1).split(" ");
        d->phonemes.intersect(QSet<QString>::fromList(phonetisation));
        d->dictionary.insert(word, phonetisation.join(" "));
    }
    fileDictIn.close();

    return true;
}

bool SphinxPronunciationDictionary::writeToFile(const QString &filename) const
{
    QFile fileDictOut(filename);
    if ( !fileDictOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream dictOut(&fileDictOut);
    dictOut.setCodec("UTF-8");
    foreach (QString word, d->dictionary.keys()) {
        dictOut << word << " " << d->dictionary.value(word) << "\n";
    }
    fileDictOut.close();
    return true;
}

QList<QString> SphinxPronunciationDictionary::phonemes() const
{
    return d->phonemes.toList();
}

QString SphinxPronunciationDictionary::phonetise(const QString &word) const
{
    return d->dictionary.value(word);
}

void SphinxPronunciationDictionary::addWord(const QString &word, const QString &phonetisation)
{
    d->phonemes.intersect(QSet<QString>::fromList(phonetisation.split(" ")));
    d->dictionary.insert(word, phonetisation);
}

bool SphinxPronunciationDictionary::contains(const QString &word) const
{
    return d->dictionary.contains(word);
}
