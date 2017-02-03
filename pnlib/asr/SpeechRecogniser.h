#ifndef ABSTRACTRECOGNISER_H
#define ABSTRACTRECOGNISER_H

#include <QObject>

namespace Praaline {
namespace Lib {

class AbstractRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit AbstractRecogniser(QObject *parent = 0) {}
    virtual ~AbstractRecogniser() {}

signals:

public slots:
};

} // namespace Lib
} // namespace Praaline

#endif // ABSTRACTRECOGNISER_H
