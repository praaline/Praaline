/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2008 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "RegionLayer.h"

#include "data/model/Model.h"
#include "base/RealTime.h"
#include "base/Profiler.h"
#include "base/LogRange.h"
#include "ColourDatabase.h"

#include "ColourMapper.h"
#include "LinearNumericalScale.h"
#include "LogNumericalScale.h"
#include "LinearColourScale.h"
#include "LogColourScale.h"
#include "LayerFactory.h"

#include "view/View.h"

#include "data/model/RegionModel.h"

#include "widgets/ItemEditDialog.h"
#include "widgets/TextAbbrev.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QTextStream>
#include <QMessageBox>

#include <iostream>
#include <cmath>

RegionLayer::RegionLayer() :
    SingleColourLayer(),
    m_model(0),
    m_editing(false),
    m_dragPointX(0),
    m_dragPointY(0),
    m_dragStartX(0),
    m_dragStartY(0),
    m_originalPoint(0, 0.0, 0, tr("New Region")),
    m_editingPoint(0, 0.0, 0, tr("New Region")),
    m_editingCommand(0),
    m_verticalScale(EqualSpaced),
    m_colourMap(0),
    m_plotStyle(PlotLines)
{
    
}

bool RegionLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<RegionLayer, RegionModel>(this, model))
        return true;
    return false;
}

void
RegionLayer::setModel(RegionModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    connect(m_model, SIGNAL(modelChanged()), this, SLOT(recalcSpacing()));
    recalcSpacing();

    //    cerr << "RegionLayer::setModel(" << model << ")" << endl;

    if (m_model && m_model->getRDFTypeURI().endsWith("Segment")) {
        setPlotStyle(PlotSegmentation);
    }
    if (m_model && m_model->getRDFTypeURI().endsWith("Change")) {
        setPlotStyle(PlotSegmentation);
    }

    emit modelReplaced();
}

// static
Model *RegionLayer::createEmptyModel(Model *baseModel)
{
    return new RegionModel(baseModel->getSampleRate(), 1, true);
}

Layer::PropertyList
RegionLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Vertical Scale");
    list.push_back("Scale Units");
    list.push_back("Plot Type");
    return list;
}

QString
RegionLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Vertical Scale") return tr("Vertical Scale");
    if (name == "Scale Units") return tr("Scale Units");
    if (name == "Plot Type") return tr("Plot Type");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
RegionLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Scale Units") return UnitsProperty;
    if (name == "Vertical Scale") return ValueProperty;
    if (name == "Plot Type") return ValueProperty;
    if (name == "Colour" && m_plotStyle == PlotSegmentation) return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

QString
RegionLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Vertical Scale" || name == "Scale Units") {
        return tr("Scale");
    }
    return SingleColourLayer::getPropertyGroupName(name);
}

int
RegionLayer::getPropertyRangeAndValue(const PropertyName &name,
                                      int *min, int *max, int *deflt) const
{
    int val = 0;

    if (name == "Colour" && m_plotStyle == PlotSegmentation) {

        if (min) *min = 0;
        if (max) *max = ColourMapper::getColourMapCount() - 1;
        if (deflt) *deflt = 0;
        
        val = m_colourMap;

    } else if (name == "Plot Type") {

        if (min) *min = 0;
        if (max) *max = 1;
        if (deflt) *deflt = 0;

        val = int(m_plotStyle);

    } else if (name == "Vertical Scale") {

        if (min) *min = 0;
        if (max) *max = 3;
        if (deflt) *deflt = int(EqualSpaced);

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
RegionLayer::getPropertyValueLabel(const PropertyName &name,
                                   int value) const
{
    if (name == "Colour" && m_plotStyle == PlotSegmentation) {
        return ColourMapper::getColourMapName(value);
    } else if (name == "Plot Type") {

        switch (value) {
        default:
        case 0: return tr("Bars");
        case 1: return tr("Segmentation");
        }

    } else if (name == "Vertical Scale") {
        switch (value) {
        default:
        case 0: return tr("Auto-Align");
        case 1: return tr("Equal Spaced");
        case 2: return tr("Linear");
        case 3: return tr("Log");
        }
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void
RegionLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Colour" && m_plotStyle == PlotSegmentation) {
        setFillColourMap(value);
    } else if (name == "Plot Type") {
        setPlotStyle(PlotStyle(value));
    } else if (name == "Vertical Scale") {
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
RegionLayer::setFillColourMap(int map)
{
    if (m_colourMap == map) return;
    m_colourMap = map;
    emit layerParametersChanged();
}

void
RegionLayer::setPlotStyle(PlotStyle style)
{
    if (m_plotStyle == style) return;
    bool colourTypeChanged = (style == PlotSegmentation ||
                              m_plotStyle == PlotSegmentation);
    m_plotStyle = style;
    if (colourTypeChanged) {
        emit layerParameterRangesChanged();
    }
    emit layerParametersChanged();
}

void
RegionLayer::setVerticalScale(VerticalScale scale)
{
    if (m_verticalScale == scale) return;
    m_verticalScale = scale;
    emit layerParametersChanged();
}

bool
RegionLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

void
RegionLayer::recalcSpacing()
{
    m_spacingMap.clear();
    m_distributionMap.clear();
    if (!m_model) return;

    //    cerr << "RegionLayer::recalcSpacing" << endl;

    for (RegionModel::PointList::const_iterator i = m_model->getPoints().begin();
         i != m_model->getPoints().end(); ++i) {
        m_distributionMap[i->value]++;
        //        cerr << "RegionLayer::recalcSpacing: value found: " << i->value << " (now have " << m_distributionMap[i->value] << " of this value)" <<  endl;
    }

    int n = 0;

    for (SpacingMap::const_iterator i = m_distributionMap.begin();
         i != m_distributionMap.end(); ++i) {
        m_spacingMap[i->first] = n++;
        //        cerr << "RegionLayer::recalcSpacing: " << i->first << " -> " << m_spacingMap[i->first] << endl;
    }
}

bool
RegionLayer::getValueExtents(double &min, double &max,
                             bool &logarithmic, QString &unit) const
{
    if (!m_model) return false;
    min = m_model->getValueMinimum();
    max = m_model->getValueMaximum();
    unit = getScaleUnits();

    if (m_verticalScale == LogScale) logarithmic = true;

    return true;
}

bool
RegionLayer::getDisplayExtents(double &min, double &max) const
{
    if (!m_model ||
            m_verticalScale == AutoAlignScale ||
            m_verticalScale == EqualSpaced) return false;

    min = m_model->getValueMinimum();
    max = m_model->getValueMaximum();

    return true;
}

RegionModel::PointList
RegionLayer::getLocalPoints(View *v, int x) const
{
    if (!m_model) return RegionModel::PointList();

    long frame = v->getFrameForX(x);

    RegionModel::PointList onPoints =
            m_model->getPoints(frame);

    if (!onPoints.empty()) {
        return onPoints;
    }

    RegionModel::PointList prevPoints =
            m_model->getPreviousPoints(frame);
    RegionModel::PointList nextPoints =
            m_model->getNextPoints(frame);

    RegionModel::PointList usePoints = prevPoints;

    if (prevPoints.empty()) {
        usePoints = nextPoints;
    } else if (long(prevPoints.begin()->frame) < v->getStartFrame() &&
               !(nextPoints.begin()->frame > v->getEndFrame())) {
        usePoints = nextPoints;
    } else if (long(nextPoints.begin()->frame) - frame <
               frame - long(prevPoints.begin()->frame)) {
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
RegionLayer::getPointToDrag(View *v, int x, int y, RegionModel::Point &p) const
{
    if (!m_model) return false;

    long frame = v->getFrameForX(x);

    RegionModel::PointList onPoints = m_model->getPoints(frame);
    if (onPoints.empty()) return false;

    int nearestDistance = -1;

    for (RegionModel::PointList::const_iterator i = onPoints.begin();
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
RegionLayer::getLabelPreceding(sv_frame_t frame) const
{
    if (!m_model) return "";
    RegionModel::PointList points = m_model->getPreviousPoints(frame);
    for (RegionModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        if (i->label != "") return i->label;
    }
    return "";
}

QString
RegionLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    RegionModel::PointList points = getLocalPoints(v, x);

    if (points.empty()) {
        if (!m_model->isReady()) {
            return tr("In progress");
        } else {
            return tr("No local points");
        }
    }

    RegionRec region(0);
    RegionModel::PointList::iterator i;

    //!!! harmonise with whatever decision is made about point y
    //!!! coords in paint method

    for (i = points.begin(); i != points.end(); ++i) {

        int y = getYForValue(v, i->value);
        int h = 3;

        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, i->value + m_model->getValueQuantization());
            if (h < 3) h = 3;
        }

        if (pos.y() >= y - h && pos.y() <= y) {
            region = *i;
            break;
        }
    }

    if (i == points.end()) return tr("No local points");

    RealTime rt = RealTime::frame2RealTime(region.frame,
                                           m_model->getSampleRate());
    RealTime rd = RealTime::frame2RealTime(region.duration,
                                           m_model->getSampleRate());
    
    QString valueText;

    valueText = tr("%1 %2").arg(region.value).arg(getScaleUnits());

    QString text;

    if (region.label == "") {
        text = QString(tr("Time:\t%1\nValue:\t%2\nDuration:\t%3\nNo label"))
                .arg(rt.toText(true).c_str())
                .arg(valueText)
                .arg(rd.toText(true).c_str());
    } else {
        text = QString(tr("Time:\t%1\nValue:\t%2\nDuration:\t%3\nLabel:\t%4"))
                .arg(rt.toText(true).c_str())
                .arg(valueText)
                .arg(rd.toText(true).c_str())
                .arg(region.label);
    }

    pos = QPoint(v->getXForFrame(region.frame),
                 getYForValue(v, region.value));
    return text;
}

bool
RegionLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    RegionModel::PointList points;

    if (snap == SnapNeighbouring) {
        points = getLocalPoints(v, v->getXForFrame(frame));
        if (points.empty()) return false;
        frame = points.begin()->frame;
        return true;
    }

    points = m_model->getPoints(frame, frame);
    sv_frame_t snapped = frame;
    bool found = false;

    for (RegionModel::PointList::const_iterator i = points.begin(); i != points.end(); ++i) {

        if (snap == SnapRight) {
            // The best frame to snap to is the end frame of whichever
            // feature we would have snapped to the start frame of if
            // we had been snapping left.
            if (i->frame <= frame) {
                if (i->frame + i->duration > frame) {
                    snapped = i->frame + i->duration;
                    found = true; // don't break, as the next may be better
                }
            } else {
                if (!found) {
                    snapped = i->frame;
                    found = true;
                }
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
            RegionModel::PointList::const_iterator j = i;
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

bool
RegionLayer::snapToSimilarFeature(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToSimilarFeature(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();

    const RegionModel::PointList &points = m_model->getPoints();
    RegionModel::PointList close = m_model->getPoints(frame, frame);

    RegionModel::PointList::const_iterator i;

    sv_frame_t matchframe = frame;
    double matchvalue = 0.f;

    for (i = close.begin(); i != close.end(); ++i) {
        if (i->frame > frame) break;
        matchvalue = i->value;
        matchframe = i->frame;
    }

    sv_frame_t snapped = frame;
    bool found = false;
    bool distant = false;
    double epsilon = 0.0001;

    i = close.begin();

    // Scan through the close points first, then the more distant ones
    // if no suitable close one is found. So the while-termination
    // condition here can only happen once i has passed through the
    // whole of the close container and then the whole of the separate
    // points container. The two iterators are totally distinct, but
    // have the same type so we cheekily use the same variable and a
    // single loop for both.

    while (i != points.end()) {
        if (!distant) {
            if (i == close.end()) {
                // switch from the close container to the points container
                i = points.begin();
                distant = true;
            }
        }
        if (snap == SnapRight) {
            if (i->frame > matchframe &&
                    fabs(i->value - matchvalue) < epsilon) {
                snapped = i->frame;
                found = true;
                break;
            }
        } else if (snap == SnapLeft) {
            if (i->frame < matchframe) {
                if (fabs(i->value - matchvalue) < epsilon) {
                    snapped = i->frame;
                    found = true; // don't break, as the next may be better
                }
            } else if (found || distant) {
                break;
            }

        } else {
            // no other snap types supported
        }
        ++i;
    }
    frame = snapped;
    return found;
}

QString
RegionLayer::getScaleUnits() const
{
    if (m_model) return m_model->getScaleUnits();
    else return "";
}

void
RegionLayer::getScaleExtents(View *v, double &min, double &max, bool &log) const
{
    min = 0.0;
    max = 0.0;
    log = false;

    QString queryUnits;
    queryUnits = getScaleUnits();

    if (m_verticalScale == AutoAlignScale) {

        if (!v->getValueExtents(queryUnits, min, max, log)) {

            min = m_model->getValueMinimum();
            max = m_model->getValueMaximum();

            //            cerr << "RegionLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;

        } else if (log) {

            LogRange::mapRange(min, max);

            //            cerr << "RegionLayer[" << this << "]::getScaleExtents: min = " << min << ", max = " << max << ", log = " << log << endl;

        }

    } else if (m_verticalScale == EqualSpaced) {

        if (!m_spacingMap.empty()) {
            SpacingMap::const_iterator i = m_spacingMap.begin();
            min = i->second;
            i = m_spacingMap.end();
            --i;
            max = i->second;
            //            cerr << "RegionLayer[" << this << "]::getScaleExtents: equal spaced; min = " << min << ", max = " << max << ", log = " << log << endl;
        }

    } else {

        min = m_model->getValueMinimum();
        max = m_model->getValueMaximum();

        if (m_verticalScale == LogScale) {
            LogRange::mapRange(min, max);
            log = true;
        }
    }

    if (max == min) max = min + 1.0;
}

int
RegionLayer::spacingIndexToY(View *v, int i) const
{
    int h = v->height();
    int n = int(m_spacingMap.size());
    // this maps from i (spacing of the value from the spacing
    // map) and n (number of region types) to y
    int y = h - (((h * i) / n) + (h / (2 * n)));
    return y;
}

double
RegionLayer::yToSpacingIndex(View *v, int y) const
{
    // we return an inexact result here (double rather than int)
    int h = v->height();
    int n = int(m_spacingMap.size());
    // from y = h - ((h * i) / n) + (h / (2 * n)) as above (vh taking place of i)
    double vh = double(2*h*n - h - 2*n*y) / double(2*h);
    return vh;
}

int
RegionLayer::getYForValue(View *v, double val) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    if (m_verticalScale == EqualSpaced) {

        if (m_spacingMap.empty()) return h/2;
        
        SpacingMap::const_iterator i = m_spacingMap.lower_bound(val);
        //!!! what now, if i->first != v?

        int y = spacingIndexToY(v, i->second);

        //        cerr << "RegionLayer::getYForValue: value " << val << " -> i->second " << i->second << " -> y " << y << endl;
        return y;


    } else {

        getScaleExtents(v, min, max, logarithmic);

        //    cerr << "RegionLayer[" << this << "]::getYForValue(" << val << "): min = " << min << ", max = " << max << ", log = " << logarithmic << endl;
        //    cerr << "h = " << h << ", margin = " << margin << endl;

        if (logarithmic) {
            val = LogRange::map(val);
        }

        return int(h - ((val - min) * h) / (max - min));
    }
}

double
RegionLayer::getValueForY(View *v, int y) const
{
    return getValueForY(v, y, -1);
}

double
RegionLayer::getValueForY(View *v, int y, int avoid) const
{
    double min = 0.0, max = 0.0;
    bool logarithmic = false;
    int h = v->height();

    if (m_verticalScale == EqualSpaced) {

        // if we're equal spaced, we probably want to snap to the
        // nearest item when close to it, and give some notification
        // that we're doing so

        if (m_spacingMap.empty()) return 1.f;

        // n is the number of distinct regions.  if we are close to
        // one of the m/n divisions in the y scale, we should snap to
        // the value of the mth region.

        double vh = yToSpacingIndex(v, y);

        // spacings in the map are integral, so find the closest one,
        // map it back to its y coordinate, and see how far we are
        // from it

        int n = int(m_spacingMap.size());
        int ivh = int(lrint(vh));
        if (ivh < 0) ivh = 0;
        if (ivh > n-1) ivh = n-1;
        int iy = spacingIndexToY(v, ivh);

        int dist = iy - y;
        int gap = h / n; // between region lines

        //        cerr << "getValueForY: y = " << y << ", vh = " << vh << ", ivh = " << ivh << " of " << n << ", iy = " << iy << ", dist = " << dist << ", gap = " << gap << endl;

        SpacingMap::const_iterator i = m_spacingMap.begin();
        while (i != m_spacingMap.end()) {
            if (i->second == ivh) break;
            ++i;
        }
        if (i == m_spacingMap.end()) i = m_spacingMap.begin();

        //        cerr << "nearest existing value = " << i->first << " at " << iy << endl;

        double val = 0;

        //        cerr << "note: avoid = " << avoid << ", i->second = " << i->second << endl;

        if (dist < -gap/3 &&
                ((avoid == -1) ||
                 (avoid != i->second && avoid != i->second - 1))) {
            // bisect gap to prior
            if (i == m_spacingMap.begin()) {
                val = i->first - 1.f;
                //                cerr << "extended down to " << val << endl;
            } else {
                SpacingMap::const_iterator j = i;
                --j;
                val = (i->first + j->first) / 2;
                //                cerr << "bisected down to " << val << endl;
            }
        } else if (dist > gap/3 &&
                   ((avoid == -1) ||
                    (avoid != i->second && avoid != i->second + 1))) {
            // bisect gap to following
            SpacingMap::const_iterator j = i;
            ++j;
            if (j == m_spacingMap.end()) {
                val = i->first + 1.f;
                //                cerr << "extended up to " << val << endl;
            } else {
                val = (i->first + j->first) / 2;
                //                cerr << "bisected up to " << val << endl;
            }
        } else {
            // snap
            val = i->first;
            //            cerr << "snapped to " << val << endl;
        }

        return val;

    } else {

        getScaleExtents(v, min, max, logarithmic);

        double val = min + (double(h - y) * double(max - min)) / h;

        if (logarithmic) {
            val = pow(10.0, val);
        }

        return val;
    }
}

QColor
RegionLayer::getColourForValue(View *v, double val) const
{
    double min, max;
    bool log;
    getScaleExtents(v, min, max, log);

    if (min > max) std::swap(min, max);
    if (max == min) max = min + 1;

    if (log) {
        LogRange::mapRange(min, max);
        val = LogRange::map(val);
    }

    //    cerr << "RegionLayer::getColourForValue: min " << min << ", max "
    //              << max << ", log " << log << ", value " << val << endl;

    QColor solid = ColourMapper(m_colourMap, min, max).map(val);
    return QColor(solid.red(), solid.green(), solid.blue(), 120);
}

int
RegionLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "Bright Blue" : "Blue"));
}

void
RegionLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    //    Profiler profiler("RegionLayer::paint", true);

    int x0 = rect.left() - 40, x1 = rect.right();
    sv_frame_t frame0 = v->getFrameForX(x0);
    sv_frame_t frame1 = v->getFrameForX(x1);

    RegionModel::PointList points(m_model->getPoints(frame0, frame1));
    if (points.empty()) return;

    paint.setPen(getBaseQColor());

    QColor brushColour(getBaseQColor());
    brushColour.setAlpha(80);

    //    cerr << "RegionLayer::paint: resolution is "
    //	      << m_model->getResolution() << " frames" << endl;

    double min = m_model->getValueMinimum();
    double max = m_model->getValueMaximum();
    if (max == min) max = min + 1.0;

    QPoint localPos;
    RegionModel::Point illuminatePoint(0);
    bool shouldIlluminate = false;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
        shouldIlluminate = getPointToDrag(v, localPos.x(), localPos.y(),
                                          illuminatePoint);
    }

    paint.save();
    paint.setRenderHint(QPainter::Antialiasing, false);
    
    //!!! point y coords if model does not haveDistinctValues() should
    //!!! be assigned to avoid overlaps

    //!!! if it does have distinct values, we should still ensure y
    //!!! coord is never completely flat on the top or bottom

    int fontHeight = paint.fontMetrics().height();

    for (RegionModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        const RegionModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);
        int y = getYForValue(v, p.value);
        int w = v->getXForFrame(p.frame + p.duration) - x;
        int h = 9;
        int ex = x + w;

        RegionModel::PointList::const_iterator j = i;
        ++j;

        if (j != points.end()) {
            const RegionModel::Point &q(*j);
            int nx = v->getXForFrame(q.frame);
            if (nx < ex) ex = nx;
        }

        if (m_model->getValueQuantization() != 0.0) {
            h = y - getYForValue(v, p.value + m_model->getValueQuantization());
            if (h < 3) h = 3;
        }

        if (w < 1) w = 1;

        if (m_plotStyle == PlotSegmentation) {
            paint.setPen(getForegroundQColor(v));
            paint.setBrush(getColourForValue(v, p.value));
        } else {
            paint.setPen(getBaseQColor());
            paint.setBrush(brushColour);
        }

        if (m_plotStyle == PlotSegmentation) {

            if (ex <= x) continue;

            if (!shouldIlluminate ||
                    // "illuminatePoint != p"
                    RegionModel::Point::Comparator()(illuminatePoint, p) ||
                    RegionModel::Point::Comparator()(p, illuminatePoint)) {

                paint.setPen(QPen(getForegroundQColor(v), 1));
                paint.drawLine(x, 0, x, v->height());
                paint.setPen(Qt::NoPen);

            } else {
                paint.setPen(QPen(getForegroundQColor(v), 2));
            }

            paint.drawRect(x, -1, ex - x, v->height() + 2);

        } else {

            if (shouldIlluminate &&
                    // "illuminatePoint == p"
                    !RegionModel::Point::Comparator()(illuminatePoint, p) &&
                    !RegionModel::Point::Comparator()(p, illuminatePoint)) {

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
            
            paint.drawLine(x, y-1, x + w, y-1);
            paint.drawLine(x, y+1, x + w, y+1);
            paint.drawLine(x, y - h/2, x, y + h/2);
            paint.drawLine(x+w, y - h/2, x + w, y + h/2);
        }
    }

    int nextLabelMinX = -100;
    int lastLabelY = 0;

    for (RegionModel::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {

        const RegionModel::Point &p(*i);

        int x = v->getXForFrame(p.frame);
        int y = getYForValue(v, p.value);

        bool illuminated = false;

        if (m_plotStyle != PlotSegmentation) {

            if (shouldIlluminate &&
                    // "illuminatePoint == p"
                    !RegionModel::Point::Comparator()(illuminatePoint, p) &&
                    !RegionModel::Point::Comparator()(p, illuminatePoint)) {

                illuminated = true;
            }
        }

        if (!illuminated) {
            QString label = p.label;
            if (label == "") {
                label = QString("%1%2").arg(p.value).arg(getScaleUnits());
            }

            int labelX, labelY;

            if (m_plotStyle != PlotSegmentation) {
                labelX = x - paint.fontMetrics().width(label) - 2;
                labelY = y + paint.fontMetrics().height()/2
                        - paint.fontMetrics().descent();
            } else {
                labelX = x + 5;
                labelY = v->getTextLabelHeight(this, paint);
                if (labelX < nextLabelMinX) {
                    if (lastLabelY < v->height()/2) {
                        labelY = lastLabelY + fontHeight;
                    }
                }
                lastLabelY = labelY;
                nextLabelMinX = labelX + paint.fontMetrics().width(label);
            }

            v->drawVisibleText(paint, labelX, labelY, label, View::OutlinedText);
        }
    }

    paint.restore();
}

int
RegionLayer::getVerticalScaleWidth(View *v, bool, QPainter &paint) const
{
    if (!m_model ||
            m_verticalScale == AutoAlignScale ||
            m_verticalScale == EqualSpaced) {
        return 0;
    } else if (m_plotStyle == PlotSegmentation) {
        if (m_verticalScale == LogScale) {
            return LogColourScale().getWidth(v, paint);
        } else {
            return LinearColourScale().getWidth(v, paint);
        }
    } else {
        if (m_verticalScale == LogScale) {
            return LogNumericalScale().getWidth(v, paint);
        } else {
            return LinearNumericalScale().getWidth(v, paint);
        }
    }
}

void
RegionLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect) const
{
    if (!m_model || m_model->getPoints().empty()) return;

    QString unit;
    double min, max;
    bool logarithmic;

    int w = getVerticalScaleWidth(v, false, paint);

    if (m_plotStyle == PlotSegmentation) {

        getValueExtents(min, max, logarithmic, unit);

        if (logarithmic) {
            LogRange::mapRange(min, max);
            LogColourScale().paintVertical(v, this, paint, 0, min, max);
        } else {
            LinearColourScale().paintVertical(v, this, paint, 0, min, max);
        }

    } else {

        getScaleExtents(v, min, max, logarithmic);

        if (logarithmic) {
            LogNumericalScale().paintVertical(v, this, paint, 0, min, max);
        } else {
            LinearNumericalScale().paintVertical(v, this, paint, 0, min, max);
        }
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
RegionLayer::drawStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double value = getValueForY(v, e->y());

    m_editingPoint = RegionModel::Point(frame, float(value), 0, "");
    m_originalPoint = m_editingPoint;

    if (m_editingCommand) finish(m_editingCommand);
    m_editingCommand = new RegionModel::EditCommand(m_model,
                                                    tr("Draw Region"));
    m_editingCommand->addPoint(m_editingPoint);

    recalcSpacing();

    m_editing = true;
}

void
RegionLayer::drawDrag(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    sv_frame_t frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    double newValue = m_editingPoint.value;
    if (m_verticalScale != EqualSpaced) newValue = getValueForY(v, e->y());

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

    recalcSpacing();
}

void
RegionLayer::drawEnd(View *, QMouseEvent *)
{
    if (!m_model || !m_editing) return;
    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;

    recalcSpacing();
}

void
RegionLayer::eraseStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) return;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
    recalcSpacing();
}

void
RegionLayer::eraseDrag(View *, QMouseEvent *)
{
}

void
RegionLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    RegionModel::Point p(0);
    if (!getPointToDrag(v, e->x(), e->y(), p)) return;
    if (p.frame != m_editingPoint.frame || p.value != m_editingPoint.value) return;

    m_editingCommand = new RegionModel::EditCommand
            (m_model, tr("Erase Region"));

    m_editingCommand->deletePoint(m_editingPoint);

    finish(m_editingCommand);
    m_editingCommand = 0;
    m_editing = false;
    recalcSpacing();
}

void
RegionLayer::editStart(View *v, QMouseEvent *e)
{
    if (!m_model) return;

    if (!getPointToDrag(v, e->x(), e->y(), m_editingPoint)) {
        return;
    }

    m_dragPointX = v->getXForFrame(m_editingPoint.frame);
    m_dragPointY = getYForValue(v, m_editingPoint.value);

    m_originalPoint = m_editingPoint;

    if (m_editingCommand) {
        finish(m_editingCommand);
        m_editingCommand = 0;
    }

    m_editing = true;
    m_dragStartX = e->x();
    m_dragStartY = e->y();
    recalcSpacing();
}

void
RegionLayer::editDrag(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    int xdist = e->x() - m_dragStartX;
    int ydist = e->y() - m_dragStartY;
    int newx = m_dragPointX + xdist;
    int newy = m_dragPointY + ydist;

    long frame = v->getFrameForX(newx);
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    // Do not bisect between two values, if one of those values is
    // that of the point we're actually moving ...
    int avoid = m_spacingMap[m_editingPoint.value];

    // ... unless there are other points with the same value
    if (m_distributionMap[m_editingPoint.value] > 1) avoid = -1;

    double value = getValueForY(v, newy, avoid);

    if (!m_editingCommand) {
        m_editingCommand = new RegionModel::EditCommand(m_model,
                                                        tr("Drag Region"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingPoint.value = float(value);
    m_editingCommand->addPoint(m_editingPoint);
    recalcSpacing();
}

void
RegionLayer::editEnd(View *, QMouseEvent *)
{
    if (!m_model || !m_editing) return;

    if (m_editingCommand) {

        QString newName = m_editingCommand->getName();

        if (m_editingPoint.frame != m_originalPoint.frame) {
            if (m_editingPoint.value != m_originalPoint.value) {
                newName = tr("Edit Region");
            } else {
                newName = tr("Relocate Region");
            }
        } else {
            newName = tr("Change Point Value");
        }

        m_editingCommand->setName(newName);
        finish(m_editingCommand);
    }

    m_editingCommand = 0;
    m_editing = false;
    recalcSpacing();
}

bool
RegionLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    RegionModel::Point region(0);
    if (!getPointToDrag(v, e->x(), e->y(), region)) return false;

    ItemEditDialog *dialog = new ItemEditDialog
            (m_model->getSampleRate(),
             ItemEditDialog::ShowTime |
             ItemEditDialog::ShowDuration |
             ItemEditDialog::ShowValue |
             ItemEditDialog::ShowText,
             getScaleUnits());

    dialog->setFrameTime(region.frame);
    dialog->setValue(region.value);
    dialog->setFrameDuration(region.duration);
    dialog->setText(region.label);

    if (dialog->exec() == QDialog::Accepted) {

        RegionModel::Point newRegion = region;
        newRegion.frame = dialog->getFrameTime();
        newRegion.value = dialog->getValue();
        newRegion.duration = dialog->getFrameDuration();
        newRegion.label = dialog->getText();
        
        RegionModel::EditCommand *command = new RegionModel::EditCommand
                (m_model, tr("Edit Region"));
        command->deletePoint(region);
        command->addPoint(newRegion);
        finish(command);
    }

    delete dialog;
    recalcSpacing();
    return true;
}

void
RegionLayer::moveSelection(Selection s, sv_frame_t newStartFrame)
{
    if (!m_model) return;

    RegionModel::EditCommand *command =
            new RegionModel::EditCommand(m_model, tr("Drag Selection"));

    RegionModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (RegionModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            RegionModel::Point newPoint(*i);
            newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
    recalcSpacing();
}

void
RegionLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    RegionModel::EditCommand *command =
            new RegionModel::EditCommand(m_model, tr("Resize Selection"));

    RegionModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
            double(newSize.getEndFrame() - newSize.getStartFrame()) /
            double(s.getEndFrame() - s.getStartFrame());

    for (RegionModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {

            double targetStart = double(i->frame);
            targetStart = double(newSize.getStartFrame()) +
                    targetStart - double(s.getStartFrame()) * ratio;

            double targetEnd = double(i->frame + i->duration);
            targetEnd = double(newSize.getStartFrame()) +
                    targetEnd - double(s.getStartFrame()) * ratio;

            RegionModel::Point newPoint(*i);
            newPoint.frame = lrint(targetStart);
            newPoint.duration = lrint(targetEnd - targetStart);
            command->deletePoint(*i);
            command->addPoint(newPoint);
        }
    }

    finish(command);
    recalcSpacing();
}

void
RegionLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    RegionModel::EditCommand *command =
            new RegionModel::EditCommand(m_model, tr("Delete Selected Points"));

    RegionModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (RegionModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {

        if (s.contains(i->frame)) {
            command->deletePoint(*i);
        }
    }

    finish(command);
    recalcSpacing();
}    

void
RegionLayer::copy(View *v, Selection s, Clipboard &to)
{
    if (!m_model) return;

    RegionModel::PointList points =
            m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (RegionModel::PointList::iterator i = points.begin();
         i != points.end(); ++i) {
        if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->value, i->duration, i->label);
            point.setReferenceFrame(alignToReference(v, i->frame));
            to.addPoint(point);
        }
    }
}

bool
RegionLayer::paste(View *v, const Clipboard &from, sv_frame_t /* frameOffset */, bool /* interactive */)
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

    RegionModel::EditCommand *command =
            new RegionModel::EditCommand(m_model, tr("Paste"));

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

        RegionModel::Point newPoint(frame);

        if (i->haveLabel()) newPoint.label = i->getLabel();
        if (i->haveValue()) newPoint.value = i->getValue();
        else newPoint.value = (m_model->getValueMinimum() +
                               m_model->getValueMaximum()) / 2;
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
    recalcSpacing();
    return true;
}

void
RegionLayer::toXml(QTextStream &stream,
                   QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes +
                             QString(" verticalScale=\"%1\" plotStyle=\"%2\"")
                             .arg(m_verticalScale)
                             .arg(m_plotStyle));
}

void
RegionLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);

    bool ok;
    VerticalScale scale = (VerticalScale)
            attributes.value("verticalScale").toInt(&ok);
    if (ok) setVerticalScale(scale);
    PlotStyle style = (PlotStyle)
            attributes.value("plotStyle").toInt(&ok);
    if (ok) setPlotStyle(style);
}


