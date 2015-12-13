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

#ifndef _SPECTROGRAM_LAYER_H_
#define _SPECTROGRAM_LAYER_H_

#include "SliceableLayer.h"
#include "base/Window.h"
#include "base/RealTime.h"
#include "base/Thread.h"
#include "base/PropertyContainer.h"
#include "data/model/PowerOfSqrtTwoZoomConstraint.h"
#include "data/model/DenseTimeValueModel.h"
#include "data/model/FFTModel.h"

#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QPixmap>

class View;
class QPainter;
class QImage;
class QPixmap;
class QTimer;
class FFTModel;
class Dense3DModelPeakCache;


/**
 * SpectrogramLayer represents waveform data (obtained from a
 * DenseTimeValueModel) in spectrogram form.
 */

class SpectrogramLayer : public SliceableLayer,
        public PowerOfSqrtTwoZoomConstraint
{
    Q_OBJECT

public:
    enum Configuration { FullRangeDb, MelodicRange, MelodicPeaks };
    
    SpectrogramLayer(Configuration = FullRangeDb);
    ~SpectrogramLayer();

    std::string getType() const { return "Spectrogram"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(const DenseTimeValueModel *model);

    virtual const ZoomConstraint *getZoomConstraint() const { return this; }
    virtual void paint(View *v, QPainter &paint, QRect rect) const;
    virtual void setSynchronousPainting(bool synchronous);

    virtual int getVerticalScaleWidth(View *v, bool detailed, QPainter &) const;
    virtual void paintVerticalScale(View *v, bool detailed, QPainter &paint, QRect rect) const;

    virtual bool getCrosshairExtents(View *, QPainter &, QPoint cursorPos,
                                     std::vector<QRect> &extents) const;
    virtual void paintCrosshairs(View *, QPainter &, QPoint) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame,
                                    int &resolution,
                                    SnapType snap) const;

    virtual void measureDoubleClick(View *, QMouseEvent *);

    virtual bool hasLightBackground() const;

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
     * Specify the channel to use from the source model.
     * A value of -1 means to mix all available channels.
     * The default is channel 0.
     */
    virtual void setChannel(int);
    virtual int getChannel() const;

    void setWindowSize(int);
    int getWindowSize() const;
    
    void setWindowHopLevel(int level);
    int getWindowHopLevel() const;

    void setWindowType(WindowType type);
    WindowType getWindowType() const;

    void setZeroPadLevel(int level);
    int getZeroPadLevel() const;

    /**
     * Set the gain multiplier for sample values in this view.
     * The default is 1.0.
     */
    void setGain(float gain);
    float getGain() const;

    /**
     * Set the threshold for sample values to qualify for being shown
     * in the FFT, in voltage units.
     *
     * The default is 0.0.
     */
    void setThreshold(float threshold);
    float getThreshold() const;

    void setMinFrequency(int);
    int getMinFrequency() const;

    void setMaxFrequency(int); // 0 -> no maximum
    int getMaxFrequency() const;

    enum ColourScale {
        LinearColourScale,
        MeterColourScale,
        dBSquaredColourScale,
        dBColourScale,
        PhaseColourScale
    };

    /**
     * Specify the scale for sample levels.  See WaveformLayer for
     * details of meter and dB scaling.  The default is dBColourScale.
     */
    void setColourScale(ColourScale);
    ColourScale getColourScale() const;

    enum FrequencyScale {
        LinearFrequencyScale,
        LogFrequencyScale
    };
    
    /**
     * Specify the scale for the y axis.
     */
    void setFrequencyScale(FrequencyScale);
    FrequencyScale getFrequencyScale() const;

    enum BinDisplay {
        AllBins,
        PeakBins,
        PeakFrequencies
    };
    
    /**
     * Specify the processing of frequency bins for the y axis.
     */
    void setBinDisplay(BinDisplay);
    BinDisplay getBinDisplay() const;

    /**
     * Normalize each column to its maximum value, independent of its
     * neighbours.
     */
    void setNormalizeColumns(bool n);
    bool getNormalizeColumns() const;

    /**
     * Normalize each value against the maximum in the visible region.
     */
    void setNormalizeVisibleArea(bool n);
    bool getNormalizeVisibleArea() const;

    /**
     * Normalize each column to its maximum value, and then scale by
     * the log of the (absolute) maximum value.
     */
    void setNormalizeHybrid(bool n);
    bool getNormalizeHybrid() const;
    
    void setColourMap(int map);
    int getColourMap() const;

    /**
     * Specify the colourmap rotation for the colour scale.
     */
    void setColourRotation(int);
    int getColourRotation() const;

    virtual VerticalPosition getPreferredFrameCountPosition() const {
        return PositionTop;
    }

    virtual bool isLayerOpaque() const { return true; }

    virtual ColourSignificance getLayerColourSignificance() const {
        return ColourHasMeaningfulValue;
    }

    double getYForFrequency(const View *v, double frequency) const;
    double getFrequencyForY(const View *v, int y) const;

    virtual int getCompletion(View *v) const;
    virtual QString getError(View *v) const;

    virtual bool getValueExtents(double &min, double &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getDisplayExtents(double &min, double &max) const;

    virtual bool setDisplayExtents(double min, double max);

    virtual bool getYScaleValue(const View *, int, double &, QString &) const;

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

    virtual void setLayerDormant(const View *v, bool dormant);

    virtual bool isLayerScrollable(const View *) const { return false; }

    virtual int getVerticalZoomSteps(int &defaultStep) const;
    virtual int getCurrentVerticalZoomStep() const;
    virtual void setVerticalZoomStep(int);
    virtual RangeMapper *getNewVerticalZoomRangeMapper() const;

    virtual const Model *getSliceableModel() const;

protected slots:
    void cacheInvalid();
    void cacheInvalid(sv_frame_t startFrame, sv_frame_t endFrame);
    
    void preferenceChanged(PropertyContainer::PropertyName name);

    void fillTimerTimedOut();

protected:
    const DenseTimeValueModel *m_model; // I do not own this

    int                 m_channel;
    int              m_windowSize;
    WindowType          m_windowType;
    int              m_windowHopLevel;
    int              m_zeroPadLevel;
    int              m_fftSize;
    float               m_gain;
    float               m_initialGain;
    float               m_threshold;
    float               m_initialThreshold;
    int                 m_colourRotation;
    int                 m_initialRotation;
    int              m_minFrequency;
    int              m_maxFrequency;
    int              m_initialMaxFrequency;
    ColourScale         m_colourScale;
    int                 m_colourMap;
    QColor              m_crosshairColour;
    FrequencyScale      m_frequencyScale;
    BinDisplay          m_binDisplay;
    bool                m_normalizeColumns;
    bool                m_normalizeVisibleArea;
    bool                m_normalizeHybrid;
    int                 m_lastEmittedZoomStep;
    bool                m_synchronous;

    mutable bool        m_haveDetailedScale;
    mutable int         m_lastPaintBlockWidth;
    mutable RealTime    m_lastPaintTime;

    enum { NO_VALUE = 0 }; // colour index for unused pixels

    class Palette
    {
    public:
        QColor getColour(unsigned char index) const {
            return m_colours[index];
        }

        void setColour(unsigned char index, QColor colour) {
            m_colours[index] = colour;
        }

    private:
        QColor m_colours[256];
    };

    Palette m_palette;

    /**
     * ImageCache covers the area of the view, at view resolution.
     * Not all of it is necessarily valid at once (it is refreshed
     * in parts when scrolling, for example).
     */
    struct ImageCache
    {
        QImage image;
        QRect validArea;
        sv_frame_t startFrame;
        int zoomLevel;
    };
    typedef std::map<const View *, ImageCache> ViewImageCache;
    void invalidateImageCaches();
    void invalidateImageCaches(sv_frame_t startFrame, sv_frame_t endFrame);
    mutable ViewImageCache m_imageCaches;

    /**
     * When painting, we draw directly onto the draw buffer and then
     * copy this to the part of the image cache that needed refreshing
     * before copying the image cache onto the window.  (Remind me why
     * we don't draw directly onto the cache?)
     */
    mutable QImage m_drawBuffer;

    mutable QTimer *m_updateTimer;

    mutable sv_frame_t m_candidateFillStartFrame;
    bool m_exiting;

    void initialisePalette();
    void rotatePalette(int distance);

    unsigned char getDisplayValue(View *v, double input) const;

    int getColourScaleWidth(QPainter &) const;

    void illuminateLocalFeatures(View *v, QPainter &painter) const;

    double getEffectiveMinFrequency() const;
    double getEffectiveMaxFrequency() const;

    // Note that the getYBin... methods return the nominal bin in the
    // un-smoothed spectrogram.  This is not necessarily the same bin
    // as is pulled from the spectrogram and drawn at the given
    // position, if the spectrogram has oversampling smoothing.  Use
    // getSmoothedYBinRange to obtain that.

    bool getXBinRange(View *v, int x, double &windowMin, double &windowMax) const;
    bool getYBinRange(View *v, int y, double &freqBinMin, double &freqBinMax) const;
    bool getSmoothedYBinRange(View *v, int y, double &freqBinMin, double &freqBinMax) const;

    bool getYBinSourceRange(View *v, int y, double &freqMin, double &freqMax) const;
    bool getAdjustedYBinSourceRange(View *v, int x, int y,
                                    double &freqMin, double &freqMax,
                                    double &adjFreqMin, double &adjFreqMax) const;
    bool getXBinSourceRange(View *v, int x, RealTime &timeMin, RealTime &timeMax) const;
    bool getXYBinSourceRange(View *v, int x, int y, double &min, double &max,
                             double &phaseMin, double &phaseMax) const;

    int getWindowIncrement() const {
        if (m_windowHopLevel == 0) return m_windowSize;
        else if (m_windowHopLevel == 1) return (m_windowSize * 3) / 4;
        else return m_windowSize / (1 << (m_windowHopLevel - 1));
    }

    int getZeroPadLevel(const View *v) const;
    int getFFTSize(const View *v) const;
    FFTModel *getFFTModel(const View *v) const;
    Dense3DModelPeakCache *getPeakCache(const View *v) const;
    void invalidateFFTModels();

    typedef std::pair<FFTModel *, sv_frame_t> FFTFillPair; // model, last fill
    typedef std::map<const View *, FFTFillPair> ViewFFTMap;
    typedef std::map<const View *, Dense3DModelPeakCache *> PeakCacheMap;
    mutable ViewFFTMap m_fftModels;
    mutable PeakCacheMap m_peakCaches;
    mutable Model *m_sliceableModel;

    class MagnitudeRange {
    public:
        MagnitudeRange() : m_min(0), m_max(0) { }
        bool operator==(const MagnitudeRange &r) {
            return r.m_min == m_min && r.m_max == m_max;
        }
        bool isSet() const { return (m_min != 0.f || m_max != 0.f); }
        void set(float min, float max) {
            m_min = min;
            m_max = max;
            if (m_max < m_min) m_max = m_min;
        }
        bool sample(float f) {
            bool changed = false;
            if (isSet()) {
                if (f < m_min) { m_min = f; changed = true; }
                if (f > m_max) { m_max = f; changed = true; }
            } else {
                m_max = m_min = f;
                changed = true;
            }
            return changed;
        }
        bool sample(const MagnitudeRange &r) {
            bool changed = false;
            if (isSet()) {
                if (r.m_min < m_min) { m_min = r.m_min; changed = true; }
                if (r.m_max > m_max) { m_max = r.m_max; changed = true; }
            } else {
                m_min = r.m_min;
                m_max = r.m_max;
                changed = true;
            }
            return changed;
        }
        float getMin() const { return m_min; }
        float getMax() const { return m_max; }
    private:
        float m_min;
        float m_max;
    };

    typedef std::map<const View *, MagnitudeRange> ViewMagMap;
    mutable ViewMagMap m_viewMags;
    mutable std::vector<MagnitudeRange> m_columnMags;
    void invalidateMagnitudes();
    bool updateViewMagnitudes(View *v) const;
    bool paintDrawBuffer(View *v, int w, int h,
                         const std::vector<int> &binforx,
                         const std::vector<double> &binfory,
                         bool usePeaksCache,
                         MagnitudeRange &overallMag,
                         bool &overallMagChanged) const;
    bool paintDrawBufferPeakFrequencies(View *v, int w, int h,
                                        const std::vector<int> &binforx,
                                        int minbin,
                                        int maxbin,
                                        double displayMinFreq,
                                        double displayMaxFreq,
                                        bool logarithmic,
                                        MagnitudeRange &overallMag,
                                        bool &overallMagChanged) const;

    virtual void updateMeasureRectYCoords(View *v, const MeasureRect &r) const;
    virtual void setMeasureRectYCoord(View *v, MeasureRect &r, bool start, int y) const;
};

#endif
