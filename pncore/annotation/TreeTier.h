#ifndef TREETIER_H
#define TREETIER_H

#include <QObject>

namespace Praaline {
namespace Core {

class TreeTier : public QObject
{
    Q_OBJECT
public:
    explicit TreeTier(QObject *parent = 0);
    ~TreeTier();

signals:

public slots:
};

} // namespace Core
} // namespace Praaline

#endif // TREETIER_H
