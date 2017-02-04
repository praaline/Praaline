#ifndef ACOUSTICMODELADAPTER_H
#define ACOUSTICMODELADAPTER_H

#include <QObject>

namespace Praaline {
namespace ASR {

class AcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit AcousticModelAdapter(QObject *parent = 0) : QObject(parent) {}
    virtual ~AcousticModelAdapter() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline


#endif // ACOUSTICMODELADAPTER_H
