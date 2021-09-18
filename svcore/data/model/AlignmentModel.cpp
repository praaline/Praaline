/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "AlignmentModel.h"

#include "SparseTimeValueModel.h"

//#define DEBUG_ALIGNMENT_MODEL 1

AlignmentModel::AlignmentModel(Model *reference,
                               Model *aligned,
                               Model *inputModel,
			       SparseTimeValueModel *path) :
    m_reference(reference),
    m_aligned(aligned),
    m_inputModel(inputModel),
    m_rawPath(path),
    m_path(0),
    m_reversePath(0),
    m_pathBegun(false),
    m_pathComplete(false)
{
    if (m_rawPath) {

        connect(m_rawPath, &Model::modelChanged,
                this, &AlignmentModel::pathChanged);

        connect(m_rawPath, &Model::modelChangedWithin,
                this, &AlignmentModel::pathChangedWithin);
        
        connect(m_rawPath, &Model::completionChanged,
                this, &AlignmentModel::pathCompletionChanged);

        constructPath();
        constructReversePath();
    }

    if (m_rawPath && m_rawPath->isReady()) {
        pathCompletionChanged();
    }
}

AlignmentModel::~AlignmentModel()
{
    if (m_inputModel) m_inputModel->aboutToDelete();
    delete m_inputModel;

    if (m_rawPath) m_rawPath->aboutToDelete();
    delete m_rawPath;

    if (m_path) m_path->aboutToDelete();
    delete m_path;

    if (m_reversePath) m_reversePath->aboutToDelete();
    delete m_reversePath;
}

bool
AlignmentModel::isOK() const
{
    if (m_rawPath) return m_rawPath->isOK();
    else return true;
}

sv_frame_t
AlignmentModel::getStartFrame() const
{
    sv_frame_t a = m_reference->getStartFrame();
    sv_frame_t b = m_aligned->getStartFrame();
    return std::min(a, b);
}

sv_frame_t
AlignmentModel::getEndFrame() const
{
    sv_frame_t a = m_reference->getEndFrame();
    sv_frame_t b = m_aligned->getEndFrame();
    return std::max(a, b);
}

sv_samplerate_t
AlignmentModel::getSampleRate() const
{
    return m_reference->getSampleRate();
}

bool
AlignmentModel::isReady(int *completion) const
{
    if (!m_pathBegun && m_rawPath) {
        if (completion) *completion = 0;
        return false;
    }
    if (m_pathComplete) {
        if (completion) *completion = 100;
        return true;
    }
    if (!m_rawPath) {
        // lack of raw path could mean path is complete (in which case
        // m_pathComplete true above) or else no alignment has been
        // set at all yet (this case)
        if (completion) *completion = 0;
        return false;
    }
    return m_rawPath->isReady(completion);
}

const ZoomConstraint *
AlignmentModel::getZoomConstraint() const
{
    return 0;
}

const Model *
AlignmentModel::getReferenceModel() const
{
    return m_reference;
}

const Model *
AlignmentModel::getAlignedModel() const
{
    return m_aligned;
}

sv_frame_t
AlignmentModel::toReference(sv_frame_t frame) const
{
#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::toReference(" << frame << ")" << endl;
#endif
    if (!m_path) {
        if (!m_rawPath) return frame;
        constructPath();
    }
    return align(m_path, frame);
}

sv_frame_t
AlignmentModel::fromReference(sv_frame_t frame) const
{
#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::fromReference(" << frame << ")" << endl;
#endif
    if (!m_reversePath) {
        if (!m_rawPath) return frame;
        constructReversePath();
    }
    return align(m_reversePath, frame);
}

void
AlignmentModel::pathChanged()
{
    if (m_pathComplete) {
        cerr << "AlignmentModel: deleting raw path model" << endl;
        if (m_rawPath) m_rawPath->aboutToDelete();
        delete m_rawPath;
        m_rawPath = 0;
    }
}

void
AlignmentModel::pathChangedWithin(sv_frame_t, sv_frame_t)
{
    if (!m_pathComplete) return;
    constructPath();
    constructReversePath();
}    

void
AlignmentModel::pathCompletionChanged()
{
    if (!m_rawPath) return;
    m_pathBegun = true;

    if (!m_pathComplete) {

        int completion = 0;
        m_rawPath->isReady(&completion);

#ifdef DEBUG_ALIGNMENT_MODEL
        cerr << "AlignmentModel::pathCompletionChanged: completion = "
                  << completion << endl;
#endif

        m_pathComplete = (completion == 100);

        if (m_pathComplete) {

            constructPath();
            constructReversePath();
            
            if (m_inputModel) m_inputModel->aboutToDelete();
            delete m_inputModel;
            m_inputModel = 0;
        }
    }

    emit completionChanged();
}

void
AlignmentModel::constructPath() const
{
    if (!m_path) {
        if (!m_rawPath) {
            cerr << "ERROR: AlignmentModel::constructPath: "
                      << "No raw path available" << endl;
            return;
        }
        m_path = new PathModel
            (m_rawPath->getSampleRate(), m_rawPath->getResolution(), false);
    } else {
        if (!m_rawPath) return;
    }
        
    m_path->clear();

    SparseTimeValueModel::PointList points = m_rawPath->getPoints();
        
    for (SparseTimeValueModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        sv_frame_t frame = i->frame;
        double value = i->value;
        sv_frame_t rframe = lrint(value * m_aligned->getSampleRate());
        m_path->addPoint(PathPoint(frame, rframe));
    }

#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::constructPath: " << m_path->getPointCount() << " points, at least " << (2 * m_path->getPointCount() * (3 * sizeof(void *) + sizeof(int) + sizeof(PathPoint))) << " bytes" << endl;
#endif
}

void
AlignmentModel::constructReversePath() const
{
    if (!m_reversePath) {
        if (!m_path) {
            cerr << "ERROR: AlignmentModel::constructReversePath: "
                      << "No forward path available" << endl;
            return;
        }
        m_reversePath = new PathModel
            (m_path->getSampleRate(), m_path->getResolution(), false);
    } else {
        if (!m_path) return;
    }
        
    m_reversePath->clear();

    PathModel::PointList points = m_path->getPoints();
        
    for (PathModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        sv_frame_t frame = i->frame;
        sv_frame_t rframe = i->mapframe;
        m_reversePath->addPoint(PathPoint(rframe, frame));
    }

#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::constructReversePath: " << m_reversePath->getPointCount() << " points, at least " << (2 * m_reversePath->getPointCount() * (3 * sizeof(void *) + sizeof(int) + sizeof(PathPoint))) << " bytes" << endl;
#endif
}

sv_frame_t
AlignmentModel::align(PathModel *path, sv_frame_t frame) const
{
    if (!path) return frame;

    // The path consists of a series of points, each with frame equal
    // to the frame on the source model and mapframe equal to the
    // frame on the target model.  Both should be monotonically
    // increasing.

    const PathModel::PointList &points = path->getPoints();

    if (points.empty()) {
#ifdef DEBUG_ALIGNMENT_MODEL
        cerr << "AlignmentModel::align: No points" << endl;
#endif
        return frame;
    }        

#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::align: frame " << frame << " requested" << endl;
#endif

    PathModel::Point point(frame);
    PathModel::PointList::const_iterator i = points.lower_bound(point);
    if (i == points.end()) {
#ifdef DEBUG_ALIGNMENT_MODEL
        cerr << "Note: i == points.end()" << endl;
#endif
        --i;
    }
    while (i != points.begin() && i->frame > frame) --i;

    sv_frame_t foundFrame = i->frame;
    sv_frame_t foundMapFrame = i->mapframe;

    sv_frame_t followingFrame = foundFrame;
    sv_frame_t followingMapFrame = foundMapFrame;

    if (++i != points.end()) {
#ifdef DEBUG_ALIGNMENT_MODEL
        cerr << "another point available" << endl;
#endif
        followingFrame = i->frame;
        followingMapFrame = i->mapframe;
    } else {
#ifdef DEBUG_ALIGNMENT_MODEL
        cerr << "no other point available" << endl;
#endif
    }        

    if (foundMapFrame < 0) return 0;

    sv_frame_t resultFrame = foundMapFrame;

    if (followingFrame != foundFrame && frame > foundFrame) {
        double interp =
            double(frame - foundFrame) /
            double(followingFrame - foundFrame);
        resultFrame += lrint(double(followingMapFrame - foundMapFrame) * interp);
    }

#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::align: resultFrame = " << resultFrame << endl;
#endif

    return resultFrame;
}

void
AlignmentModel::setPathFrom(SparseTimeValueModel *rawpath)
{
    if (m_rawPath) m_rawPath->aboutToDelete();
    delete m_rawPath;

    m_rawPath = rawpath;

    connect(m_rawPath, &Model::modelChanged,
            this, &AlignmentModel::pathChanged);

    connect(m_rawPath, &Model::modelChangedWithin,
            this, &AlignmentModel::pathChangedWithin);
        
    connect(m_rawPath, &Model::completionChanged,
            this, &AlignmentModel::pathCompletionChanged);
    
    constructPath();
    constructReversePath();

    if (m_rawPath->isReady()) {
        pathCompletionChanged();
    }        
}

void
AlignmentModel::setPath(PathModel *path)
{
    if (m_path) m_path->aboutToDelete();
    delete m_path;
    m_path = path;
#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::setPath: path = " << m_path << endl;
#endif
    constructReversePath();
#ifdef DEBUG_ALIGNMENT_MODEL
    cerr << "AlignmentModel::setPath: after construction path = "
              << m_path << ", rpath = " << m_reversePath << endl;
#endif
}
    
void
AlignmentModel::toXml(QTextStream &stream,
                      QString indent,
                      QString extraAttributes) const
{
    if (!m_path) {
        cerr << "AlignmentModel::toXml: no path" << endl;
        return;
    }

    m_path->toXml(stream, indent, "");

    Model::toXml(stream, indent,
                 QString("type=\"alignment\" reference=\"%1\" aligned=\"%2\" path=\"%3\" %4")
                 .arg(getObjectExportId(m_reference))
                 .arg(getObjectExportId(m_aligned))
                 .arg(getObjectExportId(m_path))
                 .arg(extraAttributes));
}


