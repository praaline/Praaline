#ifndef ABSTRACTLANGUAGEMODELTRAINER_H
#define ABSTRACTLANGUAGEMODELTRAINER_H

#include <QObject>

namespace Praaline {
namespace Lib {

class AbstractLanguageModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractLanguageModelTrainer(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractLanguageModelTrainer() {}

signals:

public slots:
};

} // namespace Lib
} // namespace Praaline

#endif // ABSTRACTLANGUAGEMODELTRAINER_H
