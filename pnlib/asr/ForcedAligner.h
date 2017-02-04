#ifndef FORCEDALIGNER_H
#define FORCEDALIGNER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class ForcedAligner : public QObject
{
    Q_OBJECT
public:
    explicit ForcedAligner(QObject *parent = 0) : QObject(parent) {}
    virtual ~ForcedAligner() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // FORCEDALIGNER_H
