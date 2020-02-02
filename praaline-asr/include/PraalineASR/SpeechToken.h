#ifndef SPEECHTOKEN_H
#define SPEECHTOKEN_H

#include <QString>
#include <QStringList>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SpeechToken
{
public:
    SpeechToken (int start, int end, QString orthographic) :
        start(start), end(end), orthographic(orthographic)
    {}

    int start;
    int end;
    QString orthographic;
    QStringList phonetisations;
};

class PRAALINE_ASR_SHARED_EXPORT SpeechPhone
{
public:
    SpeechPhone (QString phone, RealTime start, RealTime end, double scoreAM,
                 QString token, bool isTokenStart = false) :
        phone(phone), start(start), end(end), scoreAM(scoreAM),
        token(token), isTokenStart(isTokenStart)
    {}

    QString phone;
    RealTime start;
    RealTime end;
    double scoreAM;
    QString token;
    bool isTokenStart;

    RealTime duration() { return end - start; }
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHTOKEN_H
