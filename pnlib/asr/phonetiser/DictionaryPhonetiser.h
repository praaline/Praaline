#ifndef DICTIONARYPHONETISER_H
#define DICTIONARYPHONETISER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QMap>
#include "pnlib/asr/Phonetiser.h"

namespace Praaline {
namespace ASR {

struct DictionaryPhonetiserData;

class DictionaryPhonetiser : QObject
{
public:
    Q_OBJECT
public:
    explicit DictionaryPhonetiser(QObject *parent = nullptr);
    ~DictionaryPhonetiser();

    bool readPhoneticDictionary(const QString &filename);
    bool readPhonemeTranslation(const QString &filename);
    bool readAdditionalPhoneticDictionary(const QString &filename);

    QString symbolForFalseStarts() const;
    void setSymbolForFalseStarts(const QString &);

    void insertAdditionalPhonetisation(const QString &token, const QString &phonetisation);

    QStringList tokensOutOfVocabulary() const;
    QStringList tokensFalseStarts() const;
    bool writeTokensOutOfVocabulary(const QString &filename) const;
    bool writeTokensFalseStarts(const QString &filename) const;
    bool writeAdditionalPhoneticDictionary(const QString &filename) const;

    void clearState();

    QString phonetiseToken(const QString &token) const;
    void phonetiseTokenList(QHash<QString, QString> &tokens) const;
    void phonetiseTokenList(QMap<QString, QString> &tokens) const;

private:
    DictionaryPhonetiserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // DICTIONARYPHONETISER_H
