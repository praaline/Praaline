#ifndef PHONETISER_H
#define PHONETISER_H

#include <QObject>
#include <QList>

namespace Praaline {

namespace Core {
class Interval;
}

namespace ASR {

class Phonetiser : public QObject
{
    Q_OBJECT
public:
    explicit Phonetiser(QObject *parent = 0) : QObject(parent) {}
    virtual ~Phonetiser() {}

    QString phonetiseWord(const QString &word) = 0;
    QList<Core::Interval *> phonetiseUtterance(Core::Interval *utterance) = 0;

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // PHONETISER_H
