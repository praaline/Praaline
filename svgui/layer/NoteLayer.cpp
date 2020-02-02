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

#include "NoteLayer.h"

#include "data/model/Model.h"
#include "PraalineCore/Base/RealTime.h"
#include "base/Profiler.h"
#include "base/Pitch.h"
#include "base/LogRange.h"
#include "base/RangeMapper.h"
#include "ColourDatabase.h"
#include "view/View.h"

#include "PianoScale.h"
#include "LinearNumericalScale.h"
#include "LogNumericalScale.h"
#include "LayerFactory.h"

#include "data/model/NoteModel.h"

#include "widgets/ItemEditDialog.h"
#include "widgets/TextAbbrev.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>
#include <utility>

//#define DEBUG_NOTE_LAYER 1

NoteLayer::NoteLayer() :
    SingleColourLayer(),
    m_model(0),
    m_editing(false),
    m_dragPointX(0),
    m_dragPointY(0),
    m_dragStartX(0),
    m_dragStartY(0),
    m_originalPoint(0, 0.0, 0, 1.f, tr("New Point")),
    m_editingPoint(0, 0.0, 0, 1.f, tr("New Point")),
    m_editingCommand(0),
    m_verticalScale(AutoAlignScale),
    m_scaleMinimum(0),
    m_scaleMaximum(0)
{
    cerr << "constructed NoteLayer" << endl;
}

bool NoteLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<NoteLayer, NoteModel>(this, model))
        return true;
    return false;
}

void
NoteLayer::setModel(NoteModel *model)
{	
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    //    cerr << "NoteLayer::setModel(" << model << ")" << endl;

    m_scaleMinimum = 0;
    m_scaleMaximum = 0;

    emit modelReplaced();
}

// static
Model *NoteLayer::createEmptyModel(Model *baseModel)
{
    return new NoteModel(baseModel->getSampleRate(), 1, true);
}

Layer::PropertyList
NoteLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Vertical Scale");
    list.push_back("Scale Units");
    return list;
}

QString
NoteLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Vertical Scale") return tr("Vertical Scale");
    if (name == "Scale Units") return tr("Scale Units");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
NoteLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Scale Units") return UnitsProperty;
    if (name == "Vertical Scale") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

QString
NoteLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Vertical Scale" || name == "Scale Units") {
        return tr("Scale");
    }
    return SingleColourLayer::getPropertyGroupName(name);
}

QString
NoteLayer::getScaleUnits() const
{
    if (m_model) return m_model->getScaleUnits();
    else return "";
}

int
NoteLayer::getPropertyRangeAndValue(const PropertyName &name,
                                    int *min, int *max, int *deflt) const
{
    int val = 0;

    if (name == "Vertical Scale") {

        if (min) *min = 0;
        if (max) *max = 3;
        if (deflt) *deflt = int(AutoAlignScale);

        val = int(m_verticalScale);

    } else if (name == "Scale Units") {

        if (deflt) *deflt = 0;
        if (m_model) {
            val = UnitDatabase::getInstance()->getUnitId
                    (getScaleUnits());
        }

    } else {

        val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
NoteLayer::getPropertyValueLabel(const PropertyName &name,
                                 int value) const
{
    if (name == "Vertical Scale") {
        switch (value) {
        default:
        case 0: return tr("Auto-Align");
        case 1: return tr("Linear");
        case 2: return tr("Log");
        case 3: return tr("MIDI Notes");
        }
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void
NoteLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Vertical Scale") {
        setVerticalScale(VerticalScale(value));
    } else if (name == "Scale Units") {
        if (m_model) {
            m_model->setScaleUnits
                    (UnitDatabase::getInstance()->getUnitById(value));
            emit modelChanged();
        }
    } else {
        return SingleColourLayer::setProperty(name, value);
    }
}

void
NoteLayer::setVerticalScale(VerticalScale scale)
{
    if (m_verticalScale == scale) return;
    m_verticalScale = scale;
    emit layerParametersChanged();
}

bool
NoteLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

bool
NoteLayer::shouldConvertMIDIToHz() const
{
    QString unit = getScaleUnits();
    return (unit != "Hz");
    //    if (unit == "" ||
    //        unit.startsWith("MIDI") ||
    //        unit.startsWith("midi")) return true;
    //    return false;
}

bool
NoteLayer::getValueExtents(double &min, double &max,
                           bool &logarithmic, QString &unit) const
{
    if (!m_model) return false;
    min = m_model->getValueMinimum();
    max = m_model->getValueMaximum();

    if (shouldConvertMIDIToHz()) {
        unit = "Hz";
        min = Pitch::getFrequencyForPitch(int(lrint(min)));
        max = Pitch::getFrequencyForPitch(int(lrint(max + 1)));
    } else unit = getScaleUnits();

    if (m_verticalScale == MIDIRangeScale ||
            m_verticalScale == LogScale) logarithmic = true;

    return true;
}

bool
NoteLayer::getDisplayExtents(double &min, double &max) const
{
    if (!m_model || shouldAutoAlign()) return false;

    if (m_verticalScale == MIDIRangeScale) {
        min = Pitch::getFrequencyForPitch(0);
        max = Pitch::getFrequencyForPitch(127);
        return true;
    }

    if (m_scaleMinimum == m_scaleMaximum) {
        min = m_model->getValueMinimum();
        max = m_model->getValueMaximum();
    } else {
        min = m_scaleMinimum;
        max = m_scaleMaximum;
    }

    if (shouldConvertMIDIToHz()) {
        min = Pitch::getFrequencyForPitch(int(lrint(min)));
        max = Pitch::getFrequencyForPitch(int(lrint(max + 1)));
    }

#ifdef DEBUG_NOTE_LAYER
    cerr << "NoteLayer::getDisplayExtents: min = " << min << ", max = " << max << " (m_scaleMinimum = " << m_scaleMinimum << ", m_scaleMaximum = " << m_scaleMaximum << ")" << endl;
#endif

    return true;
}

bool
NoteLayer::setDisplayExtents(double min, double max)
{
    if (!m_model) return false;

    if (min == max) {
        if (min == 0.f) {
            max = 1.f;
        } else {
            max = min * 1.0001;
        }
    }

    m_scaleMinimum = min;
    m_scaleMaximum = max;

#ifdef DEBUG_NOTE_LAYER
    cerr << "NoteLayer::setDisplayExtents: min = " << min << ", max = " << max << endl;
#endif
    
    emit layerParametersChanged();
    return true;
}

int
NoteLayer::getVerticalZoomSteps(int &defaultStep) const
{
    if (shouldAutoAlign()) return 0;
    if (!m_model) return 0;

    defaultStep = 0;
    return 100;
}

int
NoteLayer::getCurrentVerticalZoomStep() const
{
    if (shouldAutoAlign()) return 0;
    if (!m_model) return 0;

    RangeMapper *mapper = getNewVerticalZoomRangeMapper();
    if (!mapper) return 0;

    double dmin, dmax;
    getDisplayExtents(dmin, dmax);

    int nr = mapper->getPositionForValue(dmax - dmin);

    delete mapper;

    return 100 - nr;
}

//!!! lots of duplication with TimeValueLayer

void
NoteLayer::setVerticalZoomStep(int step)
{
    if (shouldAutoAlign()) return;
    if (!m_model) return;

    RangeMapper *mapper = getNewVerticalZoomRangeMapper();
    if (!mapper) return;
    
    double min, max;
    bool logarithmic;
    QString unit;
    getValueExtents(min, max, logarithmic, unit);
    
    double dmin, dmax;
    getDisplayExtents(dmin, dmax);

    double newdist = mapper->getValueForPosition(100 - step);

    double newmin, newmax;

    if (logarithmic) {

        // see SpectrogramLayer::setVerticalZoomStep

        newmax = (newdist + sqrt(newdist*newdist + 4*dmin*dmax)) / 2;
        newmin = newmax - newdist;

        //        cerr << "newmin = " << newmin << ", newmax = " << newmax << endl;

    } else {
        double dmid = (dmax + dmin) / 2;
        newmin = dmid - newdist / 2;
        newmax = dmid + newdist / 2;
    }

    if (newmin < min) {
        newmax += (min - newmin);
        newmin = min;
    }
    if (newmax > max) {
        newmax = max;
    }
    
#ifdef DEBUG_NOTE_LAYER
    cerr << "NoteLayer::setVerticalZoomStep: " << step << ": " << newmin << " -> " << newmax << " (range " << newdist << ")" << endl;
#endif

    setDisplayExtents(newmin, newmax);
}

RangeMapper *
NoteLayer::getNewVerticalZoomRangeMapper() const
{
    if (!m_model) return 0;
    
    RangeMapper *mapper;

    double min, max;
    bool logarithmic;
    QString unit;
    getValueExtents(min, max, logarithmic, unit);

    if (min == max) return 0;
    
    if (logarithmic) {
        mapper = new LogRangeMapper(0, 100, min, max, unit);
    } else {
        mapper = new LinearRangeMapper(0, 100, min, max, unit);
    }

    return mapper;
}

NoteModel::PointList
NoteLayer::getLocalPoints(View *v, int x) const
{
    if (!m_model) return NoteModel::PointList();

    sv_frame_t frame = v->getFrameForX(x);

    NoteModel::PointList onPoints =
            m_model->getPoints(frame);

    if (!onPoints.empty()) {
        return onPoints;
    }

    NoteModel::PointList prevPoints =
            m_model->getPreviousPoints(frame);
    NoteModel::PointList nextPoints =
            m_model->getNextPoints(frame);

    NoteModel::PointList usePoints = prevPoints;

    if (prevPoints.empty()) {
        usePoints = nextPoints;
    } else if (int(prevPoints.begin()->frame) < v->getStartFrame() &&
               !(nextPoints.begin()->frame > v->getEndFrame())) {
        usePoints = nextPoints;
    } else if (int(nextPoints.begin()->frame) - frame <
               frame - int(prevPoints.begin()->frame)) {
        usePoints = nextPoints;
    }

    if (!usePoints.empty()) {
        int fuzz = 2;
        int px = v->getXForFrame(usePoints.begin()->frame);
        if ((px > x && px - x > fuzz) ||
                (px < x && x - px > fuzz + 1)) {
            usePoints.clear();
        }
    }

    return usePoints;
}

bool
NoteLayer::getPointToDrag(View *v, int x, int y, NoteModel::Point &p) const
{
    if (!m_model) return false;

    sv_frame_t frame = v->getFrameForX(x);

    NoteModel::PointList onPoints = m_model->getPoints(frame);
    if (onPoints.empty()) return false;

    //    cerr << "frame " << frame << ": " << onPoints.size() << " candidate points" << endl;

    int nearestDistance = -1;

    for (NoteModel::PointList::const_iterator i = onPoints.begin();
         i != onPoints.end(); ++i) {
        
        int distance = getYForValue(v, (*i).value) - y;
        if (distance < 0) distance = -distance;
        if (nearestDistance == -1 || distance < nearestDistance) {
            nearestDistance = distance;
            p = *i;
        }
    }

    return true;
}

QString
NoteLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    NoteModel::PointList points = getLocalPoints(v, x);

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return tr("No local points");
        }
    }

    Note note(0);
    NoteModel::PointList::iterator i;

    for (i = points.begin(); i != points.end(); ++i) {

        int y = getYForValue(v, i->value);
        int h = 3;

        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, i->value + m_model->getValueQuantization());
            if (h < 3) h = 3;
        }

        if (pos.y() >= y - h && pos.y() <= y) {
            note = *i;
            break;
        }
    }

    if (i == points.end()) return tr("No local points");

    RealTime rt = RealTime::frame2RealTime(note.frame,
                                           m_model->getSampleRate());
    RealTime rd = RealTime::frame2RealTime(note.duration,
                                           m_model->getSampleRate());
    
    QString pitchText;

    if (shouldConvertMIDIToHz()) {

        int mnote = int(lrint(note.value));
        int cents = int(lrint((note.value - float(mnote)) * 100));
        double freq = Pitch::getFrequencyForPitch(mnote, cents);
        pitchText = tr("%1 (%2, %3 Hz)")
                .arg(Pitch::getPitchLabel(mnote, cents))
                .arg(mnote)
                .arg(freq);

    } else if (getScaleUnits() == "Hz") {

        pitchText = tr("%1 Hz (%2, %3)")
                .arg(note.value)
                .arg(Pitch::getPitchLabelForFrequency(note.value))
                .arg(Pitch::getPitchForFrequency(note.value));

    } else {
        pitchText = tr("%1 %2")
                .arg(note.value).arg(getScaleUnits());
    }

    QString text;

    if (note.label == "") {
        text = QString(tr("Time:\t%1\nPitch:\t%2\nDuration:\t%3\nNo label"))
                .arg(rt.toText(true).c_str())
                .arg(pitchText)
                .arg(rd.toText(true).c_str());
    } else {
        text = QString(tr("Time:\t%1\nPitch:\t%2\nDuration:\t%3\nLabel:\t%4"))
                .arg(rt.toText(true).c_str())
                .arg(pitchText)
                .arg(rd.toText(true).c_str())
                .arg(note.label);
    }

    pos = QPoint(v->getXForFrame(note.frame),
                 getYForValue(v, note.value));
    return text;
}

bool
NoteLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    NoteModel::PointList points;

    if (snap == SnapNeighbouring) {

        points = getLocalPoints(v, v->getXForFrame(frame));
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (NoteModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        if (snap == SnapRight) {

            if (i->frame > frame) {
                snapped = i->frame;
                found = true;
                break;
            }

        } else if (snap == SnapLeft) {

            if (i->frame <= frame) {
                snapped = i->frame;
                found = true; // don't break, as the next may be better
            } else {
                break;
            }

        } else { // nearest

            NoteModel::PointList::const_iterator j = i;
            ++j;

            if (j == points.end()) {

                snapped = i->frame;
                found = true;
                break;

            } else if (j->frame >= frame) {

                if (j->frame - frame < frame - i->frame) {
                    snapped = j->frame;
                } else {
                    snapped = i->frame;
                }
                found = true;
                break;
            }
        }
    }

    frame = snapped;
    return found;
}

void
NoteLayer::getScaleExtents(View *v, double &min, double &max, bool &log) const
{
    min = 0.0;
    max = 0.0;
    log = false;

    QString queryUnits;
    if (shouldConvertMIDIToHz()) queryUnits = "Hz";
    else queryUnits = getScaleUnits();

    if (shouldAutoAlign()) {

        if (!v->getValueExtents(queryUnits, min, max, log)) {

            min = m_model->getValueMinimum();
            max = m_model->getValueMaximum();

            if (shouldConvertMIDIToHz()) {
                min = Pitch::getFrequencyForPitch(int(lrint(min)));
                max = Pitch::getFrequencyForPitch(int(lrint(max + 1)));
            }

#ifdef DEBUG_NOTE_LAYER
            cerr << "NoteLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;
#endif

        } else if (log) {

            LogRange::mapRange(min, max);

#ifdef DEBUG_NOTE_LAYER
            cerr << "NoteLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;
#endif

        }

    } else {

        getDisplayExtents(min, max);

        if (m_verticalScale == MIDIRangeScale) {
            min = Pitch::getFrequencyForPitch(0);
            max = Pitch::getFrequencyForPitch(127);
        } else if (shouldConvertMIDIToHz()) {
            min = Pitch::getFrequencyForPitch(int(lrint(min)));
            max = Pitch::getFrequencyForPitch(int(lrint(max + 1)));
        }

        if (m_verticalScale == LogScale || m_verticalScale == MIDIRangeScale) {
            LogRange::mapRange(min, max);
            log = true;
        }
    }

    if (max == min) max = min + 1.0;
}

int
NoteLayer::getYForValue(View *v, double val) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    getScaleExtents(v, min, max, logarithmic);

#ifdef DEBUG_NOTE_LAYER
    cerr << "NoteLayer[" << this << "]::getYForValue(" << val << "): min = " << min << ", max = " << max << ", log = " << logarithmic << endl;
#endif

    if (shouldConvertMIDIToHz()) {
        val = Pitch::getFrequencyForPitch(int(lrint(val)),
                                          int(lrint((val - rint(val)) * 100)));
#ifdef DEBUG_NOTE_LAYER
        cerr << "shouldConvertMIDIToHz true, val now = " << val << endl;
#endif
    }

    if (logarithmic) {
        val = LogRange::map(val);
#ifdef DEBUG_NOTE_LAYER
        cerr << "logarithmic true, val now = " << val << endl;
#endif
    }

    int y = int(h - ((val - min) * h) / (max - min)) - 1;
#ifdef DEBUG_NOTE_LAYER
    cerr << "y = " << y << endl;
#endif
    return y;
}

double
NoteLayer::getValueForY(View *v, int y) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    getScaleExtents(v, min, max, logarithmic);

    double val = min + (double(h - y) * double(max - min)) / h;

    if (logarithmic) {
        val = pow(10.0, val);
    }

    if (shouldConvertMIDIToHz()) {
        val = Pitch::getPitchForFrequency(val);
    }

    return val;
}

bool
NoteLayer::shouldAutoAlign() const
{
    if (!m_model) return false;
    return (m_verticalScale == AutoAlignScale);
}

void
NoteLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    //    Profiler profiler("NoteLayer::paint", true);

    int x0 = rect.left(), x1 = rect.right();
    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    NoteModel::PointList points(m_model->getPoints(frame0, frame1));
    if (points.empty()) return;

    paint.setPen(getBaseQColor());

    QColor brushColour(getBaseQColor());
    brushColour.setAlpha(80);

    //    cerr << "NoteLayer::paint: resolution is "
    //	      << m_model->getResolution() << " frames" << endl;

    double min = m_model->getValueMinimum();
    double max = m_model->getValueMaximum();
    if (max == min) max = min + 1.0;

    QPoint localPos;
    NoteModel::Point illuminatePoint(0);
    bool shouldIlluminate = false;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        shouldIlluminate = getPointToDrag(v, localPos.x(), localPos.y(),
                                          illuminatePoint);
    }

    paint.save();
    paint.setRenderHint(QPainter::Antialiasing, false);
    
    for (NoteModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        const NoteModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);
        int y = getYForValue(v, p.value);
        int w = v->getXForFrame(p.frame + p.duration) - x;
        int h = 3;

        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, p.value + m_model->getValueQuantization());
            if (h < 3) h = 3;
        }

        if (w < 1) w = 1;
        paint.setPen(getBaseQColor());
        paint.setBrush(brushColour);

        if (shouldIlluminate &&
                // "illuminatePoint == p"
                !NoteModel::Point::Comparator()(illuminatePoint, p) &&
                !NoteModel::Point::Comparator()(p, illuminatePoint)) {

            paint.setPen(v->getForeground());
            paint.setBrush(v->getForeground());

            QString vlabel = QString("%1%2").arg(p.value).arg(getScaleUnits());
            v->drawVisibleText(paint,
                               x - paint.fontMetrics().width(vlabel) - 2,
                               y + paint.fontMetrics().height()/2
                               - paint.fontMetrics().descent(),
                               vlabel, View::OutlinedText);

            QString hlabel = RealTime::frame2RealTime
                    (p.frame, m_model->getSampleRate()).toText(true).c_str();
            v->drawVisibleText(paint,
                               x,
                               y - h/2 - paint.fontMetrics().descent() - 2,
                               hlabel, View::OutlinedText);
        }

        paint.drawRect(x, y - h/2, w, h);
    }

    paint.restore();
}

int
NoteLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
{
    if (!m_model || shouldAutoAlign()) {
        return 0;
    } else  {
        if (m_verticalScale == LogScale || m_verticalScale == MIDIRangeScale) {
            return LogNumericalScale().getWidth(v, paint) + 10; // for piano
        } else {
            return LinearNumericalScale().getWidth(v, paint);
        }
    }
}

void
NoteLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect) const
{
    if (!m_model || m_model->getPoints().empty()) return;

    QString unit;
    double min, max;
    bool logarithmic;

    int w = getVerticalScaleWidth(v, false, paint);
    int h = v->height();

    getScaleExtents(v, min, max, logarithmic);

    if (logarithmic) {
        LogNumericalScale().paintVertical(v, this, paint, 0, min, max);
    } else {
        LinearNumericalScale().paintVertical(v, this, paint, 0, min, max);
    }
    
    if (logarithmic && (getScaleUnits() == "Hz")) {
        PianoScale().paintPianoVertical
                (v, paint, QRect(w - 10, 0, 10, h),
                 LogRange::unmap(min),
                 LogRange::unmap(max));
        paint.drawLine(w, 0, w, h);
    }

    if (getScaleUnits() != "") {
        int mw = w - 5;
        paint.drawText(5,
                       5 + paint.fontMetrics().ascent(),
                       TextAbbrev::abbreviate(getScaleUnits(),
                                              paint.fontMetrics(),
                                              mw));
    }
}

void
NoteLayer::drawStart(View *v, QMouseEvent *e)
{
    //    cerr << "NoteLayer::drawStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double value = getValueForY(v, e->y());

    m_editingPoint = NoteModel::Point(frame, float(value), 0, 0.8f, tr("New Point"));
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new NoteModel::EditCommand(m_model,
                                                  tr("Draw Point"));
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
NoteLayer::drawDrag(View *v, QMouseEvent *e)
{
    //    cerr << "NoteLayer::drawDrag(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model || !m_editing) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double newValue = getValueForY(v, e->y());

    sv_frame_t newFrame = m_editingPoint.frame;
    sv_frame_t newDuration = frame - newFrame;
    if (newDuration < 0) {
        newFrame = frame;
        newDuration = -newDuration;
    } else if (newDuration == 0) {
        newDuration = 1;
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = newFrame;
    m_editingPoint.value = float(newValue);
    m_editingPoint.duration = newDuration;
    m_editingCommand->addPoint(m_editingPoint);
}

void
NoteLayer::drawEnd(View *, QMouseEvent *)
{
    //    cerr << "NoteLayer::drawEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;
    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
NoteLayer::eraseStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
}

void
NoteLayer::eraseDrag(View *, QMouseEvent *)
{
}

void
NoteLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    NoteModel::Point p(0);
    if (!getPointToDrag(v, e->x(), e->y(), p)) return;
    if (p.frame != m_editingPoint.frame || p.value != m_editingPoint.value) return;

    m_editingCommand = new NoteModel::EditCommand(m_model, tr("Erase Point"));

    m_editingCommand->deletePoint(m_editingPoint);

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
NoteLayer::editStart(View *v, QMouseEvent *e)
{
    //    cerr << "NoteLayer::editStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;
    m_originalPoint = m_editingPoint;

    m_dragPointX = v->getXForFrame(m_editingPoint.frame);
    m_dragPointY = getYForValue(v, m_editingPoint.value);

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
    m_dragStartX = e->x();
    m_dragStartY = e->y();
}

void
NoteLayer::editDrag(View *v, QMouseEvent *e)
{
    //    cerr << "NoteLayer::editDrag(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model || !m_editing) return;

    int xdist = e->x() - m_dragStartX;
    int ydist = e->y() - m_dragStartY;
    int newx = m_dragPointX + xdist;
    int newy = m_dragPointY + ydist;

    sv_frame_t frame = v->getFrameForX(newx);
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double value = getValueForY(v, newy);

    if (!m_editingCommand) {
        m_editingCommand = new NoteModel::EditCommand(m_model,
                                                      tr("Drag Point"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingPoint.value = float(value);
    m_editingCommand->addPoint(m_editingPoint);
}

void
NoteLayer::editEnd(View *, QMouseEvent *)
{
    //    cerr << "NoteLayer::editEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;

    if (m_editingCommand) {

        QString newName = m_editingCommand->getName();

        if (m_editingPoint.frame != m_originalPoint.frame) {
            if (m_editingPoint.value != m_originalPoint.value) {
                newName = tr("Edit Point");
            } else {
                newName = tr("Relocate Point");
            }
        } else {
            newName = tr("Change Point Value");
        }

        m_editingCommand->setName(newName);
        finish(m_editingCommand);
    }

    m_editingCommand = 0;
    m_editing = false;
}

bool
NoteLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    NoteModel::Point note(0);
    if (!getPointToDrag(v, e->x(), e->y(), note)) return false;

    //    NoteModel::Point note = *points.begin();

    ItemEditDialog *dialog = new ItemEditDialog
            (m_model->getSampleRate(),
             ItemEditDialog::ShowTime |
             ItemEditDialog::ShowDuration |
             ItemEditDialog::ShowValue |
             ItemEditDialog::ShowText,
             getScaleUnits());

    dialog->setFrameTime(note.frame);
    dialog->setValue(note.value);
    dialog->setFrameDuration(note.duration);
    dialog->setText(note.label);

    if (dialog->exec() == QDialog::Accepted) {

        NoteModel::Point newNote = note;
        newNote.frame = dialog->getFrameTime();
        newNote.value = dialog->getValue();
        newNote.duration = dialog->getFrameDuration();
        newNote.label = dialog->getText();
        
        NoteModel::EditCommand *command = new NoteModel::EditCommand
                (m_model, tr("Edit Point"));
        command->deletePoint(note);
        command->addPoint(newNote);
        finish(command);
    }

    delete dialog;
    return true;
}

void
NoteLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    NoteModel::EditCommand *command =
            new NoteModel::EditCommand(m_model, tr("Drag Selection"));

    NoteModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (NoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            NoteModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
NoteLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    NoteModel::EditCommand *command =
            new NoteModel::EditCommand(m_model, tr("Resize Selection"));

    NoteModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
            double(newSize.getEndFrame() - newSize.getStartFrame()) /
            double(s.getEndFrame() - s.getStartFrame());

    for (NoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double targetStart = double(i->frame);
            targetStart = double(newSize.getStartFrame()) +
                    targetStart - double(s.getStartFrame()) * ratio;

            double targetEnd = double(i->frame + i->duration);
            targetEnd = double(newSize.getStartFrame()) +
                    targetEnd - double(s.getStartFrame()) * ratio;

            NoteModel::Point newPoint(*i);
            newPoint.frame = lrint(targetStart);
            newPoint.duration = lrint(targetEnd - targetStart);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
NoteLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    NoteModel::EditCommand *command =
            new NoteModel::EditCommand(m_model, tr("Delete Selected Points"));

    NoteModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (NoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            command->deletePoint(*i);
        }
    }

    finish(command);
}    

void
NoteLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    NoteModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (NoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->value, i->duration, i->level, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
NoteLayer::paste(View *v, const Clipboard &from, sv_frame_t /* frameOffset */, bool /* interactive */)
{
    if (!m_model) return false;

    const Clipboard::PointList &points = from.getPoints();

    bool realign = false;

    if (clipboardHasDifferentAlignment(v, from)) {

        QMessageBox::StandardButton button =
                QMessageBox::question(v, tr("Re-align pasted items?"),
                                      tr("The items you are pasting came from a layer with different source material from this one.  Do you want to re-align them in time, to match the source material for this layer?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::Yes);

        if (button == QMessageBox::Cancel) {
            return false;
        }

        if (button == QMessageBox::Yes) {
            realign = true;
        }
    }

    NoteModel::EditCommand *command =
            new NoteModel::EditCommand(m_model, tr("Paste"));

    for (Clipboard::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        
        if (!i->haveFrame()) continue;
        sv_frame_t frame = 0;

        if (!realign) {
            
            frame = i->getFrame();

        } else {

            if (i->haveReferenceFrame()) {
                frame = i->getReferenceFrame();
                frame = alignFromReference(v, frame);
            } else {
                frame = i->getFrame();
            }
        }

        NoteModel::Point newPoint(frame);

        if (i->haveLabel()) newPoint.label = i->getLabel();
        if (i->haveValue()) newPoint.value = i->getValue();
        else newPoint.value = (m_model->getValueMinimum() +
                               m_model->getValueMaximum()) / 2;
        if (i->haveLevel()) newPoint.level = i->getLevel();
        if (i->haveDuration()) newPoint.duration = i->getDuration();
        else {
            sv_frame_t nextFrame = frame;
            Clipboard::PointList::const_iterator j = i;
            for (; j != points.end(); ++j) {
                if (!j->haveFrame()) continue;
                if (j != i) break;
            }
            if (j != points.end()) {
                nextFrame = j->getFrame();
            }
            if (nextFrame == frame) {
                newPoint.duration = m_model->getResolution();
            } else {
                newPoint.duration = nextFrame - frame;
            }
        }
        
        command->addPoint(newPoint);
    }

    finish(command);
    return true;
}

void
NoteLayer::addNoteOn(sv_frame_t frame, int pitch, int velocity)
{
    m_pendingNoteOns.insert(Note(frame, float(pitch), 0, float(velocity) / 127.f, ""));
}

void
NoteLayer::addNoteOff(sv_frame_t frame, int pitch)
{
    for (NoteSet::iterator i = m_pendingNoteOns.begin();
         i != m_pendingNoteOns.end(); ++i) {
        if (lrintf((*i).value) == pitch) {
            Note note(*i);
            m_pendingNoteOns.erase(i);
            note.duration = frame - note.frame;
            if (m_model) {
                NoteModel::AddPointCommand *c = new NoteModel::AddPointCommand
                        (m_model, note, tr("Record Note"));
                // execute and bundle:
                CommandHistory::getInstance()->addCommand(c, true, true);
            }
            break;
        }
    }
}

void
NoteLayer::abandonNoteOns()
{
    m_pendingNoteOns.clear();
}

int
NoteLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "White" : "Black"));
}

void
NoteLayer::toXml(QTextStream &stream,
                 QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes +
                             QString(" verticalScale=\"%1\" scaleMinimum=\"%2\" scaleMaximum=\"%3\" ")
                             .arg(m_verticalScale)
                             .arg(m_scaleMinimum)
                             .arg(m_scaleMaximum));
}

void
NoteLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);

    bool ok, alsoOk;
    VerticalScale scale = (VerticalScale)
            attributes.value("verticalScale").toInt(&ok);
    if (ok) setVerticalScale(scale);

    float min = attributes.value("scaleMinimum").toFloat(&ok);
    float max = attributes.value("scaleMaximum").toFloat(&alsoOk);
    if (ok && alsoOk && min != max) setDisplayExtents(min, max);
}


