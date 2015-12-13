#ifndef ABSTRACTACOUSTICMODELADAPTER_H
#define ABSTRACTACOUSTICMODELADAPTER_H

#include <QObject>

class AbstractAcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit AbstractAcousticModelAdapter(QObject *parent = 0) {}
    virtual ~AbstractAcousticModelAdapter() {}

signals:

public slots:
};

#endif // ABSTRACTACOUSTICMODELADAPTER_H
