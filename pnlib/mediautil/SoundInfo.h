#ifndef SOUNDINFO_H
#define SOUNDINFO_H

#include <QString>
#include <QPair>
#include "base/RealTime.h"


class SoundInfo
{
public:
    SoundInfo();
    ~SoundInfo();

    QString filename;
    int channels;
    int sampleRate;
    int precisionBits;
    RealTime duration;
    int bitRate;
    QString encoding;
    long long filesize;
    QString checksumMD5;

    static bool getSoundInfo(const QString &filename, SoundInfo &info);
    static QPair<long long, QString> getSizeAndMD5(const QString &filename);

};

#endif // SOUNDINFO_H
