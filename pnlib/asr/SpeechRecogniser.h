#ifndef SPEECHRECOGNISER_H
#define SPEECHRECOGNISER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class SpeechRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SpeechRecogniser(QObject *parent = nullptr) {}
    virtual ~SpeechRecogniser() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHRECOGNISER_H
