/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "XmlExportable.h"
#include <map>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <iostream>

QString
XmlExportable::toXmlString(QString indent,
                           QString extraAttributes) const
{
//    cerr << "XmlExportable::toXmlString" << endl;

    QString s;

    {
        QTextStream out(&s);
        toXml(out, indent, extraAttributes);
    }

    return s;
}

QString
XmlExportable::encodeEntities(QString s)
{
    s
	.replace("&", "&amp;")
	.replace("<", "&lt;")
	.replace(">", "&gt;")
	.replace("\"", "&quot;")
	.replace("'", "&apos;");

    return s;
}

QString
XmlExportable::encodeColour(int ri, int gi, int bi)
{
    QString r, g, b;

    r.setNum(ri, 16);
    if (ri < 16) r = "0" + r;

    g.setNum(gi, 16);
    if (gi < 16) g = "0" + g;

    b.setNum(bi, 16);
    if (bi < 16) b = "0" + b;

    return "#" + r + g + b;
}

int
XmlExportable::getObjectExportId(const void * object)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static std::map<const void *, int> idMap;
    static int maxId = 0;
    
    if (idMap.find(object) == idMap.end()) {
	idMap[object] = maxId++;
    }

    return idMap[object];
}


