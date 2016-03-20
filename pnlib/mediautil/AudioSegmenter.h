#ifndef AUDIOSEGMENTER_H
#define AUDIOSEGMENTER_H

#include <QString>
#include <QList>
#include "pncore/annotation/interval.h"

class AudioSegmenter
{
public:
    static bool segment(const QString &filenameIn, const QString &pathOut,
                        QList<Interval *> segments, const QString &attribute = QString(),
                        uint newSamplingRate = 0, bool normalise = false, int channels = 1);
    static QList<QStringList> script(const QString &filenameIn, const QString &pathOut,
                                     QList<Interval *> segments, const QString &attribute = QString(),
                                     uint newSamplingRate = 0, bool normalise = false, int channels = 1);
private:
    AudioSegmenter() {}
    ~AudioSegmenter() {}
};

#endif // AUDIOSEGMENTER_H
