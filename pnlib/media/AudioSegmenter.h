#ifndef AUDIOSEGMENTER_H
#define AUDIOSEGMENTER_H

#include <QString>
#include <QList>
#include "pncore/base/RealTime.h"

namespace Praaline {

namespace Core {
    class Interval;
}

namespace Media {

class AudioSegmenter
{
public:
    static bool resample(const QString &filenameIn, const QString &filenameOut,
                         uint newSamplingRate, bool normalise = false, int channels = 1);
    static bool segment(const QString &filenameIn, const QString &pathOut,
                        QList<Praaline::Core::Interval *> segments, const QString &attribute = QString(),
                        uint newSamplingRate = 0, bool normalise = false, int channels = 1);
    static bool segment(const QString &filenameIn, const QString &filenameOut,
                        RealTime timeFrom, RealTime timeTo,
                        uint newSamplingRate = 0, bool normalise = false, int channels = 1);

    static QList<QStringList> script(const QString &filenameIn, const QString &pathOut,
                                     QList<Praaline::Core::Interval *> segments, const QString &attribute = QString(),
                                     uint newSamplingRate = 0, bool normalise = false, int channels = 1);
    static bool bookmarkCutting(const QString &filenameIn, const QString &pathOut, QList<Praaline::Core::Interval *> intervalsToRemove);

    static bool runSoxCommand(const QString &command);
    static bool generateSilence(const QString &filenameOut, RealTime duration);

private:
    AudioSegmenter() {}
    ~AudioSegmenter() {}
};

} // namespace Media
} // namespace Praaline

#endif // AUDIOSEGMENTER_H
