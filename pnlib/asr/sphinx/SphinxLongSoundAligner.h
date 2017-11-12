#ifndef SPHINXLONGSOUNDALIGNER_H
#define SPHINXLONGSOUNDALIGNER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

namespace Praaline {

namespace Core {
class Corpus;
class CorpusCommunication;
}

namespace ASR {

struct SphinxLongSoundAlignerData;

class SphinxLongSoundAligner : public QObject
{
    Q_OBJECT
public:
    explicit SphinxLongSoundAligner(QObject *parent = nullptr);
    virtual ~SphinxLongSoundAligner();



signals:

public slots:

private:
    SphinxLongSoundAlignerData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXLONGSOUNDALIGNER_H
