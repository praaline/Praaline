#ifndef SPHINXCONTINUOUSRECOGNISER_H
#define SPHINXCONTINUOUSRECOGNISER_H

#include <QObject>
#include "PraalineASR/PraalineASR_Global.h"

class PRAALINE_ASR_SHARED_EXPORT SphinxContinuousRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxContinuousRecogniser(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SPHINXCONTINUOUSRECOGNISER_H
