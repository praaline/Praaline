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

#ifndef _REGION_LAYER_H_
#define _REGION_LAYER_H_

#include "SingleColourLayer.h"
#include "VerticalScaleLayer.h"
#include "ColourScaleLayer.h"

#include "data/model/RegionModel.h"

#include <QObject>
#include <QColor>

#include <map>

class View;
class QPainter;

class RegionLayer : public SingleColourLayer,
                    public VerticalScaleLayer,
                    public ColourScaleLayer
{
    Q_OBJECT

public:
    RegionLayer();

    std::string getType() const { return "Regions"; }

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(RegionModel *model);
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
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);

    void setFillColourMap(int);
    int getFillColourMap() const { return m_colourMap; }

    enum VerticalScale {
        AutoAlignScale,
        EqualSpaced,
        LinearScale,
        LogScale,
    };

    void setVerticalScale(VerticalScale scale);
    VerticalScale getVerticalScale() const { return m_verticalScale; }

    enum PlotStyle {
	PlotLines,
	PlotSegmentation
    };

    void setPlotStyle(PlotStyle style);
    PlotStyle getPlotStyle() const { return m_plotStyle; }

    virtual bool isLayerScrollable(const View *v) const;

    virtual bool isLayerEditable() const { return true; }

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual bool getValueExtents(double &min, double &max,
                                 bool &log, QString &unit) const;

    virtual bool getDisplayExtents(double &min, double &max) const;

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

    /// VerticalScaleLayer and ColourScaleLayer methods
    int getYForValue(View *v, double value) const;
    double getValueForY(View *v, int y) const;
    virtual QString getScaleUnits() const;
    QColor getColourForValue(View *v, double value) const;

protected slots:
    void recalcSpacing();

protected:
    double getValueForY(View *v, int y, int avoid) const;
    void getScaleExtents(View *, double &min, double &max, bool &log) const;

    virtual int getDefaultColourHint(bool dark, bool &impose);

    RegionModel::PointList getLocalPoints(View *v, int x) const;

    bool getPointToDrag(View *v, int x, int y, RegionModel::Point &) const;

    RegionModel *m_model;
    bool m_editing;
    int m_dragPointX;
    int m_dragPointY;
    int m_dragStartX;
    int m_dragStartY;
    RegionModel::Point m_originalPoint;
    RegionModel::Point m_editingPoint;
    RegionModel::EditCommand *m_editingCommand;
    VerticalScale m_verticalScale;
    int m_colourMap;
    PlotStyle m_plotStyle;

    typedef std::map<double, int> SpacingMap;

    // region value -> ordering
    SpacingMap m_spacingMap;

    // region value -> number of regions with this value
    SpacingMap m_distributionMap;

    int spacingIndexToY(View *v, int i) const;
    double yToSpacingIndex(View *v, int y) const;

    void finish(RegionModel::EditCommand *command) {
        UndoableCommand *c = command->finish();
        if (c) CommandHistory::getInstance()->addCommand(c, false);
    }
};

#endif

