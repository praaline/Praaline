#ifndef SPHINXOFFLINERECOGNISER_H
#define SPHINXOFFLINERECOGNISER_H

#include <QObject>
#include <QString>
#include <QList>
#include "SphinxConfiguration.h"

#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;

namespace Praaline {
namespace ASR {

struct SphinxOfflineRecogniserData;

class SphinxOfflineRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxOfflineRecogniser(QObject *parent = nullptr);
    virtual ~SphinxOfflineRecogniser();

    SphinxConfiguration config() const;
    bool initialize(const SphinxConfiguration &config);

    bool openFeatureFile(const QString &filename);
    bool closeFeatureFile();

    bool decode(int startFrame, int endFrame);
    QString getUtteranceText() const;
    QList<Interval *> getSegmentation() const;

    bool setMLLR(const QString &filenameMLLR);
    bool setLanguageModel(const QString &filenameLM);

protected:


signals:
    void error(const QString &);
    void info(const QString &);

public slots:

private:
    SphinxOfflineRecogniserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXOFFLINERECOGNISER_H
