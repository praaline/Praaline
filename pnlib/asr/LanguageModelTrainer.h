#ifndef LANGUAGEMODELTRAINER_H
#define LANGUAGEMODELTRAINER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class LanguageModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit LanguageModelTrainer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~LanguageModelTrainer() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // LANGUAGEMODELTRAINER_H
