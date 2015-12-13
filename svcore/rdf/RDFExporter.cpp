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

#include "RDFExporter.h"
#include "RDFFeatureWriter.h"

#include <vamp-hostsdk/Plugin.h>

#include "data/model/Model.h"
#include "data/model/RegionModel.h"
#include "data/model/NoteModel.h"
#include "data/model/SparseOneDimensionalModel.h"
#include "data/model/SparseTimeValueModel.h"
#include "data/model/TextModel.h"
#include "data/model/EditableDenseThreeDimensionalModel.h"

bool
RDFExporter::canExportModel(Model *m)
{
    if (dynamic_cast<RegionModel *>(m)) return true;
    if (dynamic_cast<NoteModel *>(m)) return true;
    if (dynamic_cast<SparseTimeValueModel *>(m)) return true;
    if (dynamic_cast<SparseOneDimensionalModel *>(m)) return true;
    if (dynamic_cast<TextModel *>(m)) return true;
    // no, looks like we never implemented this one
//    if (dynamic_cast<EditableDenseThreeDimensionalModel *>(m)) return true;
    return false;
}

RDFExporter::RDFExporter(QString path, Model *m) :
    m_path(path),
    m_model(m),
    m_fw(new RDFFeatureWriter())
{
    map<string, string> params;
    params["one-file"] = path.toStdString();
    params["force"] = "true";
    m_fw->setParameters(params);
}

RDFExporter::~RDFExporter()
{
    delete m_fw;
}

bool
RDFExporter::isOK() const
{
    return true;
}

QString
RDFExporter::getError() const
{
    return "";
}

void
RDFExporter::write()
{
    QString trackId; // nil
    Transform transform; // nil
    Vamp::Plugin::OutputDescriptor output; // nil
    std::string summaryType; // nil

    Vamp::Plugin::FeatureList features;
    features.push_back(Vamp::Plugin::Feature());
    Vamp::Plugin::Feature &f = features[0];
    sv_samplerate_t sr = m_model->getSampleRate();

    {
        RegionModel *m = dynamic_cast<RegionModel *>(m_model);
        if (m) {
            f.hasTimestamp = true;
            f.hasDuration = true;
            const RegionModel::PointList &pl(m->getPoints());
            for (RegionModel::PointList::const_iterator i = pl.begin(); 
                 i != pl.end(); ++i) {
                f.timestamp = RealTime::frame2RealTime(i->frame, sr).toVampRealTime();
                f.duration = RealTime::frame2RealTime(i->duration, sr).toVampRealTime();
                f.values.clear();
                f.values.push_back(i->value);
                f.label = i->label.toStdString();
                m_fw->write(trackId, transform, output, features, summaryType);
            }
            return;
        }
    }
    {
        NoteModel *m = dynamic_cast<NoteModel *>(m_model);
        if (m) {
            f.hasTimestamp = true;
            f.hasDuration = true;
            const NoteModel::PointList &pl(m->getPoints());
            for (NoteModel::PointList::const_iterator i = pl.begin(); 
                 i != pl.end(); ++i) {
                f.timestamp = RealTime::frame2RealTime(i->frame, sr).toVampRealTime();
                f.duration = RealTime::frame2RealTime(i->duration, sr).toVampRealTime();
                f.values.clear();
                f.values.push_back(i->value);
                f.values.push_back(i->level);
                f.label = i->label.toStdString();
                m_fw->write(trackId, transform, output, features, summaryType);
            }
            return;
        }
    }
    {     
        SparseOneDimensionalModel *m = dynamic_cast<SparseOneDimensionalModel *>(m_model);
        if (m) {
            f.hasTimestamp = true;
            f.hasDuration = false;
            const SparseOneDimensionalModel::PointList &pl(m->getPoints());
            for (SparseOneDimensionalModel::PointList::const_iterator i = pl.begin(); 
                 i != pl.end(); ++i) {
                f.timestamp = RealTime::frame2RealTime(i->frame, sr).toVampRealTime();
                f.values.clear();
                f.label = i->label.toStdString();
                m_fw->write(trackId, transform, output, features, summaryType);
            }
            return;
        }
    }
    {            
        SparseTimeValueModel *m = dynamic_cast<SparseTimeValueModel *>(m_model);
        if (m) {
            f.hasTimestamp = true;
            f.hasDuration = false;
            const SparseTimeValueModel::PointList &pl(m->getPoints());
            for (SparseTimeValueModel::PointList::const_iterator i = pl.begin(); 
                 i != pl.end(); ++i) {
                f.timestamp = RealTime::frame2RealTime(i->frame, sr).toVampRealTime();
                f.values.clear();
                f.values.push_back(i->value);
                f.label = i->label.toStdString();
                m_fw->write(trackId, transform, output, features, summaryType);
            }
            return;
        }
    }
    {     
        TextModel *m = dynamic_cast<TextModel *>(m_model);
        if (m) {
            f.hasTimestamp = true;
            f.hasDuration = false;
            const TextModel::PointList &pl(m->getPoints());
            m_fw->setFixedEventTypeURI("af:Text");
            for (TextModel::PointList::const_iterator i = pl.begin(); 
                 i != pl.end(); ++i) {
                f.timestamp = RealTime::frame2RealTime(i->frame, sr).toVampRealTime();
                f.values.clear();
                f.values.push_back(i->height);
                f.label = i->label.toStdString();
                m_fw->write(trackId, transform, output, features, summaryType);
            }
            return;
        }
    }
}

QString
RDFExporter::getSupportedExtensions()
{
    return "*.ttl *.n3";
}

