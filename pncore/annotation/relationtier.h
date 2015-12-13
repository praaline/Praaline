#ifndef RELATIONTIER_H
#define RELATIONTIER_H

#include <QObject>

class RelationTier : public QObject
{
    Q_OBJECT
public:
    explicit RelationTier(QObject *parent = 0);
    ~RelationTier();

signals:

public slots:
};

#endif // RELATIONTIER_H
