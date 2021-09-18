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

#include "Model.h"
#include "AlignmentModel.h"

#include <QTextStream>

#include <iostream>

const int Model::COMPLETION_UNKNOWN = -1;

Model::~Model()
{
    //    cerr << "Model::~Model(" << this << ")" << endl;

    if (!m_aboutToDelete) {
        cerr << "NOTE: Model::~Model(" << this << ", \""
                << objectName() << "\"): Model deleted "
                << "with no aboutToDelete notification" << endl;
    }

    if (m_alignment) {
        m_alignment->aboutToDelete();
        delete m_alignment;
    }
}

void
Model::setSourceModel(Model *model)
{
    if (m_sourceModel) {
        disconnect(m_sourceModel, &Model::aboutToBeDeleted,
                   this, &Model::sourceModelAboutToBeDeleted);
    }

    m_sourceModel = model;

    if (m_sourceModel) {
        connect(m_sourceModel, &Model::alignmentCompletionChanged,
                this, &Model::alignmentCompletionChanged);
        connect(m_sourceModel, &Model::aboutToBeDeleted,
                this, &Model::sourceModelAboutToBeDeleted);
    }
}

void
Model::aboutToDelete()
{
    //    cerr << "Model(" << this << ")::aboutToDelete()" << endl;

    if (m_aboutToDelete) {
        cerr << "WARNING: Model(" << this << ", \""
             << objectName() << "\")::aboutToDelete: "
             << "aboutToDelete called more than once for the same model"
             << endl;
    }

    emit aboutToBeDeleted();
    m_aboutToDelete = true;
}

void
Model::sourceModelAboutToBeDeleted()
{
    m_sourceModel = 0;
}

void
Model::setAlignment(AlignmentModel *alignment)
{
    if (m_alignment) {
        m_alignment->aboutToDelete();
        delete m_alignment;
    }
    
    m_alignment = alignment;

    if (m_alignment) {
        connect(m_alignment, SIGNAL(completionChanged()),
                this, SIGNAL(alignmentCompletionChanged()));
    }
}

const AlignmentModel *
Model::getAlignment() const
{
    return m_alignment;
}

const Model *
Model::getAlignmentReference() const
{
    if (!m_alignment) {
        if (m_sourceModel) return m_sourceModel->getAlignmentReference();
        return 0;
    }
    return m_alignment->getReferenceModel();
}

sv_frame_t
Model::alignToReference(sv_frame_t frame) const
{
    //    cerr << "Model(" << this << ")::alignToReference(" << frame << ")" << endl;
    if (!m_alignment) {
        if (m_sourceModel) return m_sourceModel->alignToReference(frame);
        else return frame;
    }
    sv_frame_t refFrame = m_alignment->toReference(frame);
    const Model *m = m_alignment->getReferenceModel();
    if (m && refFrame > m->getEndFrame()) refFrame = m->getEndFrame();
    //    cerr << "have alignment, aligned is " << refFrame << endl;
    return refFrame;
}

sv_frame_t
Model::alignFromReference(sv_frame_t refFrame) const
{
    //    cerr << "Model(" << this << ")::alignFromReference(" << refFrame << ")" << endl;
    if (!m_alignment) {
        if (m_sourceModel) return m_sourceModel->alignFromReference(refFrame);
        else return refFrame;
    }
    sv_frame_t frame = m_alignment->fromReference(refFrame);
    if (frame > getEndFrame()) frame = getEndFrame();
    //    cerr << "have alignment, aligned is " << frame << endl;
    return frame;
}

int
Model::getAlignmentCompletion() const
{
    //    cerr << "Model::getAlignmentCompletion" << endl;
    if (!m_alignment) {
        if (m_sourceModel) return m_sourceModel->getAlignmentCompletion();
        else return 100;
    }
    int completion = 0;
    (void)m_alignment->isReady(&completion);
    //    cerr << " -> " << completion << endl;
    return completion;
}

QString
Model::getTitle() const
{
    if (m_sourceModel) return m_sourceModel->getTitle();
    else return "";
}

QString
Model::getMaker() const
{
    if (m_sourceModel) return m_sourceModel->getMaker();
    else return "";
}

QString
Model::getLocation() const
{
    if (m_sourceModel) return m_sourceModel->getLocation();
    else return "";
}

void
Model::toXml(QTextStream &stream, QString indent,
             QString extraAttributes) const
{
    stream << indent;
    stream << QString("<model id=\"%1\" name=\"%2\" sampleRate=\"%3\" start=\"%4\" end=\"%5\" %6/>\n")
              .arg(getObjectExportId(this))
              .arg(encodeEntities(objectName()))
              .arg(getSampleRate())
              .arg(getStartFrame())
              .arg(getEndFrame())
              .arg(extraAttributes);
}


