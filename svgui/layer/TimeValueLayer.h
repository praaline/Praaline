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

#ifndef _TIME_VALUE_LAYER_H_
#define _TIME_VALUE_LAYER_H_

#include "SingleColourLayer.h"
#include "VerticalScaleLayer.h"
#include "ColourScaleLayer.h"

#include "data/model/SparseTimeValueModel.h"

#include <QObject>
#include <QColor>

class View;
class QPainter;

class TimeValueLayer : public SingleColourLayer, 
        public VerticalScaleLayer,
        public ColourScaleLayer
{
    Q_OBJECT

public:
    enum PlotStyle {
        PlotPoints,
        PlotStems,
        PlotConnectedPoints,
        PlotLines,
        PlotCurve,
        PlotSegmentation,
        PlotDiscreteCurves
    };

    enum VerticalScale {
        AutoAlignScale,
        LinearScale,
        LogScale,
        PlusMinusOneScale
    };


    TimeValueLayer();

    std::string getType() const { return "TimeValues"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(SparseTimeValueModel *model);
    static Model *createEmptyModel(Model *baseModel);

    virtual void paint(View *v, QPainter &paint, QRect rect) const;


    virtual int getVerticalScaleWidth(View *v, bool, QPainter &) const;
    virtual void paintVerticalScale(View *v, bool, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;
    virtual QString getLabelPreceding(sv_frame_t) const;

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y = 0) const;
    virtual bool snapToSimilarFeature(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y = 0) const;

    virtual void drawStart(View *v, QMouseEvent *);
    virtual void drawDrag(View *v, QMouseEvent *);
    virtual void drawEnd(View *v, QMouseEvent *);

    virtual void eraseStart(View *v, QMouseEvent *);
    virtual void eraseDrag(View *v, QMouseEvent *);
    virtual void eraseEnd(View *v, QMouseEvent *);

    virtual void editStart(View *v, QMouseEvent *);
    virtual void editDrag(View *v, QMouseEvent *);
    virtual void editEnd(View *v, QMouseEvent *);

    virtual bool editOpen(View *v, QMouseEvent *);

    virtual void moveSelection(Selection s, sv_frame_t newStartFrame);
    virtual void resizeSelection(Selection s, Selection newSize);
    virtual void deleteSelection(Selection s);

    virtual void copy(View *v, Selection s, Clipboard &to);
    virtual bool paste(View *v, const Clipboard &from, sv_frame_t frameOffset,
                       bool interactive);

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual QString getPropertyIconName(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
                                          int value) const;
    virtual void setProperty(const PropertyName &, int value);

    void setFillColourMap(int);
    int getFillColourMap() const { return m_colourMap; }

    void setPlotStyle(PlotStyle style);
    PlotStyle getPlotStyle() const { return m_plotStyle; }

    void setVerticalScale(VerticalScale scale);
    VerticalScale getVerticalScale() const { return m_verticalScale; }

    void setDrawSegmentDivisions(bool);
    bool getDrawSegmentDivisions() const { return m_drawSegmentDivisions; }

    void setShowDerivative(bool);
    bool getShowDerivative() const { return m_derivative; }

    void setShowConfidenceInterval(bool);
    bool getShowConfidenceInterval() const { return m_confidenceInterval; }

    virtual bool isLayerScrollable(const View *v) const;

    virtual bool isLayerEditable() const { return true; }

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual bool needsTextLabelHeight() const {
        return m_plotStyle == PlotSegmentation && m_model->hasTextLabels();
    }

    virtual bool getValueExtents(double &min, double &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getDisplayExtents(double &min, double &max) const;
    virtual bool setDisplayExtents(double min, double max);

    virtual int getVerticalZoomSteps(int &defaultStep) const;
    virtual int getCurrentVerticalZoomStep() const;
    virtual void setVerticalZoomStep(int);
    virtual RangeMapper *getNewVerticalZoomRangeMapper() const;

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

    virtual ColourSignificance getLayerColourSignificance() const {
        if (m_plotStyle == PlotSegmentation) {
            return ColourHasMeaningfulValue;
        } else {
            return ColourDistinguishes;
        }
    }

    /// VerticalScaleLayer and ColourScaleLayer methods
    virtual int getYForValue(View *, double value) const;
    virtual double getValueForY(View *, int y) const;
    virtual QString getScaleUnits() const;
    virtual QColor getColourForValue(View *v, double value) const;

protected:
    void getScaleExtents(View *, double &min, double &max, bool &log) const;
    bool shouldAutoAlign() const;

    SparseTimeValueModel::PointList getLocalPoints(View *v, int) const;

    virtual int getDefaultColourHint(bool dark, bool &impose);

    SparseTimeValueModel *m_model;
    bool m_editing;
    SparseTimeValueModel::Point m_originalPoint;
    SparseTimeValueModel::Point m_editingPoint;
    SparseTimeValueModel::EditCommand *m_editingCommand;
    int m_colourMap;
    PlotStyle m_plotStyle;
    VerticalScale m_verticalScale;
    bool m_drawSegmentDivisions;
    bool m_derivative;
    bool m_confidenceInterval;

    mutable double m_scaleMinimum;
    mutable double m_scaleMaximum;

    void finish(SparseTimeValueModel::EditCommand *command) {
        UndoableCommand *c = command->finish();
        if (c) CommandHistory::getInstance()->addCommand(c, false);
    }
};

#endif

