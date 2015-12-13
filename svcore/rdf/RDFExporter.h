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

#ifndef _RDF_EXPORTER_H_
#define _RDF_EXPORTER_H_

#include <QString>

class Model;
class RDFFeatureWriter;

class RDFExporter
{
public:
    /**
     * Return the file extensions that we can write, in a format
     * suitable for use with QFileDialog.  For example, "*.ttl *.n3".
     */
    static QString getSupportedExtensions();

    RDFExporter(QString path, Model *model);
    virtual ~RDFExporter();

    static bool canExportModel(Model *);

    virtual bool isOK() const;
    virtual QString getError() const;

    virtual void write();

protected:
    QString m_path;
    Model *m_model;
    RDFFeatureWriter *m_fw;
};

#endif
