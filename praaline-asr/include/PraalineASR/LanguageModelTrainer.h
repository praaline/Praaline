#ifndef LANGUAGEMODELTRAINER_H
#define LANGUAGEMODELTRAINER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT LanguageModelTrainer : public QObject
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
