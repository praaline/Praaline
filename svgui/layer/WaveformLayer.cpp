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

#include "WaveformLayer.h"

#include "base/AudioLevel.h"
#include "view/View.h"
#include "base/Profiler.h"
#include "base/RangeMapper.h"
#include "ColourDatabase.h"
#include "LayerFactory.h"
#include "data/model/WaveFileModel.h"
#include "data/model/WritableWaveFileModel.h"

#include <QPainter>
#include <QPixmap>
#include <QTextStream>

#include <iostream>
#include <cmath>

//#define DEBUG_WAVEFORM_PAINT 1

WaveformLayer::WaveformLayer() :
    SingleColourLayer(),
    m_model(0),
    m_gain(1.0f),
    m_autoNormalize(false),
    m_showMeans(true),
    m_greyscale(true),
    m_channelMode(SeparateChannels),
    m_channel(-1),
    m_scale(LinearScale),
    m_middleLineHeight(0.5),
    m_aggressive(false),
    m_cache(0),
    m_cacheValid(false),
    m_cacheZoomLevel(0)
{
    
}

WaveformLayer::~WaveformLayer()
{
    delete m_cache;
}

bool WaveformLayer::trySetModel(Model *model)
{
    if (trySetModelHelper<WaveformLayer, WaveFileModel>(this, model))
        return true;
    if (trySetModelHelper<WaveformLayer, WritableWaveFileModel>(this, model))
        return true;
    return false;
}

void WaveformLayer::setModel(const RangeSummarisableTimeValueModel *model)
{
    bool channelsChanged = false;
    if (m_channel == -1) {
        if (!m_model) {
            if (model) {
                channelsChanged = true;
            }
        } else {
            if (model &&
                    m_model->getChannelCount() != model->getChannelCount()) {
                channelsChanged = true;
            }
        }
    }

    m_model = model;
    m_cacheValid = false;
    if (!m_model || !m_model->isOK()) return;

    connectSignals(m_model);

    emit modelReplaced();

    if (channelsChanged) emit layerParametersChanged();
}

Layer::PropertyList
WaveformLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Scale");
    list.push_back("Gain");
    list.push_back("Normalize Visible Area");

    if (m_model && m_model->getChannelCount() > 1 && m_channel == -1) {
        list.push_back("Channels");
    }

    return list;
}

QString
WaveformLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Scale") return tr("Scale");
    if (name == "Gain") return tr("Gain");
    if (name == "Normalize Visible Area") return tr("Normalize Visible Area");
    if (name == "Channels") return tr("Channels");
    return SingleColourLayer::getPropertyLabel(name);
}

QString
WaveformLayer::getPropertyIconName(const PropertyName &name) const
{
    if (name == "Normalize Visible Area") return "normalise";
    return "";
}

Layer::PropertyType
WaveformLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Gain") return RangeProperty;
    if (name == "Normalize Visible Area") return ToggleProperty;
    if (name == "Channels") return ValueProperty;
    if (name == "Scale") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

QString
WaveformLayer::getPropertyGroupName(const PropertyName &name) const
{
    if (name == "Gain" ||
            name == "Normalize Visible Area" ||
            name == "Scale") return tr("Scale");
    return QString();
}

int
WaveformLayer::getPropertyRangeAndValue(const PropertyName &name,
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

        val = int(lrint(log10(m_gain) * 20.0));
        if (val < *min) val = *min;
        if (val > *max) val = *max;

    } else if (name == "Normalize Visible Area") {

        val = (m_autoNormalize ? 1 : 0);
        *deflt = 0;

    } else if (name == "Channels") {

        *min = 0;
        *max = 2;
        *deflt = 0;
        if (m_channelMode == MixChannels) val = 1;
        else if (m_channelMode == MergeChannels) val = 2;
        else val = 0;

    } else if (name == "Scale") {

        *min = 0;
        *max = 2;
        *deflt = 0;

        val = (int)m_scale;

    } else {
        val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
WaveformLayer::getPropertyValueLabel(const PropertyName &name,
                                     int value) const
{
    if (name == "Scale") {
        switch (value) {
        default:
        case 0: return tr("Linear");
        case 1: return tr("Meter");
        case 2: return tr("dB");
        }
    }
    if (name == "Channels") {
        switch (value) {
        default:
        case 0: return tr("Separate");
        case 1: return tr("Mean");
        case 2: return tr("Butterfly");
        }
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

RangeMapper *
WaveformLayer::getNewPropertyRangeMapper(const PropertyName &name) const
{
    if (name == "Gain") {
        return new LinearRangeMapper(-50, 50, -25, 25, tr("dB"));
    }
    return 0;
}

void
WaveformLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Gain") {
        setGain(float(pow(10, float(value)/20.0)));
    } else if (name == "Normalize Visible Area") {
        setAutoNormalize(value ? true : false);
    } else if (name == "Channels") {
        if (value == 1) setChannelMode(MixChannels);
        else if (value == 2) setChannelMode(MergeChannels);
        else setChannelMode(SeparateChannels);
    } else if (name == "Scale") {
        switch (value) {
        default:
        case 0: setScale(LinearScale); break;
        case 1: setScale(MeterScale); break;
        case 2: setScale(dBScale); break;
        }
    } else {
        SingleColourLayer::setProperty(name, value);
    }
}

void
WaveformLayer::setGain(float gain)
{
    if (m_gain == gain) return;
    m_gain = gain;
    m_cacheValid = false;
    emit layerParametersChanged();
    emit verticalZoomChanged();
}

void
WaveformLayer::setAutoNormalize(bool autoNormalize)
{
    if (m_autoNormalize == autoNormalize) return;
    m_autoNormalize = autoNormalize;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setShowMeans(bool showMeans)
{
    if (m_showMeans == showMeans) return;
    m_showMeans = showMeans;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setUseGreyscale(bool useGreyscale)
{
    if (m_greyscale == useGreyscale) return;
    m_greyscale = useGreyscale;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setChannelMode(ChannelMode channelMode)
{
    if (m_channelMode == channelMode) return;
    m_channelMode = channelMode;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setChannel(int channel)
{
    //    cerr << "WaveformLayer::setChannel(" << channel << ")" << endl;

    if (m_channel == channel) return;
    m_channel = channel;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setScale(Scale scale)
{
    if (m_scale == scale) return;
    m_scale = scale;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setMiddleLineHeight(double height)
{
    if (m_middleLineHeight == height) return;
    m_middleLineHeight = height;
    m_cacheValid = false;
    emit layerParametersChanged();
}

void
WaveformLayer::setAggressiveCacheing(bool aggressive)
{
    if (m_aggressive == aggressive) return;
    m_aggressive = aggressive;
    m_cacheValid = false;
    emit layerParametersChanged();
}

int
WaveformLayer::getCompletion(View *) const
{
    int completion = 100;
    if (!m_model || !m_model->isOK()) return completion;
    if (m_model->isReady(&completion)) return 100;
    return completion;
}

bool
WaveformLayer::getValueExtents(double &min, double &max,
                               bool &, QString &unit) const
{
    if (m_scale == LinearScale) {
        min = 0.0;
        max = 1.0;
        unit = "V";
    } else if (m_scale == MeterScale) {
        return false; //!!!
    } else {
        min = AudioLevel::multiplier_to_dB(0.0);
        max = AudioLevel::multiplier_to_dB(1.0);
        unit = "dB";
    }
    return true;
}

int
WaveformLayer::dBscale(double sample, int m) const
{
    if (sample < 0.0) return dBscale(-sample, m);
    double dB = AudioLevel::multiplier_to_dB(sample);
    if (dB < -50.0) return 0;
    if (dB > 0.0) return m;
    return int(((dB + 50.0) * m) / 50.0 + 0.1);
}

int
WaveformLayer::getChannelArrangement(int &min, int &max,
                                     bool &merging, bool &mixing)
const
{
    if (!m_model || !m_model->isOK()) return 0;

    int channels = m_model->getChannelCount();
    if (channels == 0) return 0;

    int rawChannels = channels;

    if (m_channel == -1) {
        min = 0;
        if (m_channelMode == MergeChannels ||
                m_channelMode == MixChannels) {
            max = 0;
            channels = 1;
        } else {
            max = channels - 1;
        }
    } else {
        min = m_channel;
        max = m_channel;
        rawChannels = 1;
        channels = 1;
    }

    merging = (m_channelMode == MergeChannels && rawChannels > 1);
    mixing = (m_channelMode == MixChannels && rawChannels > 1);

    //    cerr << "WaveformLayer::getChannelArrangement: min " << min << ", max " << max << ", merging " << merging << ", channels " << channels << endl;

    return channels;
}    

bool
WaveformLayer::isLayerScrollable(const View *) const
{
    return !m_autoNormalize;
}

static float meterdbs[] = { -40, -30, -20, -15, -10,
                            -5, -3, -2, -1, -0.5, 0 };

bool
WaveformLayer::getSourceFramesForX(View *v, int x, int modelZoomLevel,
                                   sv_frame_t &f0, sv_frame_t &f1) const
{
    sv_frame_t viewFrame = v->getFrameForX(x);
    if (viewFrame < 0) {
        f0 = 0;
        f1 = 0;
        return false;
    }

    f0 = viewFrame;
    
    f0 = f0 / modelZoomLevel;
    f0 = f0 * modelZoomLevel;

    viewFrame = v->getFrameForX(x + 1);
    
    f1 = viewFrame;
    f1 = f1 / modelZoomLevel;
    f1 = f1 * modelZoomLevel;
    
    return (f0 < m_model->getEndFrame());
}

float
WaveformLayer::getNormalizeGain(View *v, int channel) const
{
    sv_frame_t startFrame = v->getStartFrame();
    sv_frame_t endFrame = v->getEndFrame();

    sv_frame_t modelStart = m_model->getStartFrame();
    sv_frame_t modelEnd = m_model->getEndFrame();
    
    sv_frame_t rangeStart, rangeEnd;

    if (startFrame < modelStart) rangeStart = modelStart;
    else rangeStart = startFrame;

    if (endFrame < 0) rangeEnd = 0;
    else if (endFrame > modelEnd) rangeEnd = modelEnd;
    else rangeEnd = endFrame;

    if (rangeEnd < rangeStart) rangeEnd = rangeStart;

    RangeSummarisableTimeValueModel::Range range =
            m_model->getSummary(channel, rangeStart, rangeEnd - rangeStart);

    int minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    (void)getChannelArrangement(minChannel, maxChannel,
                                mergingChannels, mixingChannels);

    if (mergingChannels || mixingChannels) {
        RangeSummarisableTimeValueModel::Range otherRange =
                m_model->getSummary(1, rangeStart, rangeEnd - rangeStart);
        range.setMax(std::max(range.max(), otherRange.max()));
        range.setMin(std::min(range.min(), otherRange.min()));
        range.setAbsmean(std::min(range.absmean(), otherRange.absmean()));
    }

    return float(1.0 / std::max(fabs(range.max()), fabs(range.min())));
}

void
WaveformLayer::paint(View *v, QPainter &viewPainter, QRect rect) const
{
    if (!m_model || !m_model->isOK()) {
        return;
    }

    int zoomLevel = v->getZoomLevel();

#ifdef DEBUG_WAVEFORM_PAINT
    Profiler profiler("WaveformLayer::paint", true);
    cerr << "WaveformLayer::paint (" << rect.x() << "," << rect.y()
         << ") [" << rect.width() << "x" << rect.height() << "]: zoom " << zoomLevel << endl;
#endif

    int channels = 0, minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    channels = getChannelArrangement(minChannel, maxChannel,
                                     mergingChannels, mixingChannels);
    if (channels == 0) return;

    int w = v->width();
    int h = v->height();

    bool ready = m_model->isReady();
    QPainter *paint;

    if (m_aggressive) {

#ifdef DEBUG_WAVEFORM_PAINT
        cerr << "WaveformLayer::paint: aggressive is true" << endl;
#endif

        if (m_cacheValid && (zoomLevel != m_cacheZoomLevel)) {
            m_cacheValid = false;
        }

        if (!m_cache || m_cache->width() != w || m_cache->height() != h) {
#ifdef DEBUG_WAVEFORM_PAINT
            if (m_cache) {
                cerr << "WaveformLayer::paint: cache size " << m_cache->width() << "x" << m_cache->height() << " differs from view size " << w << "x" << h << ": regenerating aggressive cache" << endl;
            }
#endif
            delete m_cache;
            m_cache = new QPixmap(w, h);
            m_cacheValid = false;
        }

        if (m_cacheValid) {
            viewPainter.drawPixmap(rect, *m_cache, rect);
            return;
        }

        paint = new QPainter(m_cache);

        paint->setPen(Qt::NoPen);
        paint->setBrush(getBackgroundQColor(v));
        paint->drawRect(rect);

        paint->setPen(getForegroundQColor(v));
        paint->setBrush(Qt::NoBrush);

    } else {
        paint = &viewPainter;
    }

    paint->setRenderHint(QPainter::Antialiasing, false);

    if (m_middleLineHeight != 0.5) {
        paint->save();
        double space = m_middleLineHeight * 2;
        if (space > 1.0) space = 2.0 - space;
        double yt = h * (m_middleLineHeight - space/2);
        paint->translate(QPointF(0, yt));
        paint->scale(1.0, space);
    }

    int x0 = 0, x1 = w - 1;
    int y0 = 0, y1 = h - 1;

    x0 = rect.left();
    x1 = rect.right();
    y0 = rect.top();
    y1 = rect.bottom();

    if (x0 > 0) --x0;
    if (x1 < v->width()) ++x1;

    // Our zoom level may differ from that at which the underlying
    // model has its blocks.

    // Each pixel within our visible range must always draw from
    // exactly the same set of underlying audio frames, no matter what
    // the range being drawn is.  And that set of underlying frames
    // must remain the same when we scroll one or more pixels left or
    // right.

    int modelZoomLevel = m_model->getSummaryBlockSize(zoomLevel);

    sv_frame_t frame0;
    sv_frame_t frame1;
    sv_frame_t spare;

    getSourceFramesForX(v, x0, modelZoomLevel, frame0, spare);
    getSourceFramesForX(v, x1, modelZoomLevel, spare, frame1);
    
#ifdef DEBUG_WAVEFORM_PAINT
    cerr << "Painting waveform from " << frame0 << " to " << frame1 << " (" << (x1-x0+1) << " pixels at zoom " << zoomLevel << " and model zoom " << modelZoomLevel << ")" <<  endl;
#endif

    RangeSummarisableTimeValueModel::RangeBlock *ranges =
            new RangeSummarisableTimeValueModel::RangeBlock;

    RangeSummarisableTimeValueModel::RangeBlock *otherChannelRanges = 0;
    RangeSummarisableTimeValueModel::Range range;

    QColor baseColour = getBaseQColor();
    std::vector<QColor> greys = getPartialShades(v);

    QColor midColour = baseColour;
    if (midColour == Qt::black) {
        midColour = Qt::gray;
    } else if (v->hasLightBackground()) {
        midColour = midColour.light(150);
    } else {
        midColour = midColour.light(50);
    }

    while ((int)m_effectiveGains.size() <= maxChannel) {
        m_effectiveGains.push_back(m_gain);
    }

    for (int ch = minChannel; ch <= maxChannel; ++ch) {

        int prevRangeBottom = -1, prevRangeTop = -1;
        QColor prevRangeBottomColour = baseColour, prevRangeTopColour = baseColour;

        m_effectiveGains[ch] = m_gain;

        if (m_autoNormalize) {
            m_effectiveGains[ch] = getNormalizeGain(v, ch);
        }

        double gain = m_effectiveGains[ch];

        int m = (h / channels) / 2;
        int my = m + (((ch - minChannel) * h) / channels);

#ifdef DEBUG_WAVEFORM_PAINT	
        cerr << "ch = " << ch << ", channels = " << channels << ", m = " << m << ", my = " << my << ", h = " << h << endl;
#endif

        if (my - m > y1 || my + m < y0) continue;

        if ((m_scale == dBScale || m_scale == MeterScale) &&
                m_channelMode != MergeChannels) {
            m = (h / channels);
            my = m + (((ch - minChannel) * h) / channels);
        }

        paint->setPen(greys[1]);
        paint->drawLine(x0, my, x1, my);

        int n = 10;
        int py = -1;
        
        if (v->hasLightBackground() &&
                v->getViewManager() &&
                v->getViewManager()->shouldShowScaleGuides()) {

            paint->setPen(QColor(240, 240, 240));

            for (int i = 1; i < n; ++i) {
                
                double val = 0.0, nval = 0.0;

                switch (m_scale) {

                case LinearScale:
                    val = (i * gain) / n;
                    if (i > 0) nval = -val;
                    break;

                case MeterScale:
                    val = AudioLevel::dB_to_multiplier(meterdbs[i]) * gain;
                    break;

                case dBScale:
                    val = AudioLevel::dB_to_multiplier(-(10*n) + i * 10) * gain;
                    break;
                }

                if (val < -1.0 || val > 1.0) continue;

                int y = getYForValue(v, val, ch);

                if (py >= 0 && abs(y - py) < 10) continue;
                else py = y;

                int ny = y;
                if (nval != 0.0) {
                    ny = getYForValue(v, nval, ch);
                }

                paint->drawLine(x0, y, x1, y);
                if (ny != y) {
                    paint->drawLine(x0, ny, x1, ny);
                }
            }
        }

        m_model->getSummaries(ch, frame0, frame1 - frame0,
                              *ranges, modelZoomLevel);

#ifdef DEBUG_WAVEFORM_PAINT
        cerr << "channel " << ch << ": " << ranges->size() << " ranges from " << frame0 << " to " << frame1 << " at zoom level " << modelZoomLevel << endl;
#endif

        if (mergingChannels || mixingChannels) {
            if (m_model->getChannelCount() > 1) {
                if (!otherChannelRanges) {
                    otherChannelRanges =
                            new RangeSummarisableTimeValueModel::RangeBlock;
                }
                m_model->getSummaries
                        (1, frame0, frame1 - frame0, *otherChannelRanges,
                         modelZoomLevel);
            } else {
                if (otherChannelRanges != ranges) delete otherChannelRanges;
                otherChannelRanges = ranges;
            }
        }

        for (int x = x0; x <= x1; ++x) {

            range = RangeSummarisableTimeValueModel::Range();

            sv_frame_t f0, f1;
            if (!getSourceFramesForX(v, x, modelZoomLevel, f0, f1)) continue;
            f1 = f1 - 1;

            if (f0 < frame0) {
                cerr << "ERROR: WaveformLayer::paint: pixel " << x << " has f0 = " << f0 << " which is less than range frame0 " << frame0 << " for x0 = " << x0 << endl;
                continue;
            }

            sv_frame_t i0 = (f0 - frame0) / modelZoomLevel;
            sv_frame_t i1 = (f1 - frame0) / modelZoomLevel;

#ifdef DEBUG_WAVEFORM_PAINT
            cerr << "WaveformLayer::paint: pixel " << x << ": i0 " << i0 << " (f " << f0 << "), i1 " << i1 << " (f " << f1 << ")" << endl;
#endif

            if (i1 > i0 + 1) {
                cerr << "WaveformLayer::paint: ERROR: i1 " << i1 << " > i0 " << i0 << " plus one (zoom = " << zoomLevel << ", model zoom = " << modelZoomLevel << ")" << endl;
            }

            if (ranges && i0 < (int)ranges->size()) {

                range = (*ranges)[i0];

                if (i1 > i0 && i1 < (int)ranges->size()) {
                    range.setMax(std::max(range.max(), (*ranges)[i1].max()));
                    range.setMin(std::min(range.min(), (*ranges)[i1].min()));
                    range.setAbsmean((range.absmean() + (*ranges)[i1].absmean()) / 2);
                }

            } else {
#ifdef DEBUG_WAVEFORM_PAINT
                cerr << "No (or not enough) ranges for i0 = " << i0 << endl;
#endif
                continue;
            }

            int rangeBottom = 0, rangeTop = 0, meanBottom = 0, meanTop = 0;

            if (mergingChannels) {

                if (otherChannelRanges && i0 < (int)otherChannelRanges->size()) {

                    range.setMax(fabsf(range.max()));
                    range.setMin(-fabsf((*otherChannelRanges)[i0].max()));
                    range.setAbsmean
                            ((range.absmean() +
                              (*otherChannelRanges)[i0].absmean()) / 2);

                    if (i1 > i0 && i1 < (int)otherChannelRanges->size()) {
                        // let's not concern ourselves about the mean
                        range.setMin
                                (std::min
                                 (range.min(),
                                  -fabsf((*otherChannelRanges)[i1].max())));
                    }
                }

            } else if (mixingChannels) {

                if (otherChannelRanges && i0 < (int)otherChannelRanges->size()) {

                    range.setMax((range.max() + (*otherChannelRanges)[i0].max()) / 2);
                    range.setMin((range.min() + (*otherChannelRanges)[i0].min()) / 2);
                    range.setAbsmean((range.absmean() + (*otherChannelRanges)[i0].absmean()) / 2);
                }
            }

            int greyLevels = 1;
            if (m_greyscale && (m_scale == LinearScale)) greyLevels = 4;

            switch (m_scale) {

            case LinearScale:
                rangeBottom = int(double(m * greyLevels) * range.min() * gain);
                rangeTop    = int(double(m * greyLevels) * range.max() * gain);
                meanBottom  = int(double(-m) * range.absmean() * gain);
                meanTop     = int(double(m) * range.absmean() * gain);
                break;

            case dBScale:
                if (!mergingChannels) {
                    int db0 = dBscale(range.min() * gain, m);
                    int db1 = dBscale(range.max() * gain, m);
                    rangeTop    = std::max(db0, db1);
                    meanTop     = std::min(db0, db1);
                    if (mixingChannels) rangeBottom = meanTop;
                    else rangeBottom = dBscale(range.absmean() * gain, m);
                    meanBottom  = rangeBottom;
                } else {
                    rangeBottom = -dBscale(range.min() * gain, m * greyLevels);
                    rangeTop    =  dBscale(range.max() * gain, m * greyLevels);
                    meanBottom  = -dBscale(range.absmean() * gain, m);
                    meanTop     =  dBscale(range.absmean() * gain, m);
                }
                break;

            case MeterScale:
                if (!mergingChannels) {
                    int r0 = abs(AudioLevel::multiplier_to_preview(range.min() * gain, m));
                    int r1 = abs(AudioLevel::multiplier_to_preview(range.max() * gain, m));
                    rangeTop    = std::max(r0, r1);
                    meanTop     = std::min(r0, r1);
                    if (mixingChannels) rangeBottom = meanTop;
                    else rangeBottom = AudioLevel::multiplier_to_preview(range.absmean() * gain, m);
                    meanBottom  = rangeBottom;
                } else {
                    rangeBottom = -AudioLevel::multiplier_to_preview(range.min() * gain, m * greyLevels);
                    rangeTop    =  AudioLevel::multiplier_to_preview(range.max() * gain, m * greyLevels);
                    meanBottom  = -AudioLevel::multiplier_to_preview(range.absmean() * gain, m);
                    meanTop     =  AudioLevel::multiplier_to_preview(range.absmean() * gain, m);
                }
                break;
            }

            rangeBottom = my * greyLevels - rangeBottom;
            rangeTop    = my * greyLevels - rangeTop;
            meanBottom  = my - meanBottom;
            meanTop     = my - meanTop;

            int topFill = (rangeTop % greyLevels);
            if (topFill > 0) topFill = greyLevels - topFill;

            int bottomFill = (rangeBottom % greyLevels);

            rangeTop = rangeTop / greyLevels;
            rangeBottom = rangeBottom / greyLevels;

            bool clipped = false;

            if (rangeTop < my - m) { rangeTop = my - m; }
            if (rangeTop > my + m) { rangeTop = my + m; }
            if (rangeBottom < my - m) { rangeBottom = my - m; }
            if (rangeBottom > my + m) { rangeBottom = my + m; }

            if (range.max() <= -1.0 ||
                    range.max() >= 1.0) clipped = true;

            if (meanBottom > rangeBottom) meanBottom = rangeBottom;
            if (meanTop < rangeTop) meanTop = rangeTop;

            bool drawMean = m_showMeans;
            if (meanTop == rangeTop) {
                if (meanTop < meanBottom) ++meanTop;
                else drawMean = false;
            }
            if (meanBottom == rangeBottom && m_scale == LinearScale) {
                if (meanBottom > meanTop) --meanBottom;
                else drawMean = false;
            }

            if (x != x0 && prevRangeBottom != -1) {
                if (prevRangeBottom > rangeBottom + 1 &&
                        prevRangeTop    > rangeBottom + 1) {
                    //		    paint->setPen(midColour);
                    paint->setPen(baseColour);
                    paint->drawLine(x-1, prevRangeTop, x, rangeBottom + 1);
                    paint->setPen(prevRangeTopColour);
                    paint->drawPoint(x-1, prevRangeTop);
                } else if (prevRangeBottom < rangeTop - 1 &&
                           prevRangeTop    < rangeTop - 1) {
                    //		    paint->setPen(midColour);
                    paint->setPen(baseColour);
                    paint->drawLine(x-1, prevRangeBottom, x, rangeTop - 1);
                    paint->setPen(prevRangeBottomColour);
                    paint->drawPoint(x-1, prevRangeBottom);
                }
            }

            if (ready) {
                if (clipped /*!!! ||
                    range.min() * gain <= -1.0 ||
                    range.max() * gain >=  1.0 */) {
                    paint->setPen(Qt::red); //!!! getContrastingColour
                } else {
                    paint->setPen(baseColour);
                }
            } else {
                paint->setPen(midColour);
            }

#ifdef DEBUG_WAVEFORM_PAINT
            cerr << "range " << rangeBottom << " -> " << rangeTop << ", means " << meanBottom << " -> " << meanTop << ", raw range " << range.min() << " -> " << range.max() << endl;
#endif

            if (rangeTop == rangeBottom) {
                paint->drawPoint(x, rangeTop);
            } else {
                paint->drawLine(x, rangeBottom, x, rangeTop);
            }

            prevRangeTopColour = baseColour;
            prevRangeBottomColour = baseColour;

            if (m_greyscale && (m_scale == LinearScale) && ready) {
                if (!clipped) {
                    if (rangeTop < rangeBottom) {
                        if (topFill > 0 &&
                                (!drawMean || (rangeTop < meanTop - 1))) {
                            paint->setPen(greys[topFill - 1]);
                            paint->drawPoint(x, rangeTop);
                            prevRangeTopColour = greys[topFill - 1];
                        }
                        if (bottomFill > 0 &&
                                (!drawMean || (rangeBottom > meanBottom + 1))) {
                            paint->setPen(greys[bottomFill - 1]);
                            paint->drawPoint(x, rangeBottom);
                            prevRangeBottomColour = greys[bottomFill - 1];
                        }
                    }
                }
            }

            if (drawMean) {
                paint->setPen(midColour);
                paint->drawLine(x, meanBottom, x, meanTop);
            }

            prevRangeBottom = rangeBottom;
            prevRangeTop = rangeTop;
        }
    }

    if (m_middleLineHeight != 0.5) {
        paint->restore();
    }

    if (m_aggressive) {

        if (ready && rect == v->rect()) {
            m_cacheValid = true;
            m_cacheZoomLevel = zoomLevel;
        }
        paint->end();
        delete paint;
        viewPainter.drawPixmap(rect, *m_cache, rect);
    }

    if (otherChannelRanges != ranges) delete otherChannelRanges;
    delete ranges;
}

QString
WaveformLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->isOK()) return "";

    int zoomLevel = v->getZoomLevel();

    int modelZoomLevel = m_model->getSummaryBlockSize(zoomLevel);

    sv_frame_t f0, f1;
    if (!getSourceFramesForX(v, x, modelZoomLevel, f0, f1)) return "";
    
    QString text;

    RealTime rt0 = RealTime::frame2RealTime(f0, m_model->getSampleRate());
    RealTime rt1 = RealTime::frame2RealTime(f1, m_model->getSampleRate());

    if (f1 != f0 + 1 && (rt0.sec != rt1.sec || rt0.msec() != rt1.msec())) {
        text += tr("Time:\t%1 - %2")
                .arg(rt0.toText(true).c_str())
                .arg(rt1.toText(true).c_str());
    } else {
        text += tr("Time:\t%1")
                .arg(rt0.toText(true).c_str());
    }

    int channels = 0, minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    channels = getChannelArrangement(minChannel, maxChannel,
                                     mergingChannels, mixingChannels);
    if (channels == 0) return "";

    for (int ch = minChannel; ch <= maxChannel; ++ch) {

        int blockSize = v->getZoomLevel();
        RangeSummarisableTimeValueModel::RangeBlock ranges;
        m_model->getSummaries(ch, f0, f1 - f0, ranges, blockSize);

        if (ranges.empty()) continue;

        RangeSummarisableTimeValueModel::Range range = ranges[0];

        QString label = tr("Level:");
        if (minChannel != maxChannel) {
            if (ch == 0) label = tr("Left:");
            else if (ch == 1) label = tr("Right:");
            else label = tr("Channel %1").arg(ch + 1);
        }

        bool singleValue = false;
        double min, max;

        if (fabs(range.min()) < 0.01) {
            min = range.min();
            max = range.max();
            singleValue = (min == max);
        } else {
            int imin = int(lrint(range.min() * 10000));
            int imax = int(lrint(range.max() * 10000));
            singleValue = (imin == imax);
            min = double(imin)/10000;
            max = double(imax)/10000;
        }

        int db = int(AudioLevel::multiplier_to_dB(std::max(fabsf(range.min()),
                                                           fabsf(range.max())))
                     * 100);

        if (!singleValue) {
            text += tr("\n%1\t%2 - %3 (%4 dB peak)")
                    .arg(label).arg(min).arg(max).arg(double(db)/100);
        } else {
            text += tr("\n%1\t%2 (%3 dB peak)")
                    .arg(label).arg(min).arg(double(db)/100);
        }
    }

    return text;
}

int
WaveformLayer::getYForValue(const View *v, double value, int channel) const
{
    int channels = 0, minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    channels = getChannelArrangement(minChannel, maxChannel,
                                     mergingChannels, mixingChannels);
    if (channels == 0) return 0;
    if (maxChannel < minChannel || channel < minChannel) return 0;

    int h = v->height();
    int m = (h / channels) / 2;

    if ((m_scale == dBScale || m_scale == MeterScale) &&
            m_channelMode != MergeChannels) {
        m = (h / channels);
    }

    int my = m + (((channel - minChannel) * h) / channels);

    int vy = 0;

    switch (m_scale) {

    case LinearScale:
        vy = int(m * value);
        break;

    case MeterScale:
        vy = AudioLevel::multiplier_to_preview(value, m);
        break;

    case dBScale:
        vy = dBscale(value, m);
        break;
    }

    //    cerr << "mergingChannels= " << mergingChannels << ", channel  = " << channel << ", value = " << value << ", vy = " << vy << endl;

    return my - vy;
}

double
WaveformLayer::getValueForY(const View *v, int y, int &channel) const
{
    int channels = 0, minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    channels = getChannelArrangement(minChannel, maxChannel,
                                     mergingChannels, mixingChannels);
    if (channels == 0) return 0;
    if (maxChannel < minChannel) return 0;

    int h = v->height();
    int m = (h / channels) / 2;

    if ((m_scale == dBScale || m_scale == MeterScale) &&
            m_channelMode != MergeChannels) {
        m = (h / channels);
    }

    channel = (y * channels) / h + minChannel;

    int my = m + (((channel - minChannel) * h) / channels);

    int vy = my - y;
    double value = 0;
    double thresh = -50.f;

    switch (m_scale) {

    case LinearScale:
        value = double(vy) / m;
        break;

    case MeterScale:
        value = AudioLevel::preview_to_multiplier(vy, m);
        break;

    case dBScale:
        value = (-thresh * double(vy)) / m + thresh;
        value = AudioLevel::dB_to_multiplier(value);
        break;
    }

    return value / m_gain;
}

bool
WaveformLayer::getYScaleValue(const View *v, int y,
                              double &value, QString &unit) const
{
    int channel;

    value = getValueForY(v, y, channel);

    if (m_scale == dBScale || m_scale == MeterScale) {

        double thresh = -50.f;
        
        if (value > 0.0) {
            value = 10.0 * log10(value);
            if (value < thresh) value = thresh;
        } else value = thresh;

        unit = "dBV";

    } else {
        unit = "V";
    }

    return true;
}

bool
WaveformLayer::getYScaleDifference(const View *v, int y0, int y1,
                                   double &diff, QString &unit) const
{
    int c0, c1;
    double v0 = getValueForY(v, y0, c0);
    double v1 = getValueForY(v, y1, c1);

    if (c0 != c1) {
        // different channels, not comparable
        diff = 0.0;
        unit = "";
        return false;
    }

    if (m_scale == dBScale || m_scale == MeterScale) {

        double thresh = -50.0;

        if (v1 == v0) diff = thresh;
        else {
            if (v1 > v0) diff = v0 / v1;
            else diff = v1 / v0;

            diff = 10.0 * log10(diff);
            if (diff < thresh) diff = thresh;
        }

        unit = "dBV";

    } else {
        diff = fabs(v1 - v0);
        unit = "V";
    }

    return true;
}

int
WaveformLayer::getVerticalScaleWidth(View *, bool, QPainter &paint) const
{
    if (m_scale == LinearScale) {
        return paint.fontMetrics().width("0.0") + 13;
    } else {
        return std::max(paint.fontMetrics().width(tr("0dB")),
                        paint.fontMetrics().width(tr("-Inf"))) + 13;
    }
}

void
WaveformLayer::paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) {
        return;
    }

    int channels = 0, minChannel = 0, maxChannel = 0;
    bool mergingChannels = false, mixingChannels = false;

    channels = getChannelArrangement(minChannel, maxChannel,
                                     mergingChannels, mixingChannels);
    if (channels == 0) return;

    int h = rect.height(), w = rect.width();
    int textHeight = paint.fontMetrics().height();
    int toff = -textHeight/2 + paint.fontMetrics().ascent() + 1;

    double gain = m_gain;

    for (int ch = minChannel; ch <= maxChannel; ++ch) {

        int lastLabelledY = -1;

        if (ch < (int)m_effectiveGains.size()) gain = m_effectiveGains[ch];

        int n = 10;

        for (int i = 0; i <= n; ++i) {

            double val = 0.0, nval = 0.0;
            QString text = "";

            switch (m_scale) {

            case LinearScale:
                val = (i * gain) / n;
                text = QString("%1").arg(double(i) / n);
                if (i == 0) text = "0.0";
                else {
                    nval = -val;
                    if (i == n) text = "1.0";
                }
                break;

            case MeterScale:
                val = AudioLevel::dB_to_multiplier(meterdbs[i]) * gain;
                text = QString("%1").arg(meterdbs[i]);
                if (i == n) text = tr("0dB");
                if (i == 0) {
                    text = tr("-Inf");
                    val = 0.0;
                }
                break;

            case dBScale:
                val = AudioLevel::dB_to_multiplier(-(10*n) + i * 10) * gain;
                text = QString("%1").arg(-(10*n) + i * 10);
                if (i == n) text = tr("0dB");
                if (i == 0) {
                    text = tr("-Inf");
                    val = 0.0;
                }
                break;
            }

            if (val < -1.0 || val > 1.0) continue;

            int y = getYForValue(v, val, ch);

            int ny = y;
            if (nval != 0.0) {
                ny = getYForValue(v, nval, ch);
            }

            bool spaceForLabel = (i == 0 ||
                                  abs(y - lastLabelledY) >= textHeight - 1);

            if (spaceForLabel) {

                int tx = 3;
                if (m_scale != LinearScale) {
                    tx = w - 10 - paint.fontMetrics().width(text);
                }

                int ty = y;
                if (ty < paint.fontMetrics().ascent()) {
                    ty = paint.fontMetrics().ascent();
                } else if (ty > h - paint.fontMetrics().descent()) {
                    ty = h - paint.fontMetrics().descent();
                } else {
                    ty += toff;
                }
                paint.drawText(tx, ty, text);

                lastLabelledY = ty - toff;

                if (ny != y) {
                    ty = ny;
                    if (ty < paint.fontMetrics().ascent()) {
                        ty = paint.fontMetrics().ascent();
                    } else if (ty > h - paint.fontMetrics().descent()) {
                        ty = h - paint.fontMetrics().descent();
                    } else {
                        ty += toff;
                    }
                    paint.drawText(tx, ty, text);
                }

                paint.drawLine(w - 7, y, w, y);
                if (ny != y) paint.drawLine(w - 7, ny, w, ny);

            } else {

                paint.drawLine(w - 4, y, w, y);
                if (ny != y) paint.drawLine(w - 4, ny, w, ny);
            }
        }
    }
}

void
WaveformLayer::toXml(QTextStream &stream,
                     QString indent, QString extraAttributes) const
{
    QString s;
    
    QString colourName, colourSpec, darkbg;
    ColourDatabase::getInstance()->getStringValues
            (m_colour, colourName, colourSpec, darkbg);

    s += QString("gain=\"%1\" "
                 "showMeans=\"%2\" "
                 "greyscale=\"%3\" "
                 "channelMode=\"%4\" "
                 "channel=\"%5\" "
                 "scale=\"%6\" "
                 "middleLineHeight=\"%7\" "
                 "aggressive=\"%8\" "
                 "autoNormalize=\"%9\"")
            .arg(m_gain)
            .arg(m_showMeans)
            .arg(m_greyscale)
            .arg(m_channelMode)
            .arg(m_channel)
            .arg(m_scale)
            .arg(m_middleLineHeight)
            .arg(m_aggressive)
            .arg(m_autoNormalize);

    SingleColourLayer::toXml(stream, indent, extraAttributes + " " + s);
}

void
WaveformLayer::setProperties(const QXmlAttributes &attributes)
{
    bool ok = false;

    SingleColourLayer::setProperties(attributes);

    float gain = attributes.value("gain").toFloat(&ok);
    if (ok) setGain(gain);

    bool showMeans = (attributes.value("showMeans") == "1" ||
                      attributes.value("showMeans") == "true");
    setShowMeans(showMeans);

    bool greyscale = (attributes.value("greyscale") == "1" ||
                      attributes.value("greyscale") == "true");
    setUseGreyscale(greyscale);

    ChannelMode channelMode = (ChannelMode)
            attributes.value("channelMode").toInt(&ok);
    if (ok) setChannelMode(channelMode);

    int channel = attributes.value("channel").toInt(&ok);
    if (ok) setChannel(channel);

    Scale scale = (Scale)attributes.value("scale").toInt(&ok);
    if (ok) setScale(scale);

    float middleLineHeight = attributes.value("middleLineHeight").toFloat(&ok);
    if (ok) setMiddleLineHeight(middleLineHeight);

    bool aggressive = (attributes.value("aggressive") == "1" ||
                       attributes.value("aggressive") == "true");
    setUseGreyscale(aggressive);

    bool autoNormalize = (attributes.value("autoNormalize") == "1" ||
                          attributes.value("autoNormalize") == "true");
    setAutoNormalize(autoNormalize);
}

int
WaveformLayer::getVerticalZoomSteps(int &defaultStep) const
{
    defaultStep = 50;
    return 100;
}

int
WaveformLayer::getCurrentVerticalZoomStep() const
{
    int val = int(lrint(log10(m_gain) * 20.0) + 50);
    if (val < 0) val = 0;
    if (val > 100) val = 100;
    return val;
}

void
WaveformLayer::setVerticalZoomStep(int step)
{
    setGain(powf(10, float(step - 50) / 20.f));
}

