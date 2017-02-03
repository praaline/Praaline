#ifndef HTKACOUSTICMODELADAPTER_H
#define HTKACOUSTICMODELADAPTER_H

#include <QObject>
#include "pnlib/asr/AcousticModelAdapter.h"

class HTKAcousticModelAdapter : public AbstractAcousticModelAdapter
{
    Q_OBJECT
public:
    explicit HTKAcousticModelAdapter(QObject *parent = 0);
    ~HTKAcousticModelAdapter();

signals:

public slots:
};

#endif // HTKACOUSTICMODELADAPTER_H
