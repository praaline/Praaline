#ifndef DICTIONARYPHONETISER_H
#define DICTIONARYPHONETISER_H

#include <QObject>
#include "Phonetiser.h"

namespace Praaline {
namespace ASR {

class DictionaryPhonetiser : public Phonetiser
{
    Q_OBJECT
public:
    explicit DictionaryPhonetiser(QObject *parent = 0);
    ~DictionaryPhonetiser();

    QString phonetiseWord(const QString &word) {}
    QList<Core::Interval *> phonetiseUtterance(Core::Interval *utterance) {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // DICTIONARYPHONETISER_H
