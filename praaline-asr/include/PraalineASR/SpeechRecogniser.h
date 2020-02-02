#ifndef SPEECHRECOGNISER_H
#define SPEECHRECOGNISER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SpeechRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SpeechRecogniser(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~SpeechRecogniser() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHRECOGNISER_H
