#ifndef ALIGNERTOKEN_H
#define ALIGNERTOKEN_H

#include <QString>
#include "pncore/base/RealTime.h"

class AlignerToken
{
public:
    AlignerToken (int start, int end, QString orthographic) : start(start), end(end), orthographic(orthographic) {}
    ~AlignerToken() {}

    int start;
    int end;
    QString orthographic;
    QString phonetisation;
};

class AlignerPhone
{
public:
    AlignerPhone (QString phone, RealTime start, RealTime end, double scoreAM, QString token) :
        phone(phone), start(start), end(end), scoreAM(scoreAM), token(token) {}

    QString phone;
    RealTime start;
    RealTime end;
    double scoreAM;
    QString token;
};

#endif // ALIGNERTOKEN_H
