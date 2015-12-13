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

#ifndef _WAVEFORM_LAYER_H_
#define _WAVEFORM_LAYER_H_

#include <QRect>

#include "SingleColourLayer.h"

#include "data/model/RangeSummarisableTimeValueModel.h"

class View;
class QPainter;
class QPixmap;

class WaveformLayer : public SingleColourLayer
{
    Q_OBJECT

public:
    WaveformLayer();
    ~WaveformLayer();

    std::string getType() const { return "Waveform"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(const RangeSummarisableTimeValueModel *model);

    virtual const ZoomConstraint *getZoomConstraint() const {
        return m_model ? m_model->getZoomConstraint() : 0;
    }

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual ColourSignificance getLayerColourSignificance() const {
        return ColourAndBackgroundSignificant;
    }

    virtual int getVerticalScaleWidth(View *v, bool detailed, QPainter &) const;
    virtual void paintVerticalScale(View *v, bool detailed, QPainter &paint, QRect rect) const;

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual QString getPropertyIconName(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual RangeMapper *getNewPropertyRangeMapper(const PropertyName &) const;
    virtual void setProperty(const PropertyName &, int value);

    /**
     * Set the gain multiplier for sample values in this view.
     *
     * The default is 1.0.
     */
    void setGain(float gain);
    float getGain() const { return m_gain; }

    /**
     * Toggle automatic normalization of the currently visible waveform.
     */
    void setAutoNormalize(bool);
    bool getAutoNormalize() const { return m_autoNormalize; }

    /**
     * Set whether to display mean values as a lighter-coloured area
     * beneath the peaks.  Rendering will be slightly faster without
     * but arguably prettier with.
     *
     * The default is to display means.
     */
    void setShowMeans(bool);
    bool getShowMeans() const { return m_showMeans; }

    /**
     * Set whether to use shades of grey (or of the base colour) to
     * provide additional perceived vertical resolution (i.e. using
     * half-filled pixels to represent levels that only just meet the
     * pixel unit boundary).  This provides a small improvement in
     * waveform quality at a small cost in rendering speed.
     * 
     * The default is to use greyscale.
     */
    void setUseGreyscale(bool);
    bool getUseGreyscale() const { return m_greyscale; }


    enum ChannelMode { SeparateChannels, MixChannels, MergeChannels };

    /**
     * Specify whether multi-channel audio data should be displayed
     * with a separate axis per channel (SeparateChannels), with a
     * single synthetic axis showing channel 0 above the axis and
     * channel 1 below (MergeChannels), or with a single axis showing
     * the average of the channels (MixChannels).
     * 
     * MergeChannels does not work for files with more than 2
     * channels.
     * 
     * The default is SeparateChannels.
     */
    void setChannelMode(ChannelMode);
    ChannelMode getChannelMode() const { return m_channelMode; }


    /**
     * Specify the channel to use from the source model.  A value of
     * -1 means to show all available channels (laid out to the
     * channel mode). The default is -1.
     */
    virtual void setChannel(int);
    virtual int getChannel() const { return m_channel; }


    enum Scale { LinearScale, MeterScale, dBScale };

    /**
     * Specify the vertical scale for sample levels.  With LinearScale,
     * the scale is directly proportional to the raw [-1, +1)
     * floating-point audio sample values.  With dBScale the
     * vertical scale is proportional to dB level (truncated at
     * -50dB).  MeterScale provides a hybrid variable scale based on
     * IEC meter scale, intended to provide a clear overview at
     * relatively small heights.
     *
     * Note that the effective gain (see setGain()) is applied before
     * vertical scaling.
     *
     * The default is LinearScale.
     */
    void setScale(Scale);
    Scale getScale() const { return m_scale; }

    /**
     * Specify the height of the middle of the waveform track or
     * tracks within the layer, from 0.0 to 1.0.
     *
     * A value of 0.0 would indicate that the waveform occupies
     * effectively no space at the very top of the layer; 1.0 would
     * indicate that the waveform occupies no space at the very
     * bottom; the default value of 0.5 indicates that it occupies the
     * whole layer, centred at the middle.
     */
    void setMiddleLineHeight(double);
    double getMiddleLineHeight() const { return m_middleLineHeight; }

    /**
     * Enable or disable aggressive pixmap cacheing.  If enabled,
     * waveforms will be rendered to an off-screen pixmap and
     * refreshed from there instead of being redrawn from the peak
     * data each time.  This may be faster if the data and zoom level
     * do not change often, but it may be slower for frequently zoomed
     * data and it will only work if the waveform is the "bottom"
     * layer on the displayed widget, as each refresh will erase
     * anything beneath the waveform.
     *
     * This is intended specifically for a panner widget display in
     * which the waveform never moves, zooms, or changes, but some
     * graphic such as a panner outline is frequently redrawn over the
     * waveform.  This situation would necessitate a lot of waveform
     * refresh if the default cacheing strategy was used.
     *
     * The default is not to use aggressive cacheing.
     */
    void setAggressiveCacheing(bool);
    bool getAggressiveCacheing() const { return m_aggressive; }

    virtual bool isLayerScrollable(const View *) const;

    virtual int getCompletion(View *) const;

    virtual bool getValueExtents(double &min, double &max,
                                 bool &log, QString &unit) const;

    virtual bool getYScaleValue(const View *v, int y,
                                double &value, QString &unit) const;
    
    virtual bool getYScaleDifference(const View *v, int y0, int y1,
                                     double &diff, QString &unit) const;

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    virtual void setProperties(const QXmlAttributes &attributes);

    virtual int getVerticalZoomSteps(int &defaultStep) const;
    virtual int getCurrentVerticalZoomStep() const;
    virtual void setVerticalZoomStep(int);

protected:
    int dBscale(double sample, int m) const;

    const RangeSummarisableTimeValueModel *m_model; // I do not own this

    /// Return value is number of channels displayed
    int getChannelArrangement(int &min, int &max,
                                 bool &merging, bool &mixing) const;

    int getYForValue(const View *v, double value, int channel) const;

    double getValueForY(const View *v, int y, int &channel) const;

    bool getSourceFramesForX(View *v, int x, int modelZoomLevel,
                             sv_frame_t &f0, sv_frame_t &f1) const;

    float getNormalizeGain(View *v, int channel) const;

    virtual void flagBaseColourChanged() { m_cacheValid = false; }

    float        m_gain;
    bool         m_autoNormalize;
    bool         m_showMeans;
    bool         m_greyscale;
    ChannelMode  m_channelMode;
    int          m_channel;
    Scale        m_scale;
    double       m_middleLineHeight;
    bool         m_aggressive;

    mutable std::vector<float> m_effectiveGains;

    mutable QPixmap *m_cache;
    mutable bool m_cacheValid;
    mutable int m_cacheZoomLevel;
};

#endif
