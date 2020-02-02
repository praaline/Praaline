/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "SliceLayer.h"

#include "view/View.h"
#include "base/AudioLevel.h"
#include "base/RangeMapper.h"
#include "PraalineCore/Base/RealTime.h"
#include "ColourMapper.h"
#include "ColourDatabase.h"
#include "LayerFactory.h"
#include "PaintAssistant.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextStream>

SliceLayer::SliceLayer() :
    m_sliceableModel(0),
    m_colourMap(0),
    m_energyScale(dBScale),
    m_samplingMode(SampleMean),
    m_plotStyle(PlotSteps),
    m_binScale(LinearBins),
    m_normalize(false),
    m_threshold(0.0),
    m_initialThreshold(0.0),
    m_gain(1.0),
    m_currentf0(0),
    m_currentf1(0)
{
}

SliceLayer::~SliceLayer()
{

}

bool SliceLayer::trySetModel(Model *)
{
    return false;
}

void
SliceLayer::setSliceableModel(const Model *model)
{
    const DenseThreeDimensionalModel *sliceable =
        dynamic_cast<const DenseThreeDimensionalModel *>(model);

    if (model && !sliceable) {
        cerr << "WARNING: SliceLayer::setSliceableModel(" << model
                  << "): model is not a DenseThreeDimensionalModel" << endl;
    }

    if (m_sliceableModel == sliceable) return;

    m_sliceableModel = sliceable;

    connectSignals(m_sliceableModel);

    emit modelReplaced();
}

void
SliceLayer::sliceableModelReplaced(const Model *orig, const Model *replacement)
{
    cerr << "SliceLayer::sliceableModelReplaced(" << orig << ", " << replacement << ")" << endl;

    if (orig == m_sliceableModel) {
        setSliceableModel
            (dynamic_cast<const DenseThreeDimensionalModel *>(replacement));
    }
}

void
SliceLayer::modelAboutToBeDeleted(Model *m)
{
    cerr << "SliceLayer::modelAboutToBeDeleted(" << m << ")" << endl;

    if (m == m_sliceableModel) {
        setSliceableModel(0);
    }
}

QString
SliceLayer::getFeatureDescription(View *v, QPoint &p) const
{
    int minbin, maxbin, range;
    return getFeatureDescriptionAux(v, p, true, minbin, maxbin, range);
}

QString
SliceLayer::getFeatureDescriptionAux(View *v, QPoint &p,
                                     bool includeBinDescription,
                                     int &minbin, int &maxbin, int &range) const
{
    minbin = 0;
    maxbin = 0;
    if (!m_sliceableModel) return "";

    int xorigin = m_xorigins[v];
    int w = v->width() - xorigin - 1;
    
    int mh = m_sliceableModel->getHeight();
    minbin = getBinForX(p.x() - xorigin, mh, w);
    maxbin = getBinForX(p.x() - xorigin + 1, mh, w);

    if (minbin >= mh) minbin = mh - 1;
    if (maxbin >= mh) maxbin = mh - 1;
    if (minbin < 0) minbin = 0;
    if (maxbin < 0) maxbin = 0;
    
    sv_samplerate_t sampleRate = m_sliceableModel->getSampleRate();

    sv_frame_t f0 = m_currentf0;
    sv_frame_t f1 = m_currentf1;

    RealTime rt0 = RealTime::frame2RealTime(f0, sampleRate);
    RealTime rt1 = RealTime::frame2RealTime(f1, sampleRate);
    
    range = int(f1 - f0 + 1);

    QString rtrangestr = QString("%1 s").arg((rt1 - rt0).toText().c_str());

    if (includeBinDescription) {

        float minvalue = 0.0;
        if (minbin < int(m_values.size())) minvalue = m_values[minbin];

        float maxvalue = minvalue;
        if (maxbin < int(m_values.size())) maxvalue = m_values[maxbin];
        
        if (minvalue > maxvalue) std::swap(minvalue, maxvalue);
        
        QString binstr;
        if (maxbin != minbin) {
            binstr = tr("%1 - %2").arg(minbin+1).arg(maxbin+1);
        } else {
            binstr = QString("%1").arg(minbin+1);
        }

        QString valuestr;
        if (maxvalue != minvalue) {
            valuestr = tr("%1 - %2").arg(minvalue).arg(maxvalue);
        } else {
            valuestr = QString("%1").arg(minvalue);
        }

        QString description = tr("Time:\t%1 - %2\nRange:\t%3 samples (%4)\nBin:\t%5\n%6 value:\t%7")
            .arg(QString::fromStdString(rt0.toText(true)))
            .arg(QString::fromStdString(rt1.toText(true)))
            .arg(range)
            .arg(rtrangestr)
            .arg(binstr)
            .arg(m_samplingMode == NearestSample ? tr("First") :
                 m_samplingMode == SampleMean ? tr("Mean") : tr("Peak"))
            .arg(valuestr);
        
        return description;
    
    } else {

        QString description = tr("Time:\t%1 - %2\nRange:\t%3 samples (%4)")
            .arg(QString::fromStdString(rt0.toText(true)))
            .arg(QString::fromStdString(rt1.toText(true)))
            .arg(range)
            .arg(rtrangestr);
        
        return description;
    }
}

double
SliceLayer::getXForBin(int bin, int count, double w) const
{
    double x = 0;

    switch (m_binScale) {

    case LinearBins:
        x = (w * bin) / count;
        break;
        
    case LogBins:
        x = (w * log10(bin + 1)) / log10(count + 1);
        break;
        
    case InvertedLogBins:
        x = w - (w * log10(count - bin - 1)) / log10(count);
        break;
    }

    return x;
}

int
SliceLayer::getBinForX(double x, int count, double w) const
{
    int bin = 0;

    switch (m_binScale) {

    case LinearBins:
        bin = int((x * count) / w + 0.0001);
        break;
        
    case LogBins:
        bin = int(pow(10.0, (x * log10(count + 1)) / w) - 1 + 0.0001);
        break;

    case InvertedLogBins:
        bin = count + 1 - int(pow(10.0, (log10(count) * (w - x)) / double(w)) + 0.0001);
        break;
    }

    return bin;
}

double
SliceLayer::getYForValue(double value, const View *v, double &norm) const
{
    norm = 0.0;

    if (m_yorigins.find(v) == m_yorigins.end()) return 0;

    value *= m_gain;

    int yorigin = m_yorigins[v];
    int h = m_heights[v];
    double thresh = getThresholdDb();

    double y = 0.0;

    if (h <= 0) return y;

    switch (m_energyScale) {

    case dBScale:
    {
        double db = thresh;
        if (value > 0.0) db = 10.0 * log10(fabs(value));
        if (db < thresh) db = thresh;
        norm = (db - thresh) / -thresh;
        y = yorigin - (double(h) * norm);
        break;
    }
    
    case MeterScale:
        y = AudioLevel::multiplier_to_preview(value, h);
        norm = double(y) / double(h);
        y = yorigin - y;
        break;
        
    case AbsoluteScale:
        value = fabs(value);
        // and fall through
        
    case LinearScale:
    default:
        norm = (value - m_threshold);
        if (norm < 0) norm = 0;
        y = yorigin - (double(h) * norm);
        break;
    }
    
    return y;
}

double
SliceLayer::getValueForY(double y, const View *v) const
{
    double value = 0.0;

    if (m_yorigins.find(v) == m_yorigins.end()) return value;

    int yorigin = m_yorigins[v];
    int h = m_heights[v];
    double thresh = getThresholdDb();

    if (h <= 0) return value;

    y = yorigin - y;

    switch (m_energyScale) {

    case dBScale:
    {
        double db = ((y / h) * -thresh) + thresh;
        value = pow(10.0, db/10.0);
        break;
    }

    case MeterScale:
        value = AudioLevel::preview_to_multiplier(int(lrint(y)), h);
        break;

    case LinearScale:
    case AbsoluteScale:
    default:
        value = y / h + m_threshold;
    }

    return value / m_gain;
}

void
SliceLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_sliceableModel || !m_sliceableModel->isOK() ||
        !m_sliceableModel->isReady()) return;

    paint.save();
    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.setBrush(Qt::NoBrush);

    if (v->getViewManager() && v->getViewManager()->shouldShowScaleGuides()) {
        if (!m_scalePoints.empty()) {
            paint.setPen(QColor(240, 240, 240)); //!!! and dark background?
            for (int i = 0; i < (int)m_scalePoints.size(); ++i) {
                paint.drawLine(0, m_scalePoints[i], rect.width(), m_scalePoints[i]);
            }
        }
    }

    paint.setPen(getBaseQColor());

    int xorigin = getVerticalScaleWidth(v, true, paint) + 1;
    int w = v->width() - xorigin - 1;

    m_xorigins[v] = xorigin; // for use in getFeatureDescription
    
    int yorigin = v->height() - 20 - paint.fontMetrics().height() - 7;
    int h = yorigin - paint.fontMetrics().height() - 8;

    m_yorigins[v] = yorigin; // for getYForValue etc
    m_heights[v] = h;

    if (h <= 0) return;

    QPainterPath path;

    int mh = m_sliceableModel->getHeight();

    int divisor = 0;

    m_values.clear();
    for (int bin = 0; bin < mh; ++bin) {
        m_values.push_back(0.0);
    }

    sv_frame_t f0 = v->getCentreFrame();
    int f0x = v->getXForFrame(f0);
    f0 = v->getFrameForX(f0x);
    sv_frame_t f1 = v->getFrameForX(f0x + 1);
    if (f1 > f0) --f1;

//    cerr << "centre frame " << v->getCentreFrame() << ", x " << f0x << ", f0 " << f0 << ", f1 " << f1 << endl;

    int res = m_sliceableModel->getResolution();
    int col0 = int(f0 / res);
    int col1 = col0;
    if (m_samplingMode != NearestSample) col1 = int(f1 / res);
    f0 = col0 * res;
    f1 = (col1 + 1) * res - 1;

//    cerr << "resolution " << res << ", col0 " << col0 << ", col1 " << col1 << ", f0 " << f0 << ", f1 " << f1 << endl;

    m_currentf0 = f0;
    m_currentf1 = f1;

    BiasCurve curve;
    getBiasCurve(curve);
    int cs = int(curve.size());

    for (int col = col0; col <= col1; ++col) {
        for (int bin = 0; bin < mh; ++bin) {
            float value = m_sliceableModel->getValueAt(col, bin);
            if (bin < cs) value *= curve[bin];
            if (m_samplingMode == SamplePeak) {
                if (value > m_values[bin]) m_values[bin] = value;
            } else {
                m_values[bin] += value;
            }
        }
        ++divisor;
    }

    float max = 0.0;
    for (int bin = 0; bin < mh; ++bin) {
        if (m_samplingMode == SampleMean && divisor > 0) {
            m_values[bin] /= float(divisor);
        }
        if (m_values[bin] > max) max = m_values[bin];
    }
    if (max != 0.0 && m_normalize) {
        for (int bin = 0; bin < mh; ++bin) {
            m_values[bin] /= max;
        }
    }

    double nx = xorigin;

    ColourMapper mapper(m_colourMap, 0, 1);

    for (int bin = 0; bin < mh; ++bin) {

        double x = nx;
        nx = xorigin + getXForBin(bin + 1, mh, w);

        double value = m_values[bin];
        double norm = 0.0;
        double y = getYForValue(value, v, norm);

        if (m_plotStyle == PlotLines) {

            if (bin == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }

        } else if (m_plotStyle == PlotSteps) {

            if (bin == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
            path.lineTo(nx, y);

        } else if (m_plotStyle == PlotBlocks) {

            path.moveTo(x, yorigin);
            path.lineTo(x, y);
            path.lineTo(nx, y);
            path.lineTo(nx, yorigin);
            path.lineTo(x, yorigin);

        } else if (m_plotStyle == PlotFilledBlocks) {

            paint.fillRect(QRectF(x, y, nx - x, yorigin - y), mapper.map(norm));
        }

    }

    if (m_plotStyle != PlotFilledBlocks) {
        paint.drawPath(path);
    }
    paint.restore();
/*
    QPoint discard;

    if (v->getViewManager() && v->getViewManager()->shouldShowFrameCount() &&
        v->shouldIlluminateLocalFeatures(this, discard)) {

        int sampleRate = m_sliceableModel->getSampleRate();

        QString startText = QString("%1 / %2")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime
                  (f0, sampleRate).toText(true)))
            .arg(f0);

        QString endText = QString(" %1 / %2")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime
                  (f1, sampleRate).toText(true)))
            .arg(f1);

        QString durationText = QString("(%1 / %2) ")
            .arg(QString::fromStdString
                 (RealTime::frame2RealTime
                  (f1 - f0 + 1, sampleRate).toText(true)))
            .arg(f1 - f0 + 1);

        v->drawVisibleText
            (paint, xorigin + 5,
             paint.fontMetrics().ascent() + 5,
             startText, View::OutlinedText);
        
        v->drawVisibleText
            (paint, xorigin + 5,
             paint.fontMetrics().ascent() + paint.fontMetrics().height() + 10,
             endText, View::OutlinedText);
        
        v->drawVisibleText
            (paint, xorigin + 5,
             paint.fontMetrics().ascent() + 2*paint.fontMetrics().height() + 15,
             durationText, View::OutlinedText);
    }
*/
}

int
SliceLayer::getVerticalScaleWidth(View *, bool, QPainter &paint) const
{
    if (m_energyScale == LinearScale || m_energyScale == AbsoluteScale) {
	return std::max(paint.fontMetrics().width("0.0") + 13,
                        paint.fontMetrics().width("x10-10"));
    } else {
	return std::max(paint.fontMetrics().width(tr("0dB")),
			paint.fontMetrics().width(tr("-Inf"))) + 13;
    }
}

void
SliceLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const
{
    double thresh = m_threshold;
    if (m_energyScale != LinearScale && m_energyScale != AbsoluteScale) {
        thresh = AudioLevel::dB_to_multiplier(getThresholdDb());
    }
    
//    int h = (rect.height() * 3) / 4;
//    int y = (rect.height() / 2) - (h / 2);
    
    int yorigin = v->height() - 20 - paint.fontMetrics().height() - 6;
    int h = yorigin - paint.fontMetrics().height() - 8;
    if (h < 0) return;

    QRect actual(rect.x(), rect.y() + yorigin - h, rect.width(), h);

    int mult = 1;

    PaintAssistant::paintVerticalLevelScale
        (paint, actual, thresh, 1.0 / m_gain,
         PaintAssistant::Scale(m_energyScale),
         mult,
         const_cast<std::vector<int> *>(&m_scalePoints));

    if (mult != 1 && mult != 0) {
        int log = int(lrint(log10(mult)));
        QString a = tr("x10");
        QString b = QString("%1").arg(-log);
        paint.drawText(3, 8 + paint.fontMetrics().ascent(), a);
        paint.drawText(3 + paint.fontMetrics().width(a),
                       3 + paint.fontMetrics().ascent(), b);
    }
}

Layer::PropertyList
SliceLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Bin Scale");
    list.push_back("Plot Type");
    list.push_back("Scale");
    list.push_back("Normalize");
    list.push_back("Threshold");
    list.push_back("Gain");

    return list;
}

QString
SliceLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Plot Type") return tr("Plot Type");
    if (name == "Scale") return tr("Scale");
    if (name == "Normalize") return tr("Normalize");
    if (name == "Threshold") return tr("Threshold");
    if (name == "Gain") return tr("Gain");
    if (name == "Sampling Mode") return tr("Sampling Mode");
    if (name == "Bin Scale") return tr("Bin Scale");
    return SingleColourLayer::getPropertyLabel(name);
}

QString
SliceLayer::getPropertyIconName(const PropertyName &name) const
{
    if (name == "Normalize") return "normalise";
    return "";
}

Layer::PropertyType
SliceLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Gain") return RangeProperty;
    if (name == "Normalize") return ToggleProperty;
    if (name == "Threshold") return RangeProperty;
    if (name == "Plot Type") return ValueProperty;
    if (name == "Scale") return ValueProperty;
    if (name == "Sampling Mode") return ValueProperty;
    if (name == "Bin Scale") return ValueProperty;
    if (name == "Colour" && m_plotStyle == PlotFilledBlocks) return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

QString
SliceLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Scale" ||
        name == "Normalize" ||
        name == "Sampling Mode" ||
        name == "Threshold" ||
        name == "Gain") return tr("Scale");
    if (name == "Plot Type" ||
        name == "Bin Scale") return tr("Bins");
    return SingleColourLayer::getPropertyGroupName(name);
}

int
SliceLayer::getPropertyRangeAndValue(const PropertyName &name,
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
        *deflt = 0;

        cerr << "gain is " << m_gain << ", mode is " << m_samplingMode << endl;

	val = int(lrint(log10(m_gain) * 20.0));
	if (val < *min) val = *min;
	if (val > *max) val = *max;

    } else if (name == "Threshold") {
        
	*min = -80;
	*max = 0;

        *deflt = int(lrint(AudioLevel::multiplier_to_dB(m_initialThreshold)));
	if (*deflt < *min) *deflt = *min;
	if (*deflt > *max) *deflt = *max;

	val = int(lrint(AudioLevel::multiplier_to_dB(m_threshold)));
	if (val < *min) val = *min;
	if (val > *max) val = *max;

    } else if (name == "Normalize") {
	
	val = (m_normalize ? 1 : 0);
        *deflt = 0;

    } else if (name == "Colour" && m_plotStyle == PlotFilledBlocks) {
            
        *min = 0;
        *max = ColourMapper::getColourMapCount() - 1;
        *deflt = 0;
        
        val = m_colourMap;

    } else if (name == "Scale") {

	*min = 0;
	*max = 3;
        *deflt = (int)dBScale;

	val = (int)m_energyScale;

    } else if (name == "Sampling Mode") {

	*min = 0;
	*max = 2;
        *deflt = (int)SampleMean;
        
	val = (int)m_samplingMode;

    } else if (name == "Plot Type") {
        
        *min = 0;
        *max = 3;
        *deflt = (int)PlotSteps;

        val = (int)m_plotStyle;

    } else if (name == "Bin Scale") {
        
        *min = 0;
        *max = 2;
        *deflt = (int)LinearBins;
//        *max = 1; // I don't think we really do want to offer inverted log

        val = (int)m_binScale;

    } else {
	val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
SliceLayer::getPropertyValueLabel(const PropertyName &name,
				    int value) const
{
    if (name == "Colour" && m_plotStyle == PlotFilledBlocks) {
        return ColourMapper::getColourMapName(value);
    }
    if (name == "Scale") {
	switch (value) {
	default:
	case 0: return tr("Linear");
	case 1: return tr("Meter");
	case 2: return tr("Log");
	case 3: return tr("Absolute");
	}
    }
    if (name == "Sampling Mode") {
	switch (value) {
	default:
	case 0: return tr("Any");
	case 1: return tr("Mean");
	case 2: return tr("Peak");
	}
    }
    if (name == "Plot Type") {
	switch (value) {
	default:
	case 0: return tr("Lines");
	case 1: return tr("Steps");
	case 2: return tr("Blocks");
	case 3: return tr("Colours");
	}
    }
    if (name == "Bin Scale") {
	switch (value) {
	default:
	case 0: return tr("Linear");
	case 1: return tr("Log");
	case 2: return tr("Rev Log");
	}
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

RangeMapper *
SliceLayer::getNewPropertyRangeMapper(const PropertyName &name) const
{
    if (name == "Gain") {
        return new LinearRangeMapper(-50, 50, -25, 25, tr("dB"));
    }
    if (name == "Threshold") {
        return new LinearRangeMapper(-80, 0, -80, 0, tr("dB"));
    }
    return SingleColourLayer::getNewPropertyRangeMapper(name);
}

void
SliceLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Gain") {
	setGain(powf(10, float(value)/20.0f));
    } else if (name == "Threshold") {
	if (value == -80) setThreshold(0.0f);
	else setThreshold(float(AudioLevel::dB_to_multiplier(value)));
    } else if (name == "Colour" && m_plotStyle == PlotFilledBlocks) {
        setFillColourMap(value);
    } else if (name == "Scale") {
	switch (value) {
	default:
	case 0: setEnergyScale(LinearScale); break;
	case 1: setEnergyScale(MeterScale); break;
	case 2: setEnergyScale(dBScale); break;
	case 3: setEnergyScale(AbsoluteScale); break;
	}
    } else if (name == "Plot Type") {
	setPlotStyle(PlotStyle(value));
    } else if (name == "Sampling Mode") {
	switch (value) {
	default:
	case 0: setSamplingMode(NearestSample); break;
	case 1: setSamplingMode(SampleMean); break;
	case 2: setSamplingMode(SamplePeak); break;
	}
    } else if (name == "Bin Scale") {
	switch (value) {
	default:
	case 0: setBinScale(LinearBins); break;
	case 1: setBinScale(LogBins); break;
	case 2: setBinScale(InvertedLogBins); break;
	}
    } else if (name == "Normalize") {
	setNormalize(value ? true : false);
    } else {
        SingleColourLayer::setProperty(name, value);
    }
}

void
SliceLayer::setFillColourMap(int map)
{
    if (m_colourMap == map) return;
    m_colourMap = map;
    emit layerParametersChanged();
}

void
SliceLayer::setEnergyScale(EnergyScale scale)
{
    if (m_energyScale == scale) return;
    m_energyScale = scale;
    emit layerParametersChanged();
}

void
SliceLayer::setSamplingMode(SamplingMode mode)
{
    if (m_samplingMode == mode) return;
    m_samplingMode = mode;
    emit layerParametersChanged();
}

void
SliceLayer::setPlotStyle(PlotStyle style)
{
    if (m_plotStyle == style) return;
    bool colourTypeChanged = (style == PlotFilledBlocks ||
                              m_plotStyle == PlotFilledBlocks);
    m_plotStyle = style;
    if (colourTypeChanged) {
        emit layerParameterRangesChanged();
    }
    emit layerParametersChanged();
}

void
SliceLayer::setBinScale(BinScale scale)
{
    if (m_binScale == scale) return;
    m_binScale = scale;
    emit layerParametersChanged();
}

void
SliceLayer::setNormalize(bool n)
{
    if (m_normalize == n) return;
    m_normalize = n;
    emit layerParametersChanged();
}

void
SliceLayer::setThreshold(float thresh)
{
    if (m_threshold == thresh) return;
    m_threshold = thresh;
    emit layerParametersChanged();
}

void
SliceLayer::setGain(float gain)
{
    if (m_gain == gain) return;
    m_gain = gain;
    emit layerParametersChanged();
}

float
SliceLayer::getThresholdDb() const
{
    if (m_threshold == 0.0) return -80.f;
    float db = float(AudioLevel::multiplier_to_dB(m_threshold));
    return db;
}

int
SliceLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
        (QString(darkbg ? "Bright Blue" : "Blue"));
}

void
SliceLayer::toXml(QTextStream &stream,
                  QString indent, QString extraAttributes) const
{
    QString s;
    
    s += QString("colourScheme=\"%1\" "
		 "energyScale=\"%2\" "
                 "samplingMode=\"%3\" "
                 "plotStyle=\"%4\" "
                 "binScale=\"%5\" "
                 "gain=\"%6\" "
                 "threshold=\"%7\" "
                 "normalize=\"%8\"")
        .arg(m_colourMap)
	.arg(m_energyScale)
        .arg(m_samplingMode)
        .arg(m_plotStyle)
        .arg(m_binScale)
        .arg(m_gain)
        .arg(m_threshold)
        .arg(m_normalize ? "true" : "false");

    SingleColourLayer::toXml(stream, indent, extraAttributes + " " + s);
}

void
SliceLayer::setProperties(const QXmlAttributes &attributes)
{
    bool ok = false;

    SingleColourLayer::setProperties(attributes);

    EnergyScale scale = (EnergyScale)
	attributes.value("energyScale").toInt(&ok);
    if (ok) setEnergyScale(scale);

    SamplingMode mode = (SamplingMode)
	attributes.value("samplingMode").toInt(&ok);
    if (ok) setSamplingMode(mode);

    int colourMap = attributes.value("colourScheme").toInt(&ok);
    if (ok) setFillColourMap(colourMap);

    PlotStyle s = (PlotStyle)
	attributes.value("plotStyle").toInt(&ok);
    if (ok) setPlotStyle(s);

    BinScale b = (BinScale)
	attributes.value("binScale").toInt(&ok);
    if (ok) setBinScale(b);

    float gain = attributes.value("gain").toFloat(&ok);
    if (ok) setGain(gain);

    float threshold = attributes.value("threshold").toFloat(&ok);
    if (ok) setThreshold(threshold);

    bool normalize = (attributes.value("normalize").trimmed() == "true");
    setNormalize(normalize);
}

bool
SliceLayer::getValueExtents(double &, double &, bool &, QString &) const
{
    return false;
}

