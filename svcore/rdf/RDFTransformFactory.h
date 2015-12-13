/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RDF_TRANSFORM_FACTORY_H_
#define _RDF_TRANSFORM_FACTORY_H_

#include <QObject>
#include <QString>

#include <vector>

#include "transform/Transform.h"

class RDFTransformFactoryImpl;
class ProgressReporter;

class RDFTransformFactory : public QObject
{
    Q_OBJECT

public:
    static QString getKnownExtensions();

    RDFTransformFactory(QString url);
    virtual ~RDFTransformFactory();

    bool isRDF(); // true if the file was parseable and had transforms in it
    bool isOK();  // true if the transforms could be completely constructed
    QString getErrorString() const;

    std::vector<Transform> getTransforms(ProgressReporter *reporter);

    static QString writeTransformToRDF(const Transform &, QString uri);

protected:
    RDFTransformFactoryImpl *m_d;
};

#endif
