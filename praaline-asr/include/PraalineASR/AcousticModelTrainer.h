#ifndef ACOUSTICMODELTRAINER_H
#define ACOUSTICMODELTRAINER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT AcousticModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit AcousticModelTrainer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AcousticModelTrainer() {}

signals:

public slots:
};


} // namespace ASR
} // namespace Praaline

#endif // ACOUSTICMODELTRAINER_H
