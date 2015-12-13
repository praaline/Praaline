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

#ifndef _COLOUR_3D_PLOT_H_
#define _COLOUR_3D_PLOT_H_

#include "SliceableLayer.h"

#include "data/model/DenseThreeDimensionalModel.h"

class View;
class QPainter;
class QImage;

/**
 * This is a view that displays dense 3-D data (time, some sort of
 * binned y-axis range, value) as a colour plot with value mapped to
 * colour range.  Its source is a DenseThreeDimensionalModel.
 *
 * This was the original implementation for the spectrogram view, but
 * it was replaced with a more efficient implementation that derived
 * the spectrogram itself from a DenseTimeValueModel instead of using
 * a three-dimensional model.  This class is retained in case it
 * becomes useful, but it will probably need some cleaning up if it's
 * ever actually used.
 */

class Colour3DPlotLayer : public SliceableLayer
{
    Q_OBJECT

public:
    Colour3DPlotLayer();
    ~Colour3DPlotLayer();

    std::string getType() const { return "Colour3DPlot"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(const DenseThreeDimensionalModel *model);

    virtual const ZoomConstraint *getZoomConstraint() const {
        return m_model ? m_model->getZoomConstraint() : 0;
    }

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual int getVerticalScaleWidth(View *v, bool, QPainter &) const;
    virtual void paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, 
				    int &resolution,
				    SnapType snap) const;

    virtual void setLayerDormant(const View *v, bool dormant);

    virtual bool isLayerScrollable(const View *v) const;

    virtual ColourSignificance getLayerColourSignificance() const {
        return ColourHasMeaningfulValue;
    }

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual PropertyList getProperties() const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual QString getPropertyIconName(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual RangeMapper *getNewPropertyRangeMapper(const PropertyName &) const;
    virtual void setProperty(const PropertyName &, int value);
    virtual void setProperties(const QXmlAttributes &);
    
    enum ColourScale {
        LinearScale,
        LogScale,
        PlusMinusOneScale,
        AbsoluteScale
    };

    void setColourScale(ColourScale);
    ColourScale getColourScale() const { return m_colourScale; }

    void setColourMap(int map);
    int getColourMap() const;

    /**
     * Set the gain multiplier for sample values in this view.
     * The default is 1.0.
     */
    void setGain(float gain);
    float getGain() const;

    enum BinScale {
	LinearBinScale,
	LogBinScale
    };
    
    /**
     * Specify the scale for the y axis.
     */
    void setBinScale(BinScale);
    BinScale getBinScale() const;

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

    void setInvertVertical(bool i);
    bool getInvertVertical() const;

    void setOpaque(bool i);
    bool getOpaque() const;

    void setSmooth(bool i);
    bool getSmooth() const;

    virtual bool getValueExtents(double &min, double &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getDisplayExtents(double &min, double &max) const;
    virtual bool setDisplayExtents(double min, double max);

    virtual bool getYScaleValue(const View *, int /* y */,
                                double &/* value */, QString &/* unit */) const;

    virtual int getVerticalZoomSteps(int &defaultStep) const;
    virtual int getCurrentVerticalZoomStep() const;
    virtual void setVerticalZoomStep(int);
    virtual RangeMapper *getNewVerticalZoomRangeMapper() const;

    virtual const Model *getSliceableModel() const { return m_model; }

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

protected slots:
    void cacheInvalid();
    void cacheInvalid(sv_frame_t startFrame, sv_frame_t endFrame);
    void modelChanged();
    void modelChangedWithin(sv_frame_t, sv_frame_t);

protected:
    const DenseThreeDimensionalModel *m_model; // I do not own this
    
    mutable QImage *m_cache;
    mutable QImage *m_peaksCache;
    mutable int m_cacheValidStart;
    mutable int m_cacheValidEnd;

    ColourScale m_colourScale;
    bool        m_colourScaleSet;
    int         m_colourMap;
    float       m_gain;
    BinScale    m_binScale;
    bool        m_normalizeColumns;
    bool        m_normalizeVisibleArea;
    bool        m_normalizeHybrid;
    bool        m_invertVertical;
    bool        m_opaque;
    bool        m_smooth;
    int         m_peakResolution;

    // Minimum and maximum bin numbers visible within the view. We
    // always snap to whole bins at view edges.
    int         m_miny;
    int         m_maxy;

    /**
     * Return the y coordinate at which the given bin "starts"
     * (i.e. at the bottom of the bin, if the given bin is an integer
     * and the vertical scale is the usual way up). Bin number may be
     * fractional, to obtain a position part-way through a bin.
     */
    double getYForBin(View *, double bin) const;

    /**
     * As getYForBin, but rounding to integer values.
     */
    int getIYForBin(View *, int bin) const;
    
    /**
     * Return the bin number, possibly fractional, at the given y
     * coordinate. Note that the whole numbers occur at the positions
     * at which the bins "start" (i.e. the bottom of the visible bin,
     * if the vertical scale is the usual way up).
     */
    double getBinForY(View *, double y) const;

    /**
     * As getBinForY, but rounding to integer values.
     */
    int getIBinForY(View *, int y) const;
    
    DenseThreeDimensionalModel::Column getColumn(int col) const;

    /**
     * True if we have the opaque or smooth flag set, or if the cells
     * are so small you can't see their borders. False for big,
     * translucent cells.
     */
    bool shouldPaintDenseIn(const View *) const; 

    int getColourScaleWidth(QPainter &) const;
    void fillCache(int firstBin, int lastBin) const;
    void paintDense(View *v, QPainter &paint, QRect rect) const;
};

#endif
