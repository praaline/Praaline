#ifndef ACOUSTICMODELADAPTER_H
#define ACOUSTICMODELADAPTER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT AcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit AcousticModelAdapter(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AcousticModelAdapter() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline


#endif // ACOUSTICMODELADAPTER_H
