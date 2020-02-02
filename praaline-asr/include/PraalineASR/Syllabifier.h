#ifndef SYLLABIFIER_H
#define SYLLABIFIER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT Syllabifier : public QObject
{
    Q_OBJECT
public:
    explicit Syllabifier(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Syllabifier() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // SYLLABIFIER_H
