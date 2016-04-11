#ifndef SOXEXTERNAL_H
#define SOXEXTERNAL_H

#include <QObject>

class SoxExternal : public QObject
{
    Q_OBJECT
public:
    explicit SoxExternal(QObject *parent = 0);

signals:
    void error(QString msg);
    void status(QString msg);

public slots:
    QString bin();
    QString about();
    QString dat(QString filename, float fps, int duration, bool x, bool y, int xFactor, int yFactor, bool forceNull);
    QString formats();
    float duration(QString filename, float fps);
};



#endif // SOXEXTERNAL_H
