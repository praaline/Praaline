#ifndef HTKACOUSTICMODELADAPTER_H
#define HTKACOUSTICMODELADAPTER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/AcousticModelAdapter.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT HTKAcousticModelAdapter : public AcousticModelAdapter
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
