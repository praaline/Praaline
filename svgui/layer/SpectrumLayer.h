
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _SPECTRUM_LAYER_H_
#define _SPECTRUM_LAYER_H_

#include "SliceLayer.h"
#include "LayerFactory.h"

#include "base/Window.h"

#include "data/model/DenseTimeValueModel.h"

#include <QColor>
#include <QMutex>

class FFTModel;

class SpectrumLayer : public SliceLayer
{
    Q_OBJECT

public:
    SpectrumLayer();
    ~SpectrumLayer();

    std::string getType() const { return "Spectrum"; }


    virtual const Model *getModel() const { return m_originModel; }
    virtual bool trySetModel(Model *);
    void setModel(DenseTimeValueModel *model);

    virtual bool getCrosshairExtents(View *, QPainter &, QPoint cursorPos,
                                     std::vector<QRect> &extents) const;
    virtual void paintCrosshairs(View *, QPainter &, QPoint) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual VerticalPosition getPreferredFrameCountPosition() const {
	return PositionTop;
    }

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
    virtual void setProperties(const QXmlAttributes &);

    virtual bool getValueExtents(double &min, double &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getXScaleValue(const View *v, int x,
                                double &value, QString &unit) const;

    virtual bool getYScaleValue(const View *, int y,
                                double &value, QString &unit) const;

    virtual bool getYScaleDifference(const View *, int y0, int y1,
                                     double &diff, QString &unit) const;

    virtual bool isLayerScrollable(const View *) const { return false; }

    virtual void setChannel(int);
    virtual int getChannel() const { return m_channel; }

    void setWindowSize(int);
    int getWindowSize() const { return m_windowSize; }
    
    void setWindowHopLevel(int level);
    int getWindowHopLevel() const { return m_windowHopLevel; }

    void setWindowType(WindowType type);
    WindowType getWindowType() const { return m_windowType; }

    void setShowPeaks(bool);
    bool getShowPeaks() const { return m_showPeaks; }

    virtual int getVerticalScaleWidth(View *, bool, QPainter &) const { return 0; }

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

protected slots:
    void preferenceChanged(PropertyContainer::PropertyName name);

protected:
    // make this SliceLayer method unavailable to the general public
//    virtual void setModel(DenseThreeDimensionalModel *model) {
//        SliceLayer::setModel(model);
//    }

    DenseTimeValueModel    *m_originModel;
    int                     m_channel;
    bool                    m_channelSet;
    int                  m_windowSize;
    WindowType              m_windowType;
    int                  m_windowHopLevel;
    bool                    m_showPeaks;
    mutable bool            m_newFFTNeeded;

    mutable QMutex m_fftMutex;

    void setupFFT();

    virtual void getBiasCurve(BiasCurve &) const;
    BiasCurve m_biasCurve;

    virtual double getXForBin(int bin, int totalBins, double w) const;
    virtual int getBinForX(double x, int totalBins, double w) const;

    double getFrequencyForX(double x, double w) const;
    double getXForFrequency(double freq, double w) const;

    int getWindowIncrement() const {
        if (m_windowHopLevel == 0) return m_windowSize;
        else if (m_windowHopLevel == 1) return (m_windowSize * 3) / 4;
        else return m_windowSize / (1 << (m_windowHopLevel - 1));
    }
};

#endif
