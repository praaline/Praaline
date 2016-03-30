#ifndef SPHINXONLINERECOGNISER_H
#define SPHINXONLINERECOGNISER_H

#include <QObject>

class SphinxOnlineRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxOnlineRecogniser(QObject *parent = 0);

signals:

public slots:
};

#endif // SPHINXONLINERECOGNISER_H