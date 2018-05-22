#ifndef SPHINXPRONUNCIATIONDICTIONARY_H
#define SPHINXPRONUNCIATIONDICTIONARY_H

#include <QString>
#include <QList>

namespace Praaline {
namespace ASR {

struct SphinxPronunciationDictionaryData;

class SphinxPronunciationDictionary
{
public:
    explicit SphinxPronunciationDictionary();
    virtual ~SphinxPronunciationDictionary();

    bool readFromFile(const QString &filename);
    bool writeToFile(const QString &filename) const;
    QList<QString> phonemes() const;
    bool writePhonemeList(const QString &filename) const;

    QString phonetise(const QString &word) const;
    void addWord(const QString &word, const QString &phonetisation);
    bool contains(const QString &word) const;

private:
    SphinxPronunciationDictionaryData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXPRONUNCIATIONDICTIONARY_H
