#ifndef ACOUSTICMODELTRAINER_H
#define ACOUSTICMODELTRAINER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class AcousticModelTrainer : public QObject
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
