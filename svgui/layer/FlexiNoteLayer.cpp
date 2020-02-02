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

#include "FlexiNoteLayer.h"

#include "data/model/Model.h"
#include "data/model/SparseTimeValueModel.h"
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

#include "data/model/FlexiNoteModel.h"

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
#include <limits> // GF: included to compile std::numerical_limits on linux
#include <vector>

FlexiNoteLayer::FlexiNoteLayer() :
    SingleColourLayer(),

    // m_model(0),
    // m_editing(false),
    // m_originalPoint(0, 0.0, 0, 1.f, tr("New Point")),
    // m_editingPoint(0, 0.0, 0, 1.f, tr("New Point")),
    // m_editingCommand(0),
    // m_verticalScale(AutoAlignScale),
    // m_scaleMinimum(0),
    // m_scaleMaximum(0)

    m_model(0),
    m_editing(false),
    m_intelligentActions(true),
    m_dragPointX(0),
    m_dragPointY(0),
    m_dragStartX(0),
    m_dragStartY(0),
    m_originalPoint(0, 0.0, 0, 1.f, tr("New Point")),
    m_editingPoint(0, 0.0, 0, 1.f, tr("New Point")),
    m_greatestLeftNeighbourFrame(0),
    m_smallestRightNeighbourFrame(0),
    m_editingCommand(0),
    m_verticalScale(AutoAlignScale),
    m_editMode(DragNote),
    m_scaleMinimum(34), 
    m_scaleMaximum(77)
{
}

bool FlexiNoteLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<FlexiNoteLayer, FlexiNoteModel>(this, model))
        return true;
    return false;
}

void
FlexiNoteLayer::setModel(FlexiNoteModel *model) 
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    // m_scaleMinimum = 0;
    // m_scaleMaximum = 0;

    emit modelReplaced();
}

// static
Model *FlexiNoteLayer::createEmptyModel(Model *baseModel)
{
    return new FlexiNoteModel(baseModel->getSampleRate(), 1, true);
}

Layer::PropertyList
FlexiNoteLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Vertical Scale");
    list.push_back("Scale Units");
    return list;
}

QString
FlexiNoteLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Vertical Scale") return tr("Vertical Scale");
    if (name == "Scale Units") return tr("Scale Units");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
FlexiNoteLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Scale Units") return UnitsProperty;
    if (name == "Vertical Scale") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

QString
FlexiNoteLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Vertical Scale" || name == "Scale Units") {
        return tr("Scale");
    }
    return SingleColourLayer::getPropertyGroupName(name);
}

QString
FlexiNoteLayer::getScaleUnits() const
{
    if (m_model) return m_model->getScaleUnits();
    else return "";
}

int
FlexiNoteLayer::getPropertyRangeAndValue(const PropertyName &name,
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
FlexiNoteLayer::getPropertyValueLabel(const PropertyName &name,
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
FlexiNoteLayer::setProperty(const PropertyName &name, int value)
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
FlexiNoteLayer::setVerticalScale(VerticalScale scale)
{
    if (m_verticalScale == scale) return;
    m_verticalScale = scale;
    emit layerParametersChanged();
}

bool
FlexiNoteLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

bool
FlexiNoteLayer::shouldConvertMIDIToHz() const
{
    QString unit = getScaleUnits();
    return (unit != "Hz");
//    if (unit == "" ||
//        unit.startsWith("MIDI") ||
//        unit.startsWith("midi")) return true;
//    return false;
}

bool
FlexiNoteLayer::getValueExtents(double &min, double &max,
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
FlexiNoteLayer::getDisplayExtents(double &min, double &max) const
{
    if (!m_model || shouldAutoAlign()) {
//        std::cerr << "No model or shouldAutoAlign()" << std::endl;
        return false;
    }

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
FlexiNoteLayer::setDisplayExtents(double min, double max)
{
    if (!m_model) return false;

    if (min == max) {
        if (min == 0.f) {
            max = 1.f;
        } else {
            max = min * 1.0001f;
        }
    }

    m_scaleMinimum = min;
    m_scaleMaximum = max;

#ifdef DEBUG_NOTE_LAYER
    cerr << "FlexiNoteLayer::setDisplayExtents: min = " << min << ", max = " << max << endl;
#endif
    
    emit layerParametersChanged();
    return true;
}

int
FlexiNoteLayer::getVerticalZoomSteps(int &defaultStep) const
{
    if (shouldAutoAlign()) return 0;
    if (!m_model) return 0;

    defaultStep = 0;
    return 100;
}

int
FlexiNoteLayer::getCurrentVerticalZoomStep() const
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
FlexiNoteLayer::setVerticalZoomStep(int step)
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
    cerr << "FlexiNoteLayer::setVerticalZoomStep: " << step << ": " << newmin << " -> " << newmax << " (range " << newdist << ")" << endl;
#endif

    setDisplayExtents(newmin, newmax);
}

RangeMapper *
FlexiNoteLayer::getNewVerticalZoomRangeMapper() const
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

FlexiNoteModel::PointList
FlexiNoteLayer::getLocalPoints(View *v, int x) const
{
    if (!m_model) return FlexiNoteModel::PointList();

    sv_frame_t frame = v->getFrameForX(x);

    FlexiNoteModel::PointList onPoints =
        m_model->getPoints(frame);

    if (!onPoints.empty()) {
        return onPoints;
    }

    FlexiNoteModel::PointList prevPoints =
        m_model->getPreviousPoints(frame);
    FlexiNoteModel::PointList nextPoints =
        m_model->getNextPoints(frame);

    FlexiNoteModel::PointList usePoints = prevPoints;

    if (prevPoints.empty()) {
        usePoints = nextPoints;
    } else if (prevPoints.begin()->frame < v->getStartFrame() &&
               !(nextPoints.begin()->frame > v->getEndFrame())) {
        usePoints = nextPoints;
    } else if (nextPoints.begin()->frame - frame <
               frame - prevPoints.begin()->frame) {
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
FlexiNoteLayer::getPointToDrag(View *v, int x, int y, FlexiNoteModel::Point &p) const
{
    if (!m_model) return false;

    sv_frame_t frame = v->getFrameForX(x);

    FlexiNoteModel::PointList onPoints = m_model->getPoints(frame);
    if (onPoints.empty()) return false;

//    cerr << "frame " << frame << ": " << onPoints.size() << " candidate points" << endl;

    int nearestDistance = -1;

    for (FlexiNoteModel::PointList::const_iterator i = onPoints.begin();
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

bool
FlexiNoteLayer::getNoteToEdit(View *v, int x, int y, FlexiNoteModel::Point &p) const
{
    // GF: find the note that is closest to the cursor
    if (!m_model) return false;

    sv_frame_t frame = v->getFrameForX(x);

    FlexiNoteModel::PointList onPoints = m_model->getPoints(frame);
    if (onPoints.empty()) return false;

//    std::cerr << "frame " << frame << ": " << onPoints.size() << " candidate points" << std::endl;

    int nearestDistance = -1;

    for (FlexiNoteModel::PointList::const_iterator i = onPoints.begin();
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
FlexiNoteLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    FlexiNoteModel::PointList points = getLocalPoints(v, x);

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return tr("No local points");
        }
    }

    FlexiNote note(0);
    FlexiNoteModel::PointList::iterator i;

    for (i = points.begin(); i != points.end(); ++i) {

        int y = getYForValue(v, i->value);
        int h = NOTE_HEIGHT; // GF: larger notes

        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, i->value + m_model->getValueQuantization());
            if (h < NOTE_HEIGHT) h = NOTE_HEIGHT;
        }

        // GF: this is not quite correct
        if (pos.y() >= y - 4 && pos.y() <= y + h) {
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
        int cents = int(lrint((note.value - double(mnote)) * 100));
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
FlexiNoteLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    FlexiNoteModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalPoints(v, v->getXForFrame(frame));
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }    

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (FlexiNoteModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        cerr << "FlexiNoteModel: point at " << i->frame << endl;

        if (snap == SnapRight) {
            if (i->frame > frame) {
                snapped = i->frame;
                found = true;
                break;
            } else if (i->frame + i->duration >= frame) {
                snapped = i->frame + i->duration;
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
            FlexiNoteModel::PointList::const_iterator j = i;
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

    cerr << "snapToFeatureFrame: frame " << frame << " -> snapped " << snapped << ", found = " << found << endl;

    frame = snapped;
    return found;
}

void
FlexiNoteLayer::getScaleExtents(View *v, double &min, double &max, bool &log) const
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
            cerr << "FlexiNoteLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;
#endif

        } else if (log) {

            LogRange::mapRange(min, max);

#ifdef DEBUG_NOTE_LAYER
            cerr << "FlexiNoteLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;
#endif
        }

    } else {

        getDisplayExtents(min, max);

        if (m_verticalScale == MIDIRangeScale) {
            min = Pitch::getFrequencyForPitch(0);
            max = Pitch::getFrequencyForPitch(70);
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
FlexiNoteLayer::getYForValue(View *v, double val) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    getScaleExtents(v, min, max, logarithmic);

#ifdef DEBUG_NOTE_LAYER
    cerr << "FlexiNoteLayer[" << this << "]::getYForValue(" << val << "): min = " << min << ", max = " << max << ", log = " << logarithmic << endl;
#endif

    if (shouldConvertMIDIToHz()) {
        val = Pitch::getFrequencyForPitch(int(lrint(val)),
                                          int(lrint((val - floor(val)) * 100.0)));
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
FlexiNoteLayer::getValueForY(View *v, int y) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    getScaleExtents(v, min, max, logarithmic);

    double val = min + (double(h - y) * double(max - min)) / h;

    if (logarithmic) {
        val = pow(10.f, val);
    }

    if (shouldConvertMIDIToHz()) {
        val = Pitch::getPitchForFrequency(val);
    }

    return val;
}

bool
FlexiNoteLayer::shouldAutoAlign() const
{
    if (!m_model) return false;
    return (m_verticalScale == AutoAlignScale);
}

void
FlexiNoteLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

//    Profiler profiler("FlexiNoteLayer::paint", true);

    int x1 = rect.right();
    sv_frame_t frame1 = v->getFrameForX(x1);

    FlexiNoteModel::PointList points(m_model->getPoints(0, frame1));
    if (points.empty()) return;

    paint.setPen(getBaseQColor());

    QColor brushColour(getBaseQColor());
    brushColour.setAlpha(80);

//    cerr << "FlexiNoteLayer::paint: resolution is "
//        << m_model->getResolution() << " frames" << endl;

    double min = m_model->getValueMinimum();
    double max = m_model->getValueMaximum();
    if (max == min) max = min + 1.0;

    QPoint localPos;
    FlexiNoteModel::Point illuminatePoint(0);
    bool shouldIlluminate = false;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        shouldIlluminate = getPointToDrag(v, localPos.x(), localPos.y(),
                                          illuminatePoint);
    }

    paint.save();
    paint.setRenderHint(QPainter::Antialiasing, false);
    
    int noteNumber = 0;

    for (FlexiNoteModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        ++noteNumber;
        const FlexiNoteModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);
        int y = getYForValue(v, p.value);
        int w = v->getXForFrame(p.frame + p.duration) - x;
        int h = NOTE_HEIGHT; //GF: larger notes
    
        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, p.value + m_model->getValueQuantization());
            if (h < NOTE_HEIGHT) h = NOTE_HEIGHT; //GF: larger notes
        }

        if (w < 1) w = 1;
        paint.setPen(getBaseQColor());
        paint.setBrush(brushColour);

        if (shouldIlluminate &&
                // "illuminatePoint == p"
                !FlexiNoteModel::Point::Comparator()(illuminatePoint, p) &&
                !FlexiNoteModel::Point::Comparator()(p, illuminatePoint)) {

                paint.drawLine(x, -1, x, v->height() + 1);
                paint.drawLine(x+w, -1, x+w, v->height() + 1);
        
                paint.setPen(v->getForeground());
                // paint.setBrush(v->getForeground());
        
                QString vlabel = QString("freq: %1%2").arg(p.value).arg(m_model->getScaleUnits());
                // v->drawVisibleText(paint, 
                //                    x - paint.fontMetrics().width(vlabel) - 2,
                //                    y + paint.fontMetrics().height()/2
                //                      - paint.fontMetrics().descent(), 
                //                    vlabel, View::OutlinedText);
                v->drawVisibleText(paint, 
                                   x,
                                   y - h/2 - 2 - paint.fontMetrics().height()
                                     - paint.fontMetrics().descent(), 
                                   vlabel, View::OutlinedText);

                QString hlabel = "dur: " + QString(RealTime::frame2RealTime
                    (p.duration, m_model->getSampleRate()).toText(true).c_str());
                v->drawVisibleText(paint, 
                                   x,
                                   y - h/2 - paint.fontMetrics().descent() - 2,
                                   hlabel, View::OutlinedText);

                QString llabel = QString("%1").arg(p.label);
                v->drawVisibleText(paint, 
                                   x,
                                   y + h + 2 + paint.fontMetrics().descent(),
                                   llabel, View::OutlinedText);
                QString nlabel = QString("%1").arg(noteNumber);
                v->drawVisibleText(paint, 
                                   x + paint.fontMetrics().averageCharWidth() / 2,
                                   y + h/2 - paint.fontMetrics().descent(),
                                   nlabel, View::OutlinedText);
        }
    
        paint.drawRect(x, y - h/2, w, h);
    }

    paint.restore();
}

int
FlexiNoteLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
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
FlexiNoteLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect) const
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
FlexiNoteLayer::drawStart(View *v, QMouseEvent *e)
{
//    cerr << "FlexiNoteLayer::drawStart(" << e->x() << "," << e->y() << ")" << endl;

    if (!m_model) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double value = getValueForY(v, e->y());

    m_editingPoint = FlexiNoteModel::Point(frame, float(value), 0, 0.8f, tr("New Point"));
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new FlexiNoteModel::EditCommand(m_model,
                                                       tr("Draw Point"));
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
FlexiNoteLayer::drawDrag(View *v, QMouseEvent *e)
{
//    cerr << "FlexiNoteLayer::drawDrag(" << e->x() << "," << e->y() << ")" << endl;

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
FlexiNoteLayer::drawEnd(View *, QMouseEvent *)
{
//    cerr << "FlexiNoteLayer::drawEnd(" << e->x() << "," << e->y() << ")" << endl;
    if (!m_model || !m_editing) return;
    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
FlexiNoteLayer::eraseStart(View *v, QMouseEvent *e)
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
FlexiNoteLayer::eraseDrag(View *, QMouseEvent *)
{
}

void
FlexiNoteLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    FlexiNoteModel::Point p(0);
    if (!getPointToDrag(v, e->x(), e->y(), p)) return;
    if (p.frame != m_editingPoint.frame || p.value != m_editingPoint.value) return;

    m_editingCommand = new FlexiNoteModel::EditCommand(m_model, tr("Erase Point"));

    m_editingCommand->deletePoint(m_editingPoint);

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
}

void
FlexiNoteLayer::editStart(View *v, QMouseEvent *e)
{
//    cerr << "FlexiNoteLayer::editStart(" << e->x() << "," << e->y() << ")" << endl;
    std::cerr << "FlexiNoteLayer::editStart(" << e->x() << "," << e->y() << ")" << std::endl;

    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;
    m_originalPoint = FlexiNote(m_editingPoint);
    
    if (m_editMode == RightBoundary) {
        m_dragPointX = v->getXForFrame(m_editingPoint.frame + m_editingPoint.duration);
    } else {
        m_dragPointX = v->getXForFrame(m_editingPoint.frame);
    }
    m_dragPointY = getYForValue(v, m_editingPoint.value);

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
    m_dragStartX = e->x();
    m_dragStartY = e->y();
    
    sv_frame_t onset = m_originalPoint.frame;
    sv_frame_t offset = m_originalPoint.frame + m_originalPoint.duration - 1;
    
    m_greatestLeftNeighbourFrame = -1;
    m_smallestRightNeighbourFrame = std::numeric_limits<int>::max();
    
    for (FlexiNoteModel::PointList::const_iterator i = m_model->getPoints().begin();
         i != m_model->getPoints().end(); ++i) {
        FlexiNote currentNote = *i;
        
        // left boundary
        if (currentNote.frame + currentNote.duration - 1 < onset) {
            m_greatestLeftNeighbourFrame = currentNote.frame + currentNote.duration - 1;
        }
        
        // right boundary
        if (currentNote.frame > offset) {
            m_smallestRightNeighbourFrame = currentNote.frame;
            break;
        }
    }
    std::cerr << "editStart: mode is " << m_editMode << ", note frame: " << onset << ", left boundary: " << m_greatestLeftNeighbourFrame << ", right boundary: " << m_smallestRightNeighbourFrame << std::endl;
}

void
FlexiNoteLayer::editDrag(View *v, QMouseEvent *e)
{
//    cerr << "FlexiNoteLayer::editDrag(" << e->x() << "," << e->y() << ")" << endl;
    std::cerr << "FlexiNoteLayer::editDrag(" << e->x() << "," << e->y() << ")" << std::endl;

    if (!m_model || !m_editing) return;

    int xdist = e->x() - m_dragStartX;
    int ydist = e->y() - m_dragStartY;
    int newx = m_dragPointX + xdist;
    int newy = m_dragPointY + ydist;

    sv_frame_t dragFrame = v->getFrameForX(newx);
    if (dragFrame < 0) dragFrame = 0;
    dragFrame = dragFrame / m_model->getResolution() * m_model->getResolution();
    
    double value = getValueForY(v, newy);

    if (!m_editingCommand) {
        m_editingCommand = new FlexiNoteModel::EditCommand(m_model,
                                                           tr("Drag Point"));
    }

    m_editingCommand->deletePoint(m_editingPoint);

    std::cerr << "edit mode: " << m_editMode << " intelligent actions = "
              << m_intelligentActions << std::endl;
    
    switch (m_editMode) {
    case LeftBoundary : {
        // left 
        if (m_intelligentActions && dragFrame <= m_greatestLeftNeighbourFrame) dragFrame = m_greatestLeftNeighbourFrame + 1;
        // right
        if (m_intelligentActions && dragFrame >= m_originalPoint.frame + m_originalPoint.duration) {
            dragFrame = m_originalPoint.frame + m_originalPoint.duration - 1;
        }
        m_editingPoint.frame = dragFrame;
        m_editingPoint.duration = m_originalPoint.frame - dragFrame + m_originalPoint.duration;
        break;
    }
    case RightBoundary : {
        // left
        if (m_intelligentActions && dragFrame <= m_greatestLeftNeighbourFrame) dragFrame = m_greatestLeftNeighbourFrame + 1;
        if (m_intelligentActions && dragFrame >= m_smallestRightNeighbourFrame) dragFrame = m_smallestRightNeighbourFrame - 1;
        m_editingPoint.duration = dragFrame - m_originalPoint.frame + 1;
        break;
    }
    case DragNote : {
        // left
        if (m_intelligentActions && dragFrame <= m_greatestLeftNeighbourFrame) dragFrame = m_greatestLeftNeighbourFrame + 1;
        // right
        if (m_intelligentActions && dragFrame + m_originalPoint.duration >= m_smallestRightNeighbourFrame) {
            dragFrame = m_smallestRightNeighbourFrame - m_originalPoint.duration;
        }
        m_editingPoint.frame = dragFrame;

        m_editingPoint.value = float(value);

        // Re-analyse region within +/- 1 semitone of the dragged value
        float cents = 0;
        int midiPitch = Pitch::getPitchForFrequency(m_editingPoint.value, &cents);
        double lower = Pitch::getFrequencyForPitch(midiPitch - 1, cents);
        double higher = Pitch::getFrequencyForPitch(midiPitch + 1, cents);
        
        emit reAnalyseRegion(m_editingPoint.frame,
                             m_editingPoint.frame + m_editingPoint.duration,
                             float(lower), float(higher));
        break;
    }
    case SplitNote: // nothing
        break;
    }

//    updateNoteValueFromPitchCurve(v, m_editingPoint);
    m_editingCommand->addPoint(m_editingPoint);

    std::cerr << "added new point(" << m_editingPoint.frame << "," << m_editingPoint.duration << ")" << std::endl;
}

void
FlexiNoteLayer::editEnd(View *v, QMouseEvent *e)
{
//    cerr << "FlexiNoteLayer::editEnd(" << e->x() << "," << e->y() << ")" << endl;
    std::cerr << "FlexiNoteLayer::editEnd(" << e->x() << "," << e->y() << ")" << std::endl;
    
    if (!m_model || !m_editing) return;

    if (m_editingCommand) {

        QString newName = m_editingCommand->getName();

        if (m_editMode == DragNote) {
            //!!! command nesting is wrong?
            emit materialiseReAnalysis();
        }

        m_editingCommand->deletePoint(m_editingPoint);
        updateNoteValueFromPitchCurve(v, m_editingPoint);
        m_editingCommand->addPoint(m_editingPoint);

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

void
FlexiNoteLayer::splitStart(View *v, QMouseEvent *e)
{
    // GF: note splitting starts (!! remove printing soon)
    std::cerr << "splitStart (n.b. editStart will be called later, if the user drags the mouse)" << std::endl;
    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;
    // m_originalPoint = m_editingPoint;
    // 
    // m_dragPointX = v->getXForFrame(m_editingPoint.frame);
    // m_dragPointY = getYForValue(v, m_editingPoint.value);

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
    m_dragStartX = e->x();
    m_dragStartY = e->y();
}

void
FlexiNoteLayer::splitEnd(View *v, QMouseEvent *e)
{
    // GF: note splitting ends. (!! remove printing soon)
    std::cerr << "splitEnd" << std::endl;
    if (!m_model || !m_editing || m_editMode != SplitNote) return;

    int xdist = e->x() - m_dragStartX;
    int ydist = e->y() - m_dragStartY;
    if (xdist != 0 || ydist != 0) { 
        std::cerr << "mouse moved" << std::endl;    
        return; 
    }

    sv_frame_t frame = v->getFrameForX(e->x());

    splitNotesAt(v, frame, e);
}

void
FlexiNoteLayer::splitNotesAt(View *v, sv_frame_t frame)
{
    splitNotesAt(v, frame, 0);
}

void
FlexiNoteLayer::splitNotesAt(View *v, sv_frame_t frame, QMouseEvent *e)
{
    FlexiNoteModel::PointList onPoints = m_model->getPoints(frame);
    if (onPoints.empty()) return;
    
    FlexiNote note(*onPoints.begin());

    FlexiNoteModel::EditCommand *command = new FlexiNoteModel::EditCommand
        (m_model, tr("Edit Point"));
    command->deletePoint(note);

    if (!e || !(e->modifiers() & Qt::ShiftModifier)) {

        int gap = 0; // MM: I prefer a gap of 0, but we can decide later
    
        FlexiNote newNote1(note.frame, note.value, 
                           frame - note.frame - gap, 
                           note.level, note.label);
    
        FlexiNote newNote2(frame, note.value, 
                           note.duration - newNote1.duration, 
                           note.level, note.label);
                       
        if (m_intelligentActions) {
            if (updateNoteValueFromPitchCurve(v, newNote1)) {
                command->addPoint(newNote1);
            }
            if (updateNoteValueFromPitchCurve(v, newNote2)) {
                command->addPoint(newNote2);
            }
        } else {
            command->addPoint(newNote1);
            command->addPoint(newNote2);
        }
    }

    finish(command);
}

void
FlexiNoteLayer::addNote(View *v, QMouseEvent *e)
{
    std::cerr << "addNote" << std::endl;
    if (!m_model) return;

    sv_frame_t duration = 10000;
    
    sv_frame_t frame = v->getFrameForX(e->x());
    double value = getValueForY(v, e->y());
    
    FlexiNoteModel::PointList noteList = m_model->getPoints();

    if (m_intelligentActions) {
        sv_frame_t smallestRightNeighbourFrame = 0;
        for (FlexiNoteModel::PointList::const_iterator i = noteList.begin();
             i != noteList.end(); ++i) {
            FlexiNote currentNote = *i;
            if (currentNote.frame > frame) {
                smallestRightNeighbourFrame = currentNote.frame;
                break;
            }
        }
        if (smallestRightNeighbourFrame > 0) {
            duration = std::min(smallestRightNeighbourFrame - frame + 1, duration);
            duration = (duration > 0) ? duration : 0;
        }
    }

    if (!m_intelligentActions || 
        (m_model->getPoints(frame).empty() && duration > 0)) {
        FlexiNote newNote(frame, float(value), duration, 100.f, "new note");
        FlexiNoteModel::EditCommand *command = new FlexiNoteModel::EditCommand
            (m_model, tr("Add Point"));
        command->addPoint(newNote);
        finish(command);
    }
}

SparseTimeValueModel *
FlexiNoteLayer::getAssociatedPitchModel(View *v) const
{
    // Better than we used to do, but still not very satisfactory

//    cerr << "FlexiNoteLayer::getAssociatedPitchModel()" << endl;

    for (int i = 0; i < v->getLayerCount(); ++i) {
        Layer *layer = v->getLayer(i);
        if (layer &&
            layer->getLayerPresentationName() != "candidate") {
//            cerr << "FlexiNoteLayer::getAssociatedPitchModel: looks like our layer is " << layer << endl;
            SparseTimeValueModel *model = qobject_cast<SparseTimeValueModel *>
                (layer->getModel());
//            cerr << "FlexiNoteLayer::getAssociatedPitchModel: and its model is " << model << endl;
            if (model && model->getScaleUnits() == "Hz") {
                cerr << "FlexiNoteLayer::getAssociatedPitchModel: it's good, returning " << model << endl;
                return model;
            }
        }
    }
    cerr << "FlexiNoteLayer::getAssociatedPitchModel: failed to find a model" << endl;
    return 0;
}

void
FlexiNoteLayer::snapSelectedNotesToPitchTrack(View *v, Selection s)
{
    if (!m_model) return;

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    FlexiNoteModel::EditCommand *command = new FlexiNoteModel::EditCommand
        (m_model, tr("Snap Notes"));

    cerr << "snapSelectedNotesToPitchTrack: selection is from " << s.getStartFrame() << " to " << s.getEndFrame() << endl;

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        FlexiNote note(*i);

        cerr << "snapSelectedNotesToPitchTrack: looking at note from " << note.frame << " to " << note.frame + note.duration << endl;

        if (!s.contains(note.frame) &&
            !s.contains(note.frame + note.duration - 1)) {
            continue;
        }

        cerr << "snapSelectedNotesToPitchTrack: making new note" << endl;
        FlexiNote newNote(note);

        command->deletePoint(note);

        if (updateNoteValueFromPitchCurve(v, newNote)) {
            command->addPoint(newNote);
        }
    }
    
    finish(command);
}

void
FlexiNoteLayer::mergeNotes(View *v, Selection s, bool inclusive)
{
    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    FlexiNoteModel::PointList::iterator i = points.begin();
    if (inclusive) {
        while (i != points.end() && i->frame + i->duration < s.getStartFrame()) {
            ++i;
        }
    } else {
        while (i != points.end() && i->frame < s.getStartFrame()) {
            ++i;
        }
    }
        
    if (i == points.end()) return;

    FlexiNoteModel::EditCommand *command = 
        new FlexiNoteModel::EditCommand(m_model, tr("Merge Notes"));

    FlexiNote newNote(*i);

    while (i != points.end()) {

        if (inclusive) {
            if (i->frame >= s.getEndFrame()) break;
        } else {
            if (i->frame + i->duration > s.getEndFrame()) break;
        }

        newNote.duration = i->frame + i->duration - newNote.frame;
        command->deletePoint(*i);

        ++i;
    }

    updateNoteValueFromPitchCurve(v, newNote);
    command->addPoint(newNote);
    finish(command);
}

bool
FlexiNoteLayer::updateNoteValueFromPitchCurve(View *v,
                                              FlexiNoteModel::Point &note) const
{
    SparseTimeValueModel *model = getAssociatedPitchModel(v);
    if (!model) return false;
        
    std::cerr << model->getTypeName() << std::endl;

    SparseModel<TimeValuePoint>::PointList dataPoints =
        model->getPoints(note.frame, note.frame + note.duration);
   
    std::cerr << "frame " << note.frame << ": " << dataPoints.size() << " candidate points" << std::endl;
   
    if (dataPoints.empty()) return false;

    std::vector<double> pitchValues;
   
    for (SparseModel<TimeValuePoint>::PointList::const_iterator i =
             dataPoints.begin(); i != dataPoints.end(); ++i) {
        if (i->frame >= note.frame &&
            i->frame < note.frame + note.duration) {
            pitchValues.push_back(i->value);
        }
    }
        
    if (pitchValues.empty()) return false;

    sort(pitchValues.begin(), pitchValues.end());
    int size = int(pitchValues.size());
    double median;

    if (size % 2 == 0) {
        median = (pitchValues[size/2 - 1] + pitchValues[size/2]) / 2;
    } else {
        median = pitchValues[size/2];
    }

    std::cerr << "updateNoteValueFromPitchCurve: corrected from " << note.value << " to median " << median << std::endl;
    
    note.value = float(median);

    return true;
}

void 
FlexiNoteLayer::mouseMoveEvent(View *v, QMouseEvent *e)
{
    // GF: context sensitive cursors
    // v->setCursor(Qt::ArrowCursor);
    FlexiNoteModel::Point note(0);
    if (!getNoteToEdit(v, e->x(), e->y(), note)) { 
        // v->setCursor(Qt::UpArrowCursor);
        return; 
    }

    bool closeToLeft = false, closeToRight = false,
        closeToTop = false, closeToBottom = false;
    getRelativeMousePosition(v, note, e->x(), e->y(),
                             closeToLeft, closeToRight,
                             closeToTop, closeToBottom);
    
    if (closeToLeft) {
        v->setCursor(Qt::SizeHorCursor);
        m_editMode = LeftBoundary;
        cerr << "edit mode -> LeftBoundary" << endl;
    } else if (closeToRight) {
        v->setCursor(Qt::SizeHorCursor);
        m_editMode = RightBoundary;
        cerr << "edit mode -> RightBoundary" << endl;
    } else if (closeToTop) {
        v->setCursor(Qt::CrossCursor);
        m_editMode = DragNote;
        cerr << "edit mode -> DragNote" << endl;
    } else if (closeToBottom) {
        v->setCursor(Qt::UpArrowCursor);
        m_editMode = SplitNote;
        cerr << "edit mode -> SplitNote" << endl;
    } else {
        v->setCursor(Qt::ArrowCursor);
    }
}

void
FlexiNoteLayer::getRelativeMousePosition(View *v, FlexiNoteModel::Point &note, int x, int y, bool &closeToLeft, bool &closeToRight, bool &closeToTop, bool &closeToBottom) const
{
    // GF: TODO: consoloidate the tolerance values
    if (!m_model) return;

    int ctol = 0;
    int noteStartX = v->getXForFrame(note.frame);
    int noteEndX = v->getXForFrame(note.frame + note.duration);
    int noteValueY = getYForValue(v,note.value);
    int noteStartY = noteValueY - (NOTE_HEIGHT / 2);
    int noteEndY = noteValueY + (NOTE_HEIGHT / 2);
    
    bool closeToNote = false;
    
    if (y >= noteStartY-ctol && y <= noteEndY+ctol && x >= noteStartX-ctol && x <= noteEndX+ctol) closeToNote = true;
    if (!closeToNote) return;
    
    int tol = NOTE_HEIGHT / 2;
    
    if (x >= noteStartX - tol && x <= noteStartX + tol) closeToLeft = true;
    if (x >= noteEndX - tol && x <= noteEndX + tol) closeToRight = true;
    if (y >= noteStartY - tol && y <= noteStartY + tol) closeToTop = true;
    if (y >= noteEndY - tol && y <= noteEndY + tol) closeToBottom = true;

//    cerr << "FlexiNoteLayer::getRelativeMousePosition: close to: left " << closeToLeft << " right " << closeToRight << " top " << closeToTop << " bottom " << closeToBottom << endl;
}


bool
FlexiNoteLayer::editOpen(View *v, QMouseEvent *e)
{
    std::cerr << "Opening note editor dialog" << std::endl;
    if (!m_model) return false;

    FlexiNoteModel::Point note(0);
    if (!getPointToDrag(v, e->x(), e->y(), note)) return false;

//    FlexiNoteModel::Point note = *points.begin();

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

        FlexiNoteModel::Point newNote = note;
        newNote.frame = dialog->getFrameTime();
        newNote.value = dialog->getValue();
        newNote.duration = dialog->getFrameDuration();
        newNote.label = dialog->getText();
        
        FlexiNoteModel::EditCommand *command = new FlexiNoteModel::EditCommand
            (m_model, tr("Edit Point"));
        command->deletePoint(note);
        command->addPoint(newNote);
        finish(command);
    }

    delete dialog;
    return true;
}

void
FlexiNoteLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    FlexiNoteModel::EditCommand *command =
        new FlexiNoteModel::EditCommand(m_model, tr("Drag Selection"));

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            FlexiNoteModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
FlexiNoteLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    FlexiNoteModel::EditCommand *command =
        new FlexiNoteModel::EditCommand(m_model, tr("Resize Selection"));

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
        double(newSize.getEndFrame() - newSize.getStartFrame()) /
        double(s.getEndFrame() - s.getStartFrame());

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double targetStart = double(i->frame);
            targetStart = double(newSize.getStartFrame()) +
                targetStart - double(s.getStartFrame()) * ratio;

            double targetEnd = double(i->frame + i->duration);
            targetEnd = double(newSize.getStartFrame()) +
                targetEnd - double(s.getStartFrame()) * ratio;

            FlexiNoteModel::Point newPoint(*i);
            newPoint.frame = lrint(targetStart);
            newPoint.duration = lrint(targetEnd - targetStart);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
}

void
FlexiNoteLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    FlexiNoteModel::EditCommand *command =
        new FlexiNoteModel::EditCommand(m_model, tr("Delete Selected Points"));

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            command->deletePoint(*i);
        }
    }

    finish(command);
}    

void
FlexiNoteLayer::deleteSelectionInclusive(Selection s)
{
    if (!m_model) return;

    FlexiNoteModel::EditCommand *command =
        new FlexiNoteModel::EditCommand(m_model, tr("Delete Selected Points"));

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        bool overlap = !(
            ((s.getStartFrame() <= i->frame) && (s.getEndFrame() <= i->frame)) || // selection is left of note
            ((s.getStartFrame() >= (i->frame+i->duration)) && (s.getEndFrame() >= (i->frame+i->duration))) // selection is right of note
            );
        if (overlap) {
            command->deletePoint(*i);
        }
    }

    finish(command);
}

void
FlexiNoteLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    FlexiNoteModel::PointList points =
        m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (FlexiNoteModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->value, i->duration, i->level, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
FlexiNoteLayer::paste(View *v, const Clipboard &from, sv_frame_t /*frameOffset */, bool /* interactive */)
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

    FlexiNoteModel::EditCommand *command =
        new FlexiNoteModel::EditCommand(m_model, tr("Paste"));

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

        FlexiNoteModel::Point newPoint(frame);
  
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
FlexiNoteLayer::addNoteOn(sv_frame_t frame, int pitch, int velocity)
{
    m_pendingNoteOns.insert(FlexiNote(frame, float(pitch), 0, float(velocity / 127.0), ""));
}

void
FlexiNoteLayer::addNoteOff(sv_frame_t frame, int pitch)
{
    for (FlexiNoteSet::iterator i = m_pendingNoteOns.begin();
         i != m_pendingNoteOns.end(); ++i) {
        if (lrint((*i).value) == pitch) {
            FlexiNote note(*i);
            m_pendingNoteOns.erase(i);
            note.duration = frame - note.frame;
            if (m_model) {
                FlexiNoteModel::AddPointCommand *c = new FlexiNoteModel::AddPointCommand
                    (m_model, note, tr("Record FlexiNote"));
                // execute and bundle:
                CommandHistory::getInstance()->addCommand(c, true, true);
            }
            break;
        }
    }
}

void
FlexiNoteLayer::abandonNoteOns()
{
    m_pendingNoteOns.clear();
}

int
FlexiNoteLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
        (QString(darkbg ? "White" : "Black"));
}

void
FlexiNoteLayer::toXml(QTextStream &stream,
                      QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes +
                             QString(" verticalScale=\"%1\" scaleMinimum=\"%2\" scaleMaximum=\"%3\" ")
                             .arg(m_verticalScale)
                             .arg(m_scaleMinimum)
                             .arg(m_scaleMaximum));
}

void
FlexiNoteLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);

    bool ok;
    VerticalScale scale = (VerticalScale)
        attributes.value("verticalScale").toInt(&ok);
    if (ok) setVerticalScale(scale);

//    bool alsoOk;
//    double min = attributes.value("scaleMinimum").toDouble(&ok);
//    double max = attributes.value("scaleMaximum").toDouble(&alsoOk);
//    if (ok && alsoOk && min != max) setDisplayExtents(min, max);
}

void
FlexiNoteLayer::setVerticalRangeToNoteRange(View *v)
{
    double minf = std::numeric_limits<double>::max();
    double maxf = 0;
    bool hasNotes = 0;
    for (FlexiNoteModel::PointList::const_iterator i = m_model->getPoints().begin();
         i != m_model->getPoints().end(); ++i) {
        hasNotes = 1;
        FlexiNote note = *i;
        if (note.value < minf) minf = note.value;
        if (note.value > maxf) maxf = note.value;
    }
    
    std::cerr << "min frequency:" << minf << ", max frequency: " << maxf << std::endl;
    
    if (hasNotes) {
        v->getLayer(1)->setDisplayExtents(minf*0.66,maxf*1.5); 
        // MM: this is a hack because we rely on 
        // * this layer being automatically aligned to layer 1
        // * layer one is a log frequency layer.
    }
}


