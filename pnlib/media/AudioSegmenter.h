#ifndef AUDIOSEGMENTER_H
#define AUDIOSEGMENTER_H

#include <QString>
#include <QList>

namespace Praaline {

namespace Core {
    class Interval;
}

namespace Media {

class AudioSegmenter
{
public:
    static bool segment(const QString &filenameIn, const QString &pathOut,
                        QList<Praaline::Core::Interval *> segments, const QString &attribute = QString(),
                        uint newSamplingRate = 0, bool normalise = false, int channels = 1);

    static QList<QStringList> script(const QString &filenameIn, const QString &pathOut,
                                     QList<Praaline::Core::Interval *> segments, const QString &attribute = QString(),
                                     uint newSamplingRate = 0, bool normalise = false, int channels = 1);
    static bool bookmarkCutting(const QString &filenameIn, const QString &pathOut, QList<Praaline::Core::Interval *> intervalsToRemove);
    static bool resample(const QString &filenameIn, const QString &filenameOut,
                         uint newSamplingRate, bool normalise = false, int channels = 1);

private:
    AudioSegmenter() {}
    ~AudioSegmenter() {}
};

} // namespace Media
} // namespace Praaline

#endif // AUDIOSEGMENTER_H
