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

#ifndef _TIME_RULER_H_
#define _TIME_RULER_H_

#include "SingleColourLayer.h"

#include <QRect>
#include <QColor>

class View;
class Model;
class QPainter;

class TimeRulerLayer : public SingleColourLayer
{
    Q_OBJECT

public:
    TimeRulerLayer();

    std::string getType() const { return "TimeRuler"; }

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual const Model *getModel() const { return m_model; }
    virtual bool trySetModel(Model *);
    void setModel(Model *);

    enum LabelHeight { LabelTop, LabelMiddle, LabelBottom };
    void setLabelHeight(LabelHeight h) { m_labelHeight = h; }
    LabelHeight getLabelHeight() const { return m_labelHeight; }

    virtual bool snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y = 0) const;

    virtual ColourSignificance getLayerColourSignificance() const {
        return ColourIrrelevant;
    }

    virtual bool getValueExtents(double &, double &, bool &, QString &) const {
        return false;
    }

    virtual QString getLayerPresentationName() const;

    virtual int getVerticalScaleWidth(View *, bool, QPainter &) const { return 0; }

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

protected:
    Model *m_model;
    LabelHeight m_labelHeight;

    virtual int getDefaultColourHint(bool dark, bool &impose);

    int getMajorTickSpacing(View *, bool &quarterTicks) const;
};

#endif
