#ifndef SPEECHTOKEN_H
#define SPEECHTOKEN_H

#include <QString>
#include "pncore/base/RealTime.h"

namespace Praaline {
namespace ASR {

class SpeechToken
{
public:
    SpeechToken (int start, int end, QString orthographic) : start(start), end(end), orthographic(orthographic) {}
    ~SpeechToken() {}

    int start;
    int end;
    QString orthographic;
    QString phonetisation;
};

class SpeechPhone
{
public:
    SpeechPhone (QString phone, RealTime start, RealTime end, double scoreAM, QString token) :
        phone(phone), start(start), end(end), scoreAM(scoreAM), token(token) {}

    QString phone;
    RealTime start;
    RealTime end;
    double scoreAM;
    QString token;
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHTOKEN_H
