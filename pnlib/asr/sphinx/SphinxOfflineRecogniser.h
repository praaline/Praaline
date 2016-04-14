#ifndef SPHINXOFFLINERECOGNISER_H
#define SPHINXOFFLINERECOGNISER_H

#include <QObject>
#include <QString>
#include <QList>
#include "SphinxConfiguration.h"

struct SphinxOfflineRecogniserData;
class Interval;

class SphinxOfflineRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit SphinxOfflineRecogniser(QObject *parent = 0);
    virtual ~SphinxOfflineRecogniser();

    SphinxConfiguration config() const;
    bool initialize(const SphinxConfiguration &config);

    bool openFeatureFile(const QString &filename);
    bool closeFeatureFile();

    bool decode(int startFrame, int endFrame);
    QString getUtterance() const;
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

#endif // SPHINXOFFLINERECOGNISER_H
