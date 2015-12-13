#ifndef TREETIER_H
#define TREETIER_H

#include <QObject>

class TreeTier : public QObject
{
    Q_OBJECT
public:
    explicit TreeTier(QObject *parent = 0);
    ~TreeTier();

signals:

public slots:
};

#endif // TREETIER_H
