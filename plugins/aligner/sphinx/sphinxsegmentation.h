#ifndef SPHINXSEGMENTATION_H
#define SPHINXSEGMENTATION_H

#include <QObject>
#include <QList>
#include <QHash>
#include "pncore/annotation/intervaltier.h"

class SphinxSegmentation : public QObject
{
    Q_OBJECT
public:
    explicit SphinxSegmentation(QObject *parent = 0);
    ~SphinxSegmentation();

    static bool readSegmentationHypothesisFile(const QString &filename, QHash<QString, QList<Interval *> > &utterances);

signals:

public slots:
};

#endif // SPHINXSEGMENTATION_H
