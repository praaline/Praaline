#include <QString>
#include <QList>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QTextStream>

#include "PraalineASR/Sphinx/SphinxPronunciationDictionary.h"

namespace Praaline {
namespace ASR {

struct SphinxPronunciationDictionaryData {
    QMap<QString, QList<QString> > dictionary;
    QSet<QString> phonemes;
};

SphinxPronunciationDictionary::SphinxPronunciationDictionary() :
    d(new SphinxPronunciationDictionaryData)
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
        while (line.contains("  ")) line = line.replace("  ", " ");
        QString word_and_id = line.section(" ", 0, 0);
        QString word;
        if (word_and_id.contains("("))
            word = word_and_id.section("(", 0, 0).trimmed();
        else
            word = word_and_id.trimmed();
        QStringList phonetisation = line.section(" ", 1, -1).split(" ");
        d->phonemes.unite(phonetisation.toSet());
        QString phonetisation_text = phonetisation.join(" ");
        if (!d->dictionary.contains(word)) {
            d->dictionary.insert(word, QList<QString>() << phonetisation_text);
        } else {
            if (!d->dictionary[word].contains(phonetisation_text))
                d->dictionary[word].append(phonetisation_text);
        }
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
        if (d->dictionary.value(word).count() == 1) {
            dictOut << word << " " << d->dictionary.value(word).first() << "\n";
        }
        else if (d->dictionary.value(word).count() > 1) {
            for (int id = 1; id <= d->dictionary.value(word).count(); ++id) {
                dictOut << word;
                if (id > 1) dictOut << QString("(%1)").arg(id);
                dictOut << " " << d->dictionary.value(word).at(id) << "\n";
            }
        }
    }
    fileDictOut.close();
    return true;
}

QList<QString> SphinxPronunciationDictionary::phonemes() const
{
    return d->phonemes.values();
}

bool SphinxPronunciationDictionary::writePhonemeList(const QString &filename) const
{
    QFile filePhonemesOut(filename);
    if ( !filePhonemesOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream phonemesOut(&filePhonemesOut);
    phonemesOut.setCodec("UTF-8");
    foreach (QString phoneme, d->phonemes) {
        phonemesOut << phoneme << "\n";
    }
    filePhonemesOut.close();
    return true;
}

QString SphinxPronunciationDictionary::phonetise(const QString &word) const
{
    return d->dictionary.value(word, QList<QString>()).join(" | ");
}

void SphinxPronunciationDictionary::addWord(const QString &word, const QString &phonetisation)
{
//    d->phonemes.intersect(QSet<QString>::fromList(phonetisation.split(" ")));
//    d->dictionary.insert(word, phonetisation);
}

bool SphinxPronunciationDictionary::contains(const QString &word) const
{
    return d->dictionary.contains(word);
}

} // namespace ASR
} // namespace Praaline
