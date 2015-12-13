#ifndef ABSTRACTFEATUREEXTRACTOR_H
#define ABSTRACTFEATUREEXTRACTOR_H

#include <QObject>

class AbstractFeatureExtractor : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFeatureExtractor(QObject *parent = 0) {}
    virtual ~AbstractFeatureExtractor() {}

signals:

public slots:
};

#endif // ABSTRACTFEATUREEXTRACTOR_H
