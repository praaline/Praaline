#ifndef ACOUSTICFEATUREEXTRACTOR_H
#define ACOUSTICFEATUREEXTRACTOR_H

#include <QObject>

namespace Praaline {
namespace ASR {

class AcousticFeatureExtractor : public QObject
{
    Q_OBJECT
public:
    explicit AcousticFeatureExtractor(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AcousticFeatureExtractor() {}

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // ACOUSTICFEATUREEXTRACTOR_H
