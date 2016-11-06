#ifndef RELATIONTIER_H
#define RELATIONTIER_H

#include <QObject>

namespace Praaline {
namespace Core {

class RelationTier : public QObject
{
    Q_OBJECT
public:
    explicit RelationTier(QObject *parent = 0);
    ~RelationTier();

signals:

public slots:
};

} // namespace Core
} // namespace Praaline

#endif // RELATIONTIER_H
