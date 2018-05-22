#include <QString>
#include <QList>
#include <QHash>
#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "PhonemeTranslation.h"

namespace Praaline {
namespace ASR {

struct PhonemeTranslationData {
    QString nameSource;
    QString nameTarget;
    QHash<QString, QString> phonemesSourceToTarget;
    QHash<QString, QString> phonemesTargetToSource;
};

PhonemeTranslation::PhonemeTranslation() :
    d(new PhonemeTranslationData)
{
}

PhonemeTranslation::PhonemeTranslation(const QString &nameSource, const QString &nameTarget) :
    d(new PhonemeTranslationData)
{
    d->nameSource = nameSource;
    d->nameTarget = nameTarget;
}

PhonemeTranslation::~PhonemeTranslation()
{
    delete d;
}

QString PhonemeTranslation::translate(const QString &phoneme) const
{
    return d->phonemesSourceToTarget.value(phoneme, phoneme);
}

QString PhonemeTranslation::backtranslate(const QString &phoneme) const
{
    return d->phonemesTargetToSource.value(phoneme, phoneme);
}

QString PhonemeTranslation::nameSource() const
{
    return d->nameSource;
}

void PhonemeTranslation::setNameSource(const QString &name)
{
    d->nameSource = name;
}

QString PhonemeTranslation::nameTarget() const
{
    return d->nameTarget;
}

void PhonemeTranslation::setNameTarget(const QString &name)
{
    d->nameTarget = name;
}

void PhonemeTranslation::clear()
{
    d->phonemesSourceToTarget.clear();
    d->phonemesTargetToSource.clear();
}

void PhonemeTranslation::insertTranslation(const QString &phonemeSource, const QString &phonemeTarget)
{
    d->phonemesSourceToTarget.insert(phonemeSource, phonemeTarget);
}

void PhonemeTranslation::insertBacktranslation(const QString &phonemeTarget, const QString &phonemeSource)
{
    d->phonemesTargetToSource.insert(phonemeTarget, phonemeSource);
}

void PhonemeTranslation::insertCommutative(const QString &phonemeA, const QString &phonemeB)
{
    d->phonemesSourceToTarget.insert(phonemeA, phonemeB);
    d->phonemesTargetToSource.insert(phonemeB, phonemeA);
}

bool PhonemeTranslation::read(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject jsonPhonemeTranslation = jsonDoc.object();
    // Header: phoneme translation source and target name
    if (jsonPhonemeTranslation.contains("source") && jsonPhonemeTranslation["source"].isString())
        d->nameSource = jsonPhonemeTranslation["source"].toString();
    if (jsonPhonemeTranslation.contains("target") && jsonPhonemeTranslation["target"].isString())
        d->nameTarget = jsonPhonemeTranslation["target"].toString();
    // Source to Target array
    d->phonemesSourceToTarget.clear();
    if (jsonPhonemeTranslation.contains("translation") && jsonPhonemeTranslation["translation"].isArray()) {
        QJsonArray translationArray = jsonPhonemeTranslation["translation"].toArray();
        for (int translationIndex = 0; translationIndex < translationArray.size(); ++translationIndex) {
            QJsonObject jsonTranslation = translationArray[translationIndex].toObject();
            foreach (QString phonemeSource, jsonTranslation.keys()) {
                QString phonemeTarget = jsonTranslation.value(phonemeSource).toString();
                d->phonemesSourceToTarget.insert(phonemeSource, phonemeTarget);
            }
        }
    }
    // Target to Source array
    d->phonemesTargetToSource.clear();
    if (jsonPhonemeTranslation.contains("backtranslation") && jsonPhonemeTranslation["backtranslation"].isArray()) {
        QJsonArray backtranslationArray = jsonPhonemeTranslation["backtranslation"].toArray();
        for (int backtranslationIndex = 0; backtranslationIndex < backtranslationArray.size(); ++backtranslationIndex) {
            QJsonObject jsonBacktranslation = backtranslationArray[backtranslationIndex].toObject();
            foreach (QString phonemeTarget, jsonBacktranslation.keys()) {
                QString phonemeSource = jsonBacktranslation.value(phonemeTarget).toString();
                d->phonemesTargetToSource.insert(phonemeTarget, phonemeSource);
            }
        }
    }
    return true;
}

bool PhonemeTranslation::write(const QString &filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonObject jsonPhonemeTranslation;
    // Header
    jsonPhonemeTranslation["source"] = d->nameSource;
    jsonPhonemeTranslation["target"] = d->nameTarget;
    // Source to Target array
    QJsonArray translationArray;
    foreach (QString phonemeSource, d->phonemesSourceToTarget.keys()) {
        QJsonObject jsonTranslation;
        jsonTranslation[phonemeSource] = d->phonemesSourceToTarget.value(phonemeSource);
        translationArray.append(jsonTranslation);
    }
    jsonPhonemeTranslation["translation"] = translationArray;
    // Target to Source array
    QJsonArray backtranslationArray;
    foreach (QString phonemeTarget, d->phonemesTargetToSource.keys()) {
        QJsonObject jsonBacktranslation;
        jsonBacktranslation[phonemeTarget] = d->phonemesTargetToSource.value(phonemeTarget);
        backtranslationArray.append(jsonBacktranslation);
    }
    jsonPhonemeTranslation["backtranslation"] = backtranslationArray;
    // Write out JSON document
    QJsonDocument jsonDoc(jsonPhonemeTranslation);
    file.write(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

} // namespace ASR
} // namespace Praaline
