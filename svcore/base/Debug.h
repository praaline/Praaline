/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2010-2011 Chris Cannam and QMUL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <QDebug>
#include <QTextStream>

#include "PraalineCore/Base/RealTime.h"

#include <string>
#include <iostream>

class QString;
class QUrl;

QDebug &operator<<(QDebug &, const std::string &);
std::ostream &operator<<(std::ostream &, const QString &);
std::ostream &operator<<(std::ostream &, const QUrl &);

using std::cout;
using std::cerr;
using std::endl;

#ifndef NDEBUG

extern QDebug &getSVDebug();

#define SVDEBUG getSVDebug()

inline QDebug &operator<<(QDebug &d, const RealTime &rt) {
    d << rt.toString();
    return d;
}

template <typename T>
inline QDebug &operator<<(QDebug &d, const T &t) {
    QString s;
    QTextStream ts(&s);
    ts << t;
    d << s;
    return d;
}

#else

class NoDebug
{
public:
    inline NoDebug() {}
    inline ~NoDebug(){}

    template <typename T>
    inline NoDebug &operator<<(const T &) { return *this; }

    inline NoDebug &operator<<(QTextStreamFunction) { return *this; }
};

#define SVDEBUG NoDebug()

#endif /* !NDEBUG */

#endif /* !_DEBUG_H_ */

