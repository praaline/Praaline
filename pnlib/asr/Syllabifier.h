#ifndef SYLLABIFIER_H
#define SYLLABIFIER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class Syllabifier : public QObject
{
    Q_OBJECT
public:
    explicit Syllabifier(QObject *parent = nullptr);

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // SYLLABIFIER_H
