#ifndef WEBDESIGNERMODE_H
#define WEBDESIGNERMODE_H

#include <QObject>

class WebDesignerMode : public QObject
{
    Q_OBJECT
public:
    explicit WebDesignerMode(QObject *parent = nullptr);

signals:

public slots:
};

#endif // WEBDESIGNERMODE_H