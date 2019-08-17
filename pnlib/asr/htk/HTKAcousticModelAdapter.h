#ifndef HTKACOUSTICMODELADAPTER_H
#define HTKACOUSTICMODELADAPTER_H

#include <QObject>
#include "pnlib/asr/AcousticModelAdapter.h"

namespace Praaline {
namespace ASR {

class HTKAcousticModelAdapter : public AcousticModelAdapter
{
    Q_OBJECT
public:
    explicit HTKAcousticModelAdapter(QObject *parent = nullptr);
    ~HTKAcousticModelAdapter();

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // HTKACOUSTICMODELADAPTER_H
