/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Colour3DPlotLayer.h"

#include "view/View.h"
#include "base/Profiler.h"
#include "base/LogRange.h"
#include "base/RangeMapper.h"
#include "ColourMapper.h"
#include "LayerFactory.h"

#include <QPainter>
#include <QImage>
#include <QRect>
#include <QTextStream>

#include <iostream>

#include <cassert>

#ifndef __GNUC__
#include <alloca.h>
#endif

using std::vector;

//#define DEBUG_COLOUR_3D_PLOT_LAYER_PAINT 1

Colour3DPlotLayer::Colour3DPlotLayer() :
    m_model(0),
    m_cache(0),
    m_peaksCache(0),
    m_cacheValidStart(0),
    m_cacheValidEnd(0),
    m_colourScale(LinearScale),
    m_colourScaleSet(false),
    m_colourMap(0),
    m_gain(1.0),
    m_binScale(LinearBinScale),
    m_normalizeColumns(false),
    m_normalizeVisibleArea(false),
    m_normalizeHybrid(false),
    m_invertVertical(false),
    m_opaque(false),
    m_smooth(false),
    m_peakResolution(256),
    m_miny(0),
    m_maxy(0)
{
    
}

Colour3DPlotLayer::~Colour3DPlotLayer()
{
    delete m_cache;
    delete m_peaksCache;
}

bool Colour3DPlotLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<Colour3DPlotLayer, DenseThreeDimensionalModel>(this, model))
        return true;
    return false;
}

void
Colour3DPlotLayer::setModel(const DenseThreeDimensionalModel *model)
{
    if (m_model == model) return;
    const DenseThreeDimensionalModel *oldModel = m_model;
    m_model = model;
    if (!m_model || !m_model->isOK()) return;

    connectSignals(m_model);

    connect(m_model, SIGNAL(modelChanged()), this, SLOT(modelChanged()));
    connect(m_model, SIGNAL(modelChangedWithin(sv_frame_t, sv_frame_t)),
            this, SLOT(modelChangedWithin(sv_frame_t, sv_frame_t)));

    m_peakResolution = 256;
    if (model->getResolution() > 512) {
        m_peakResolution = 16;
    } else if (model->getResolution() > 128) {
        m_peakResolution = 64;
    } else if (model->getResolution() > 2) {
        m_peakResolution = 128;
    }
    cacheInvalid();

    emit modelReplaced();
    emit sliceableModelReplaced(oldModel, model);
}

void
Colour3DPlotLayer::cacheInvalid()
{
    delete m_cache;
    delete m_peaksCache;
    m_cache = 0;
    m_peaksCache = 0;
    m_cacheValidStart = 0;
    m_cacheValidEnd = 0;
}

void
Colour3DPlotLayer::cacheInvalid(sv_frame_t startFrame, sv_frame_t endFrame)
{
    if (!m_cache || !m_model) return;

    int modelResolution = m_model->getResolution();
    int start = int(startFrame / modelResolution);
    int end = int(endFrame / modelResolution + 1);
    if (m_cacheValidStart < end) m_cacheValidStart = end;
    if (m_cacheValidEnd > start) m_cacheValidEnd = start;
    if (m_cacheValidStart > m_cacheValidEnd) m_cacheValidEnd = m_cacheValidStart;
}

void
Colour3DPlotLayer::modelChanged()
{
    if (!m_colourScaleSet && m_colourScale == LinearScale) {
        if (m_model) {
            if (m_model->shouldUseLogValueScale()) {
                setColourScale(LogScale);
            } else {
                m_colourScaleSet = true;
            }
        }
    }
    cacheInvalid();
}

void
Colour3DPlotLayer::modelChangedWithin(sv_frame_t startFrame, sv_frame_t endFrame)
{
    if (!m_colourScaleSet && m_colourScale == LinearScale) {
        if (m_model && m_model->getWidth() > 50) {
            if (m_model->shouldUseLogValueScale()) {
                setColourScale(LogScale);
            } else {
                m_colourScaleSet = true;
            }
        }
    }
    cacheInvalid(startFrame, endFrame);
}

Layer::PropertyList
Colour3DPlotLayer::getProperties() const
{
    PropertyList list;
    list.push_back("Colour");
    list.push_back("Colour Scale");
    list.push_back("Normalize Columns");
    list.push_back("Normalize Visible Area");
    list.push_back("Gain");
    list.push_back("Bin Scale");
    list.push_back("Invert Vertical Scale");
    list.push_back("Opaque");
    list.push_back("Smooth");
    return list;
}

QString
Colour3DPlotLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Colour") return tr("Colour");
    if (name == "Colour Scale") return tr("Scale");
    if (name == "Normalize Columns") return tr("Normalize Columns");
    if (name == "Normalize Visible Area") return tr("Normalize Visible Area");
    if (name == "Invert Vertical Scale") return tr("Invert Vertical Scale");
    if (name == "Gain") return tr("Gain");
    if (name == "Opaque") return tr("Always Opaque");
    if (name == "Smooth") return tr("Smooth");
    if (name == "Bin Scale") return tr("Bin Scale");
    return "";
}

QString
Colour3DPlotLayer::getPropertyIconName(const PropertyName &name) const
{
    if (name == "Normalize Columns") return "normalise-columns";
    if (name == "Normalize Visible Area") return "normalise";
    if (name == "Invert Vertical Scale") return "invert-vertical";
    if (name == "Opaque") return "opaque";
    if (name == "Smooth") return "smooth";
    return "";
}

Layer::PropertyType
Colour3DPlotLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Gain") return RangeProperty;
    if (name == "Normalize Columns") return ToggleProperty;
    if (name == "Normalize Visible Area") return ToggleProperty;
    if (name == "Invert Vertical Scale") return ToggleProperty;
    if (name == "Opaque") return ToggleProperty;
    if (name == "Smooth") return ToggleProperty;
    return ValueProperty;
}

QString
Colour3DPlotLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Normalize Columns" ||
            name == "Normalize Visible Area" ||
            name == "Colour Scale" ||
            name == "Gain") return tr("Scale");
    if (name == "Bin Scale" ||
            name == "Invert Vertical Scale") return tr("Bins");
    if (name == "Opaque" ||
            name == "Smooth" ||
            name == "Colour") return tr("Colour");
    return QString();
}

int
Colour3DPlotLayer::getPropertyRangeAndValue(const PropertyName &name,
                                            int *min, int *max, int *deflt) const
{
    int val = 0;

    int garbage0, garbage1, garbage2;
    if (!min) min = &garbage0;
    if (!max) max = &garbage1;
    if (!deflt) deflt = &garbage2;

    if (name == "Gain") {

        *min = -50;
        *max = 50;

        *deflt = int(lrint(log10(1.0) * 20.0));
        if (*deflt < *min) *deflt = *min;
        if (*deflt > *max) *deflt = *max;

        val = int(lrint(log10(m_gain) * 20.0));
        if (val < *min) val = *min;
        if (val > *max) val = *max;

    } else if (name == "Colour Scale") {

        *min = 0;
        *max = 3;
        *deflt = (int)LinearScale;

        val = (int)m_colourScale;

    } else if (name == "Colour") {

        *min = 0;
        *max = ColourMapper::getColourMapCount() - 1;
        *deflt = 0;

        val = m_colourMap;

    } else if (name == "Normalize Columns") {

        *deflt = 0;
        val = (m_normalizeColumns ? 1 : 0);

    } else if (name == "Normalize Visible Area") {

        *deflt = 0;
        val = (m_normalizeVisibleArea ? 1 : 0);

    } else if (name == "Invert Vertical Scale") {

        *deflt = 0;
        val = (m_invertVertical ? 1 : 0);

    } else if (name == "Bin Scale") {

        *min = 0;
        *max = 1;
        *deflt = int(LinearBinScale);
        val = (int)m_binScale;

    } else if (name == "Opaque") {

        *deflt = 0;
        val = (m_opaque ? 1 : 0);
        
    } else if (name == "Smooth") {

        *deflt = 0;
        val = (m_smooth ? 1 : 0);
        
    } else {
        val = Layer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
Colour3DPlotLayer::getPropertyValueLabel(const PropertyName &name,
                                         int value) const
{
    if (name == "Colour") {
        return ColourMapper::getColourMapName(value);
    }
    if (name == "Colour Scale") {
        switch (value) {
        default:
        case 0: return tr("Linear");
        case 1: return tr("Log");
        case 2: return tr("+/-1");
        case 3: return tr("Absolute");
        }
    }
    if (name == "Bin Scale") {
        switch (value) {
        default:
        case 0: return tr("Linear");
        case 1: return tr("Log");
        }
    }
    return tr("<unknown>");
}

RangeMapper *
Colour3DPlotLayer::getNewPropertyRangeMapper(const PropertyName &name) const
{
    if (name == "Gain") {
        return new LinearRangeMapper(-50, 50, -25, 25, tr("dB"));
    }
    return 0;
}

void
Colour3DPlotLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Gain") {
        setGain(float(pow(10, value/20.0)));
    } else if (name == "Colour Scale") {
        switch (value) {
        default:
        case 0: setColourScale(LinearScale); break;
        case 1: setColourScale(LogScale); break;
        case 2: setColourScale(PlusMinusOneScale); break;
        case 3: setColourScale(AbsoluteScale); break;
        }
    } else if (name == "Colour") {
        setColourMap(value);
    } else if (name == "Normalize Columns") {
        setNormalizeColumns(value ? true : false);
    } else if (name == "Normalize Visible Area") {
        setNormalizeVisibleArea(value ? true : false);
    } else if (name == "Invert Vertical Scale") {
        setInvertVertical(value ? true : false);
    } else if (name == "Opaque") {
        setOpaque(value ? true : false);
    } else if (name == "Smooth") {
        setSmooth(value ? true : false);
    } else if (name == "Bin Scale") {
        switch (value) {
        default:
        case 0: setBinScale(LinearBinScale); break;
        case 1: setBinScale(LogBinScale); break;
        }
    }
}

void
Colour3DPlotLayer::setColourScale(ColourScale scale)
{
    if (m_colourScale == scale) return;
    m_colourScale = scale;
    m_colourScaleSet = true;
    cacheInvalid();
    emit layerParametersChanged();
}

void
Colour3DPlotLayer::setColourMap(int map)
{
    if (m_colourMap == map) return;
    m_colourMap = map;
    cacheInvalid();
    emit layerParametersChanged();
}

void
Colour3DPlotLayer::setGain(float gain)
{
    if (m_gain == gain) return;
    m_gain = gain;
    cacheInvalid();
    emit layerParametersChanged();
}

float
Colour3DPlotLayer::getGain() const
{
    return m_gain;
}

void
Colour3DPlotLayer::setBinScale(BinScale binScale)
{
    if (m_binScale == binScale) return;
    m_binScale = binScale;
    cacheInvalid();
    emit layerParametersChanged();
}

Colour3DPlotLayer::BinScale
Colour3DPlotLayer::getBinScale() const
{
    return m_binScale;
}

void
Colour3DPlotLayer::setNormalizeColumns(bool n)
{
    if (m_normalizeColumns == n) return;
    m_normalizeColumns = n;
    cacheInvalid();
    emit layerParametersChanged();
}

bool
Colour3DPlotLayer::getNormalizeColumns() const
{
    return m_normalizeColumns;
}

void
Colour3DPlotLayer::setNormalizeHybrid(bool n)
{
    if (m_normalizeHybrid == n) return;
    m_normalizeHybrid = n;
    cacheInvalid();
    emit layerParametersChanged();
}

bool
Colour3DPlotLayer::getNormalizeHybrid() const
{
    return m_normalizeHybrid;
}

void
Colour3DPlotLayer::setNormalizeVisibleArea(bool n)
{
    if (m_normalizeVisibleArea == n) return;
    m_normalizeVisibleArea = n;
    cacheInvalid();
    emit layerParametersChanged();
}

bool
Colour3DPlotLayer::getNormalizeVisibleArea() const
{
    return m_normalizeVisibleArea;
}

void
Colour3DPlotLayer::setInvertVertical(bool n)
{
    if (m_invertVertical == n) return;
    m_invertVertical = n;
    cacheInvalid();
    emit layerParametersChanged();
}

void
Colour3DPlotLayer::setOpaque(bool n)
{
    if (m_opaque == n) return;
    m_opaque = n;
    emit layerParametersChanged();
}

void
Colour3DPlotLayer::setSmooth(bool n)
{
    if (m_smooth == n) return;
    m_smooth = n;
    emit layerParametersChanged();
}

bool
Colour3DPlotLayer::getInvertVertical() const
{
    return m_invertVertical;
}

bool
Colour3DPlotLayer::getOpaque() const
{
    return m_opaque;
}

bool
Colour3DPlotLayer::getSmooth() const
{
    return m_smooth;
}

void
Colour3DPlotLayer::setLayerDormant(const View *v, bool dormant)
{
    if (dormant) {

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "Colour3DPlotLayer::setLayerDormant(" << dormant << ")"
             << endl;
#endif

        if (isLayerDormant(v)) {
            return;
        }

        Layer::setLayerDormant(v, true);

        cacheInvalid();

    } else {

        Layer::setLayerDormant(v, false);
    }
}

bool
Colour3DPlotLayer::isLayerScrollable(const View *v) const
{
    if (m_normalizeVisibleArea) {
        return false;
    }
    if (shouldPaintDenseIn(v)) {
        return true;
    }
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

bool
Colour3DPlotLayer::getValueExtents(double &min, double &max,
                                   bool &logarithmic, QString &unit) const
{
    if (!m_model) return false;

    min = 0;
    max = double(m_model->getHeight());

    logarithmic = false;
    unit = "";

    return true;
}

bool
Colour3DPlotLayer::getDisplayExtents(double &min, double &max) const
{
    if (!m_model) return false;

    double hmax = double(m_model->getHeight());
    
    min = m_miny;
    max = m_maxy;
    if (max <= min) {
        min = 0;
        max = hmax;
    }
    if (min < 0) min = 0;
    if (max > hmax) max = hmax;

    return true;
}

bool
Colour3DPlotLayer::setDisplayExtents(double min, double max)
{
    if (!m_model) return false;

    m_miny = int(lrint(min));
    m_maxy = int(lrint(max));
    
    emit layerParametersChanged();
    return true;
}

bool
Colour3DPlotLayer::getYScaleValue(const View *, int,
                                  double &, QString &) const
{
    return false;//!!!
}

int
Colour3DPlotLayer::getVerticalZoomSteps(int &defaultStep) const
{
    if (!m_model) return 0;

    defaultStep = 0;
    int h = m_model->getHeight();
    return h;
}

int
Colour3DPlotLayer::getCurrentVerticalZoomStep() const
{
    if (!m_model) return 0;

    double min, max;
    getDisplayExtents(min, max);
    return m_model->getHeight() - int(lrint(max - min));
}

void
Colour3DPlotLayer::setVerticalZoomStep(int step)
{
    if (!m_model) return;

    //    cerr << "Colour3DPlotLayer::setVerticalZoomStep(" <<step <<"): before: miny = " << m_miny << ", maxy = " << m_maxy << endl;

    int dist = m_model->getHeight() - step;
    if (dist < 1) dist = 1;
    double centre = m_miny + (m_maxy - m_miny) / 2.0;
    m_miny = int(lrint(centre - dist/2.0));
    if (m_miny < 0) m_miny = 0;
    m_maxy = m_miny + dist;
    if (m_maxy > m_model->getHeight()) m_maxy = m_model->getHeight();

    //    cerr << "Colour3DPlotLayer::setVerticalZoomStep(" <<step <<"):  after: miny = " << m_miny << ", maxy = " << m_maxy << endl;
    
    emit layerParametersChanged();
}

RangeMapper *
Colour3DPlotLayer::getNewVerticalZoomRangeMapper() const
{
    if (!m_model) return 0;

    return new LinearRangeMapper(0, m_model->getHeight(),
                                 0, m_model->getHeight(), "");
}

double
Colour3DPlotLayer::getYForBin(View *v, double bin) const
{
    double y = bin;
    if (!m_model) return y;
    double mn = 0, mx = m_model->getHeight();
    getDisplayExtents(mn, mx);
    double h = v->height();
    if (m_binScale == LinearBinScale) {
        y = h - (((bin - mn) * h) / (mx - mn));
    } else {
        double logmin = mn + 1, logmax = mx + 1;
        LogRange::mapRange(logmin, logmax);
        y = h - (((LogRange::map(bin + 1) - logmin) * h) / (logmax - logmin));
    }
    return y;
}

int
Colour3DPlotLayer::getIYForBin(View *v, int bin) const
{
    return int(round(getYForBin(v, bin)));
}

double
Colour3DPlotLayer::getBinForY(View *v, double y) const
{
    double bin = y;
    if (!m_model) return bin;
    double mn = 0, mx = m_model->getHeight();
    getDisplayExtents(mn, mx);
    double h = v->height();
    if (m_binScale == LinearBinScale) {
        bin = mn + ((h - y) * (mx - mn)) / h;
    } else {
        double logmin = mn + 1, logmax = mx + 1;
        LogRange::mapRange(logmin, logmax);
        bin = LogRange::unmap(logmin + ((h - y) * (logmax - logmin)) / h) - 1;
    }
    return bin;
}

int
Colour3DPlotLayer::getIBinForY(View *v, int y) const
{
    return int(floor(getBinForY(v, y)));
}

QString
Colour3DPlotLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    if (!m_model) return "";

    int x = pos.x();
    int y = pos.y();

    sv_frame_t modelStart = m_model->getStartFrame();
    int modelResolution = m_model->getResolution();

    double srRatio =
            v->getViewManager()->getMainModelSampleRate() /
            m_model->getSampleRate();

    int sx0 = int((double(v->getFrameForX(x)) / srRatio - double(modelStart)) /
                  modelResolution);

    int f0 = sx0 * modelResolution;
    int f1 =  f0 + modelResolution;

    int sh = m_model->getHeight();

    int symin = m_miny;
    int symax = m_maxy;
    if (symax <= symin) {
        symin = 0;
        symax = sh;
    }
    if (symin < 0) symin = 0;
    if (symax > sh) symax = sh;

    //    double binHeight = double(v->height()) / (symax - symin);
    //    int sy = int((v->height() - y) / binHeight) + symin;

    int sy = getIBinForY(v, y);

    if (sy < 0 || sy >= m_model->getHeight()) {
        return "";
    }

    if (m_invertVertical) sy = m_model->getHeight() - sy - 1;

    float value = m_model->getValueAt(sx0, sy);

    //    cerr << "bin value (" << sx0 << "," << sy << ") is " << value << endl;
    
    QString binName = m_model->getBinName(sy);
    if (binName == "") binName = QString("[%1]").arg(sy + 1);
    else binName = QString("%1 [%2]").arg(binName).arg(sy + 1);

    QString text = tr("Time:\t%1 - %2\nBin:\t%3\nValue:\t%4")
            .arg(RealTime::frame2RealTime(f0, m_model->getSampleRate())
                 .toText(true).c_str())
            .arg(RealTime::frame2RealTime(f1, m_model->getSampleRate())
                 .toText(true).c_str())
            .arg(binName)
            .arg(value);

    return text;
}

int
Colour3DPlotLayer::getColourScaleWidth(QPainter &) const
{
    int cw = 20;
    return cw;
}

int
Colour3DPlotLayer::getVerticalScaleWidth(View *, bool, QPainter &paint) const
{
    if (!m_model) return 0;

    QString sampleText = QString("[%1]").arg(m_model->getHeight());
    int tw = paint.fontMetrics().width(sampleText);
    bool another = false;

    for (int i = 0; i < m_model->getHeight(); ++i) {
        if (m_model->getBinName(i).length() > sampleText.length()) {
            sampleText = m_model->getBinName(i);
            another = true;
        }
    }
    if (another) {
        tw = std::max(tw, paint.fontMetrics().width(sampleText));
    }

    return tw + 13 + getColourScaleWidth(paint);
}

void
Colour3DPlotLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const
{
    if (!m_model) return;

    int h = rect.height(), w = rect.width();

    int cw = getColourScaleWidth(paint);
    
    int ch = h - 20;
    if (ch > 20 && m_cache) {

        double min = m_model->getMinimumLevel();
        double max = m_model->getMaximumLevel();

        double mmin = min;
        double mmax = max;

        if (m_colourScale == LogScale) {
            LogRange::mapRange(mmin, mmax);
        } else if (m_colourScale == PlusMinusOneScale) {
            mmin = -1.f;
            mmax = 1.f;
        } else if (m_colourScale == AbsoluteScale) {
            if (mmin < 0) {
                if (fabs(mmin) > fabs(mmax)) mmax = fabs(mmin);
                else mmax = fabs(mmax);
                mmin = 0;
            } else {
                mmin = fabs(mmin);
                mmax = fabs(mmax);
            }
        }

        if (max == min) max = min + 1.f;
        if (mmax == mmin) mmax = mmin + 1.f;

        paint.setPen(v->getForeground());
        paint.drawRect(4, 10, cw - 8, ch+1);

        for (int y = 0; y < ch; ++y) {
            double value = ((max - min) * (double(ch-y) - 1.0)) / double(ch) + min;
            if (m_colourScale == LogScale) {
                value = LogRange::map(value);
            }
            int pixel = int(((value - mmin) * 256) / (mmax - mmin));
            if (pixel >= 0 && pixel < 256) {
                QRgb c = m_cache->color(pixel);
                paint.setPen(QColor(qRed(c), qGreen(c), qBlue(c)));
                paint.drawLine(5, 11 + y, cw - 5, 11 + y);
            } else {
                cerr << "WARNING: Colour3DPlotLayer::paintVerticalScale: value " << value << ", mmin " << mmin << ", mmax " << mmax << " leads to invalid pixel " << pixel << endl;
            }
        }

        QString minstr = QString("%1").arg(min);
        QString maxstr = QString("%1").arg(max);
        
        paint.save();

        QFont font = paint.font();
        font.setPixelSize(10);
        paint.setFont(font);

        int msw = paint.fontMetrics().width(maxstr);

        QMatrix m;
        m.translate(cw - 6, ch + 10);
        m.rotate(-90);

        paint.setWorldMatrix(m);

        v->drawVisibleText(paint, 2, 0, minstr, View::OutlinedText);

        m.translate(ch - msw - 2, 0);
        paint.setWorldMatrix(m);

        v->drawVisibleText(paint, 0, 0, maxstr, View::OutlinedText);

        paint.restore();
    }

    paint.setPen(v->getForeground());

    int sh = m_model->getHeight();

    int symin = m_miny;
    int symax = m_maxy;
    if (symax <= symin) {
        symin = 0;
        symax = sh;
    }
    if (symin < 0) symin = 0;
    if (symax > sh) symax = sh;

    paint.save();

    int py = h;

    for (int i = symin; i <= symax; ++i) {

        int y0;

        y0 = getIYForBin(v, i);
        int h = py - y0;

        if (i > symin) {
            if (paint.fontMetrics().height() >= h) {
                if (h >= 8) {
                    QFont tf = paint.font();
                    tf.setPixelSize(h-2);
                    paint.setFont(tf);
                } else {
                    continue;
                }
            }
        }

        py = y0;

        if (i < symax) {
            paint.drawLine(cw, y0, w, y0);
        }

        if (i > symin) {

            int idx = i - 1;
            if (m_invertVertical) idx = m_model->getHeight() - idx - 1;

            QString text = m_model->getBinName(idx);
            if (text == "") text = QString("[%1]").arg(idx + 1);

            int ty = y0 + (h/2) - (paint.fontMetrics().height()/2) +
                    paint.fontMetrics().ascent() + 1;

            paint.drawText(cw + 5, ty, text);
        }
    }

    paint.restore();
}

DenseThreeDimensionalModel::Column
Colour3DPlotLayer::getColumn(int col) const
{
    Profiler profiler("Colour3DPlotLayer::getColumn");

    DenseThreeDimensionalModel::Column values = m_model->getColumn(col);
    while (values.size() < m_model->getHeight()) values.push_back(0.f);
    if (!m_normalizeColumns && !m_normalizeHybrid) return values;

    double colMax = 0.f, colMin = 0.f;
    double min = 0.f, max = 0.f;

    min = m_model->getMinimumLevel();
    max = m_model->getMaximumLevel();

    for (int y = 0; y < values.size(); ++y) {
        if (y == 0 || values.at(y) > colMax) colMax = values.at(y);
        if (y == 0 || values.at(y) < colMin) colMin = values.at(y);
    }
    if (colMin == colMax) colMax = colMin + 1;
    
    for (int y = 0; y < values.size(); ++y) {

        double value = values.at(y);
        double norm = (value - colMin) / (colMax - colMin);
        double newvalue = min + (max - min) * norm;

        if (value != newvalue) values[y] = float(newvalue);
    }

    if (m_normalizeHybrid && (colMax > 0.0)) {
        double logmax = log10(colMax);
        for (int y = 0; y < values.size(); ++y) {
            values[y] = float(values[y] * logmax);
        }
    }

    return values;
}

void
Colour3DPlotLayer::fillCache(int firstBin, int lastBin) const
{
    Profiler profiler("Colour3DPlotLayer::fillCache", true);

    sv_frame_t modelStart = m_model->getStartFrame();
    sv_frame_t modelEnd = m_model->getEndFrame();
    int modelResolution = m_model->getResolution();

    int modelStartBin = int(modelStart / modelResolution);
    int modelEndBin = int(modelEnd / modelResolution);

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "Colour3DPlotLayer::fillCache: range " << firstBin << " -> " << lastBin << " of model range " << modelStartBin << " -> " << modelEndBin << " (model resolution " << modelResolution << ")" << endl;
#endif

    int cacheWidth = modelEndBin - modelStartBin + 1;
    if (lastBin > modelEndBin) cacheWidth = lastBin - modelStartBin + 1;
    int cacheHeight = m_model->getHeight();

    if (m_cache && m_cache->height() != cacheHeight) {
        // height has changed: delete everything rather than resizing
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "Colour3DPlotLayer::fillCache: Cache height has changed, recreating" << endl;
#endif
        delete m_cache;
        delete m_peaksCache;
        m_cache = 0;
        m_peaksCache = 0;
    }

    if (m_cache && m_cache->width() != cacheWidth) {
        // width has changed and we have an existing cache: resize it
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "Colour3DPlotLayer::fillCache: Cache width has changed, resizing existing cache" << endl;
#endif
        QImage *newCache =
                new QImage(m_cache->copy(0, 0, cacheWidth, cacheHeight));
        delete m_cache;
        m_cache = newCache;
        if (m_peaksCache) {
            QImage *newPeaksCache =
                    new QImage(m_peaksCache->copy
                               (0, 0, cacheWidth / m_peakResolution + 1, cacheHeight));
            delete m_peaksCache;
            m_peaksCache = newPeaksCache;
        }
    }

    if (!m_cache) {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "Colour3DPlotLayer::fillCache: Have no cache, making one" << endl;
#endif
        m_cache = new QImage
                (cacheWidth, cacheHeight, QImage::Format_Indexed8);
        m_cache->setColorCount(256);
        m_cache->fill(0);
        if (!m_normalizeVisibleArea) {
            m_peaksCache = new QImage
                    (cacheWidth / m_peakResolution + 1, cacheHeight,
                     QImage::Format_Indexed8);
            m_peaksCache->setColorCount(256);
            m_peaksCache->fill(0);
        } else if (m_peaksCache) {
            delete m_peaksCache;
            m_peaksCache = 0;
        }
        m_cacheValidStart = 0;
        m_cacheValidEnd = 0;
    }

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "cache size = " << m_cache->width() << "x" << m_cache->height()
         << " peaks cache size = " << m_peaksCache->width() << "x" << m_peaksCache->height() << endl;
#endif

    if (m_cacheValidStart <= firstBin && m_cacheValidEnd >= lastBin) {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "Cache is valid in this region already" << endl;
#endif
        return;
    }
    
    int fillStart = firstBin;
    int fillEnd = lastBin;

    if (fillStart < modelStartBin) fillStart = modelStartBin;
    if (fillStart > modelEndBin) fillStart = modelEndBin;
    if (fillEnd < modelStartBin) fillEnd = modelStartBin;
    if (fillEnd > modelEndBin) fillEnd = modelEndBin;

    bool normalizeVisible = (m_normalizeVisibleArea && !m_normalizeColumns);

    if (!normalizeVisible && (m_cacheValidStart < m_cacheValidEnd)) {
        
        if (m_cacheValidEnd < fillStart) {
            fillStart = m_cacheValidEnd + 1;
        }
        if (m_cacheValidStart > fillEnd) {
            fillEnd = m_cacheValidStart - 1;
        }
        
        m_cacheValidStart = std::min(fillStart, m_cacheValidStart);
        m_cacheValidEnd = std::max(fillEnd, m_cacheValidEnd);

    } else {

        // when normalising the visible area, the only valid area,
        // ever, is the currently visible one

        m_cacheValidStart = fillStart;
        m_cacheValidEnd = fillEnd;
    }

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "Cache size " << cacheWidth << "x" << cacheHeight << " will be valid from " << m_cacheValidStart << " to " << m_cacheValidEnd << " (fillStart = " << fillStart << ", fillEnd = " << fillEnd << ")" << endl;
#endif

    DenseThreeDimensionalModel::Column values;

    double min = m_model->getMinimumLevel();
    double max = m_model->getMaximumLevel();

    if (m_colourScale == LogScale) {
        LogRange::mapRange(min, max);
    } else if (m_colourScale == PlusMinusOneScale) {
        min = -1.f;
        max = 1.f;
    } else if (m_colourScale == AbsoluteScale) {
        if (min < 0) {
            if (fabs(min) > fabs(max)) max = fabs(min);
            else max = fabs(max);
            min = 0;
        } else {
            min = fabs(min);
            max = fabs(max);
        }
    }
    
    if (max == min) max = min + 1.f;
    
    ColourMapper mapper(m_colourMap, 0.f, 255.f);
    
    for (int index = 0; index < 256; ++index) {
        QColor colour = mapper.map(index);
        m_cache->setColor
                (index, qRgb(colour.red(), colour.green(), colour.blue()));
        if (m_peaksCache) {
            m_peaksCache->setColor
                    (index, qRgb(colour.red(), colour.green(), colour.blue()));
        }
    }
    
    double visibleMax = 0.f, visibleMin = 0.f;

    if (normalizeVisible) {
        
        for (int c = fillStart; c <= fillEnd; ++c) {

            values = getColumn(c);

            double colMax = 0.f, colMin = 0.f;

            for (int y = 0; y < cacheHeight; ++y) {
                if (y >= values.size()) break;
                if (y == 0 || values[y] > colMax) colMax = values[y];
                if (y == 0 || values[y] < colMin) colMin = values[y];
            }

            if (c == fillStart || colMax > visibleMax) visibleMax = colMax;
            if (c == fillStart || colMin < visibleMin) visibleMin = colMin;
        }

        if (m_colourScale == LogScale) {
            visibleMin = LogRange::map(visibleMin);
            visibleMax = LogRange::map(visibleMax);
            if (visibleMin > visibleMax) std::swap(visibleMin, visibleMax);
        } else if (m_colourScale == AbsoluteScale) {
            if (visibleMin < 0) {
                if (fabs(visibleMin) > fabs(visibleMax)) visibleMax = fabs(visibleMin);
                else visibleMax = fabs(visibleMax);
                visibleMin = 0;
            } else {
                visibleMin = fabs(visibleMin);
                visibleMax = fabs(visibleMax);
            }
        }
    }
    
    if (visibleMin == visibleMax) visibleMax = visibleMin + 1;

    int *peaks = 0;
    if (m_peaksCache) {
        peaks = new int[cacheHeight];
        for (int y = 0; y < cacheHeight; ++y) {
            peaks[y] = 0;
        }
    }

    Profiler profiler2("Colour3DPlotLayer::fillCache: filling", true);

    for (int c = fillStart; c <= fillEnd; ++c) {

        values = getColumn(c);

        if (c >= m_cache->width()) {
            cerr << "ERROR: column " << c << " >= cache width "
                 << m_cache->width() << endl;
            continue;
        }

        for (int y = 0; y < cacheHeight; ++y) {

            double value = min;
            if (y < values.size()) {
                value = values.at(y);
            }

            value = value * m_gain;

            if (m_colourScale == LogScale) {
                value = LogRange::map(value);
            } else if (m_colourScale == AbsoluteScale) {
                value = fabs(value);
            }
            
            if (normalizeVisible) {
                double norm = (value - visibleMin) / (visibleMax - visibleMin);
                value = min + (max - min) * norm;
            }

            int pixel = int(((value - min) * 256) / (max - min));
            if (pixel < 0) pixel = 0;
            if (pixel > 255) pixel = 255;
            if (peaks && (pixel > peaks[y])) peaks[y] = pixel;

            if (m_invertVertical) {
                m_cache->setPixel(c, cacheHeight - y - 1, pixel);
            } else {
                if (y >= m_cache->height()) {
                    cerr << "ERROR: row " << y << " >= cache height " << m_cache->height() << endl;
                } else {
                    m_cache->setPixel(c, y, pixel);
                }
            }
        }

        if (peaks) {
            int notch = (c % m_peakResolution);
            if (notch == m_peakResolution-1 || c == fillEnd) {
                int pc = c / m_peakResolution;
                if (pc >= m_peaksCache->width()) {
                    cerr << "ERROR: peak column " << pc
                         << " (from col " << c << ") >= peaks cache width "
                         << m_peaksCache->width() << endl;
                    continue;
                }
                for (int y = 0; y < cacheHeight; ++y) {
                    if (m_invertVertical) {
                        m_peaksCache->setPixel(pc, cacheHeight - y - 1, peaks[y]);
                    } else {
                        if (y >= m_peaksCache->height()) {
                            cerr << "ERROR: row " << y
                                 << " >= peaks cache height "
                                 << m_peaksCache->height() << endl;
                        } else {
                            m_peaksCache->setPixel(pc, y, peaks[y]);
                        }
                    }
                }
                for (int y = 0; y < cacheHeight; ++y) {
                    peaks[y] = 0;
                }
            }
        }
    }

    delete[] peaks;
}

bool
Colour3DPlotLayer::shouldPaintDenseIn(const View *v) const
{
    if (!m_model || !v || !(v->getViewManager())) {
        return false;
    }
    double srRatio =
            v->getViewManager()->getMainModelSampleRate() / m_model->getSampleRate();
    if (m_opaque ||
            m_smooth ||
            m_model->getHeight() >= v->height() ||
            ((m_model->getResolution() * srRatio) / v->getZoomLevel()) < 2) {
        return true;
    }
    return false;
}

void
Colour3DPlotLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    /*
    if (m_model) {
        cerr << "Colour3DPlotLayer::paint: model says shouldUseLogValueScale = " << m_model->shouldUseLogValueScale() << endl;
    }
*/
    Profiler profiler("Colour3DPlotLayer::paint");
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "Colour3DPlotLayer::paint(): m_model is " << m_model << ", zoom level is " << v->getZoomLevel() << ", rect is (" << rect.x() << "," << rect.y() << ") " << rect.width() << "x" << rect.height() << endl;
#endif

    int completion = 0;
    if (!m_model || !m_model->isOK() || !m_model->isReady(&completion)) {
        if (completion > 0) {
            paint.fillRect(0, 10, v->width() * completion / 100,
                           10, QColor(120, 120, 120));
        }
        return;
    }

    if (m_normalizeVisibleArea && !m_normalizeColumns) rect = v->rect();

    sv_frame_t modelStart = m_model->getStartFrame();
    sv_frame_t modelEnd = m_model->getEndFrame();
    int modelResolution = m_model->getResolution();

    // The cache is from the model's start frame to the model's end
    // frame at the model's window increment frames per pixel.  We
    // want to draw from our start frame + x0 * zoomLevel to our start
    // frame + x1 * zoomLevel at zoomLevel frames per pixel.

    //  We have quite different paint mechanisms for rendering "large"
    //  bins (more than one bin per pixel in both directions) and
    //  "small".  This is "large"; see paintDense below for "small".

    int x0 = rect.left();
    int x1 = rect.right() + 1;

    int h = v->height();

    double srRatio =
            v->getViewManager()->getMainModelSampleRate() / m_model->getSampleRate();

    int sx0 = int((double(v->getFrameForX(x0)) / srRatio - double(modelStart))
                  / modelResolution);
    int sx1 = int((double(v->getFrameForX(x1)) / srRatio - double(modelStart))
                  / modelResolution);
    int sh = m_model->getHeight();

    int symin = m_miny;
    int symax = m_maxy;
    if (symax <= symin) {
        symin = 0;
        symax = sh;
    }
    if (symin < 0) symin = 0;
    if (symax > sh) symax = sh;

    if (sx0 > 0) --sx0;
    fillCache(sx0 < 0 ? 0 : sx0,
              sx1 < 0 ? 0 : sx1);

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "Colour3DPlotLayer::paint: height = "<< m_model->getHeight() << ", modelStart = " << modelStart << ", resolution = " << modelResolution << ", model rate = " << m_model->getSampleRate() << " (zoom level = " << v->getZoomLevel() << ", srRatio = " << srRatio << ")" << endl;
#endif

    if (shouldPaintDenseIn(v)) {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
        cerr << "calling paintDense" << endl;
#endif
        paintDense(v, paint, rect);
        return;
    }

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "Colour3DPlotLayer::paint: w " << x1-x0 << ", h " << h << ", sx0 " << sx0 << ", sx1 " << sx1 << ", sw " << sx1-sx0 << ", sh " << sh << endl;
    cerr << "Colour3DPlotLayer: sample rate is " << m_model->getSampleRate() << ", resolution " << m_model->getResolution() << endl;
#endif

    QPoint illuminatePos;
    bool illuminate = v->shouldIlluminateLocalFeatures(this, illuminatePos);
    
    const int buflen = 40;
    char labelbuf[buflen];

    for (int sx = sx0; sx <= sx1; ++sx) {

        sv_frame_t fx = sx * modelResolution;

        if (fx + modelResolution <= modelStart || fx > modelEnd) continue;

        int rx0 = v->getXForFrame(int(double(fx + modelStart) * srRatio));
        int rx1 = v->getXForFrame(int(double(fx + modelStart + modelResolution + 1) * srRatio));

        int rw = rx1 - rx0;
        if (rw < 1) rw = 1;

        bool showLabel = (rw > 10 &&
                          paint.fontMetrics().width("0.000000") < rw - 3 &&
                          paint.fontMetrics().height() < (h / sh));
        
        for (int sy = symin; sy < symax; ++sy) {

            int ry0 = getIYForBin(v, sy);
            int ry1 = getIYForBin(v, sy + 1);
            QRect r(rx0, ry1, rw, ry0 - ry1);

            QRgb pixel = qRgb(255, 255, 255);
            if (sx >= 0 && sx < m_cache->width() &&
                    sy >= 0 && sy < m_cache->height()) {
                pixel = m_cache->pixel(sx, sy);
            }

            if (rw == 1) {
                paint.setPen(pixel);
                paint.setBrush(Qt::NoBrush);
                paint.drawLine(r.x(), r.y(), r.x(), r.y() + r.height() - 1);
                continue;
            }

            QColor pen(255, 255, 255, 80);
            QColor brush(pixel);

            if (rw > 3 && r.height() > 3) {
                brush.setAlpha(160);
            }

            paint.setPen(Qt::NoPen);
            paint.setBrush(brush);

            if (illuminate) {
                if (r.contains(illuminatePos)) {
                    paint.setPen(v->getForeground());
                }
            }
            
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
            //            cerr << "rect " << r.x() << "," << r.y() << " "
            //                      << r.width() << "x" << r.height() << endl;
#endif

            paint.drawRect(r);

            if (showLabel) {
                if (sx >= 0 && sx < m_cache->width() &&
                        sy >= 0 && sy < m_cache->height()) {
                    double value = m_model->getValueAt(sx, sy);
                    snprintf(labelbuf, buflen, "%06f", value);
                    QString text(labelbuf);
                    paint.setPen(v->getBackground());
                    paint.drawText(rx0 + 2,
                                   ry0 - h / sh - 1 + 2 + paint.fontMetrics().ascent(),
                                   text);
                }
            }
        }
    }
}

void
Colour3DPlotLayer::paintDense(View *v, QPainter &paint, QRect rect) const
{
    Profiler profiler("Colour3DPlotLayer::paintDense", true);
    if (!m_cache) return;

    double modelStart = double(m_model->getStartFrame());
    double modelResolution = double(m_model->getResolution());

    sv_samplerate_t mmsr = v->getViewManager()->getMainModelSampleRate();
    sv_samplerate_t msr = m_model->getSampleRate();
    double srRatio = mmsr / msr;

    int x0 = rect.left();
    int x1 = rect.right() + 1;

    const int w = x1 - x0; // const so it can be used as array size below
    int h = v->height(); // we always paint full height
    int sh = m_model->getHeight();

    int symin = m_miny;
    int symax = m_maxy;
    if (symax <= symin) {
        symin = 0;
        symax = sh;
    }
    if (symin < 0) symin = 0;
    if (symax > sh) symax = sh;

    QImage img(w, h, QImage::Format_Indexed8);
    img.setColorTable(m_cache->colorTable());

    uchar *peaks = new uchar[w];
    memset(peaks, 0, w);

    int zoomLevel = v->getZoomLevel();
    
    QImage *source = m_cache;

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT    
    cerr << "modelResolution " << modelResolution << ", srRatio "
         << srRatio << ", m_peakResolution " << m_peakResolution
         << ", zoomLevel " << zoomLevel << ", result "
         << ((modelResolution * srRatio * m_peakResolution) / zoomLevel)
         << endl;
#endif

    if (m_peaksCache) {
        if (((modelResolution * srRatio * m_peakResolution) / zoomLevel) < 1) {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT    
            cerr << "using peaks cache" << endl;
#endif
            source = m_peaksCache;
            modelResolution *= m_peakResolution;
        } else {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT    
            cerr << "not using peaks cache" << endl;
#endif
        }
    } else {
#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT    
        cerr << "have no peaks cache" << endl;
#endif
    }

    int sw = source->width();
    
    sv_frame_t xf = -1;
    sv_frame_t nxf = v->getFrameForX(x0);

    double epsilon = 0.000001;

    vector<double> sxa(w*2);
    
    for (int x = 0; x < w; ++x) {

        xf = nxf;
        nxf = xf + zoomLevel;

        double sx0 = (double(xf) / srRatio - modelStart) / modelResolution;
        double sx1 = (double(nxf) / srRatio - modelStart) / modelResolution;

        sxa[x*2] = sx0;
        sxa[x*2 + 1] = sx1;
    }

    double logmin = symin+1, logmax = symax+1;
    LogRange::mapRange(logmin, logmax);

#ifdef DEBUG_COLOUR_3D_PLOT_LAYER_PAINT
    cerr << "m_smooth = " << m_smooth << ", w = " << w << ", h = " << h << endl;
#endif

    if (m_smooth) {
        
        for (int y = 0; y < h; ++y) {

            double sy = getBinForY(v, y) - 0.5;
            int syi = int(sy + epsilon);
            if (syi < 0 || syi >= source->height()) continue;

            uchar *targetLine = img.scanLine(y);
            uchar *sourceLine = source->scanLine(syi);
            uchar *nextSource;
            if (syi + 1 < source->height()) {
                nextSource = source->scanLine(syi + 1);
            } else {
                nextSource = sourceLine;
            }

            for (int x = 0; x < w; ++x) {

                targetLine[x] = 0;

                double sx0 = sxa[x*2];
                if (sx0 < 0) continue;
                int sx0i = int(sx0 + epsilon);
                if (sx0i >= sw) break;

                double a = sourceLine[sx0i];
                double b = a;
                double value;

                double sx1 = sxa[x*2+1];
                if (sx1 > sx0 + 1.f) {
                    int sx1i = int(sx1);
                    bool have = false;
                    for (int sx = sx0i; sx <= sx1i; ++sx) {
                        if (sx < 0 || sx >= sw) continue;
                        if (!have) {
                            a = sourceLine[sx];
                            b = nextSource[sx];
                            have = true;
                        } else {
                            a = std::max(a, double(sourceLine[sx]));
                            b = std::max(b, double(nextSource[sx]));
                        }
                    }
                    double yprop = sy - syi;
                    value = (a * (1.f - yprop) + b * yprop);
                } else {
                    a = sourceLine[sx0i];
                    b = nextSource[sx0i];
                    double yprop = sy - syi;
                    value = (a * (1.f - yprop) + b * yprop);
                    int oi = sx0i + 1;
                    double xprop = sx0 - sx0i;
                    xprop -= 0.5;
                    if (xprop < 0) {
                        oi = sx0i - 1;
                        xprop = -xprop;
                    }
                    if (oi < 0 || oi >= sw) oi = sx0i;
                    a = sourceLine[oi];
                    b = nextSource[oi];
                    value = (value * (1.f - xprop) +
                             (a * (1.f - yprop) + b * yprop) * xprop);
                }
                
                int vi = int(lrint(value));
                if (vi > 255) vi = 255;
                if (vi < 0) vi = 0;
                targetLine[x] = uchar(vi);
            }
        }
    } else {

        double sy0 = getBinForY(v, 0);

        int psy0i = -1, psy1i = -1;

        for (int y = 0; y < h; ++y) {

            double sy1 = sy0;
            sy0 = getBinForY(v, double(y + 1));

            int sy0i = int(sy0 + epsilon);
            int sy1i = int(sy1);

            uchar *targetLine = img.scanLine(y);

            if (sy0i == psy0i && sy1i == psy1i) {
                // same source scan line as just computed
                goto copy;
            }

            psy0i = sy0i;
            psy1i = sy1i;

            for (int x = 0; x < w; ++x) {
                peaks[x] = 0;
            }

            for (int sy = sy0i; sy <= sy1i; ++sy) {

                if (sy < 0 || sy >= source->height()) continue;

                uchar *sourceLine = source->scanLine(sy);

                for (int x = 0; x < w; ++x) {

                    double sx1 = sxa[x*2 + 1];
                    if (sx1 < 0) continue;
                    int sx1i = int(sx1);

                    double sx0 = sxa[x*2];
                    if (sx0 < 0) continue;
                    int sx0i = int(sx0 + epsilon);
                    if (sx0i >= sw) break;

                    uchar peak = 0;
                    for (int sx = sx0i; sx <= sx1i; ++sx) {
                        if (sx < 0 || sx >= sw) continue;
                        if (sourceLine[sx] > peak) peak = sourceLine[sx];
                    }
                    peaks[x] = peak;
                }
            }

copy:
            for (int x = 0; x < w; ++x) {
                targetLine[x] = peaks[x];
            }
        }
    }

    delete[] peaks;

    paint.drawImage(x0, 0, img);
}

bool
Colour3DPlotLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        return Layer::snapToFeatureFrame(v, frame, resolution, snap, y);
    }

    resolution = m_model->getResolution();
    sv_frame_t left = (frame / resolution) * resolution;
    sv_frame_t right = left + resolution;

    switch (snap) {
    case SnapLeft:  frame = left;  break;
    case SnapRight: frame = right; break;
    case SnapNearest:
    case SnapNeighbouring:
        if (frame - left > right - frame) frame = right;
        else frame = left;
        break;
    }
    
    return true;
}

void
Colour3DPlotLayer::toXml(QTextStream &stream,
                         QString indent, QString extraAttributes) const
{
    QString s = QString("scale=\"%1\" "
                        "colourScheme=\"%2\" "
                        "normalizeColumns=\"%3\" "
                        "normalizeVisibleArea=\"%4\" "
                        "minY=\"%5\" "
                        "maxY=\"%6\" "
                        "invertVertical=\"%7\" "
                        "opaque=\"%8\" %9")
            .arg((int)m_colourScale)
            .arg(m_colourMap)
            .arg(m_normalizeColumns ? "true" : "false")
            .arg(m_normalizeVisibleArea ? "true" : "false")
            .arg(m_miny)
            .arg(m_maxy)
            .arg(m_invertVertical ? "true" : "false")
            .arg(m_opaque ? "true" : "false")
            .arg(QString("binScale=\"%1\" smooth=\"%2\" gain=\"%3\" ")
                 .arg((int)m_binScale)
                 .arg(m_smooth ? "true" : "false")
                 .arg(m_gain));
    
    Layer::toXml(stream, indent, extraAttributes + " " + s);
}

void
Colour3DPlotLayer::setProperties(const QXmlAttributes &attributes)
{
    bool ok = false, alsoOk = false;

    ColourScale scale = (ColourScale)attributes.value("scale").toInt(&ok);
    if (ok) setColourScale(scale);

    int colourMap = attributes.value("colourScheme").toInt(&ok);
    if (ok) setColourMap(colourMap);

    BinScale binscale = (BinScale)attributes.value("binScale").toInt(&ok);
    if (ok) setBinScale(binscale);

    bool normalizeColumns =
            (attributes.value("normalizeColumns").trimmed() == "true");
    setNormalizeColumns(normalizeColumns);

    bool normalizeVisibleArea =
            (attributes.value("normalizeVisibleArea").trimmed() == "true");
    setNormalizeVisibleArea(normalizeVisibleArea);

    bool invertVertical =
            (attributes.value("invertVertical").trimmed() == "true");
    setInvertVertical(invertVertical);

    bool opaque =
            (attributes.value("opaque").trimmed() == "true");
    setOpaque(opaque);

    bool smooth =
            (attributes.value("smooth").trimmed() == "true");
    setSmooth(smooth);

    float gain = attributes.value("gain").toFloat(&ok);
    if (ok) setGain(gain);

    float min = attributes.value("minY").toFloat(&ok);
    float max = attributes.value("maxY").toFloat(&alsoOk);
    if (ok && alsoOk) setDisplayExtents(min, max);
}

