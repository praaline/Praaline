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

#ifndef _OVERVIEW_H_
#define _OVERVIEW_H_

#include "View.h"

#include <QPoint>
#include <QTime>

class QWidget;
class QPaintEvent;
class Layer;
class View;

#include <map>

class Overview : public View
{
    Q_OBJECT

public:
    Overview(QWidget *parent = 0);

    void registerView(View *view);
    void unregisterView(View *view);

    virtual QString getPropertyContainerIconName() const { return "panner"; }

public slots:
    virtual void modelChangedWithin(sv_frame_t startFrame, sv_frame_t endFrame);
    virtual void modelReplaced();

    virtual void globalCentreFrameChanged(sv_frame_t);
    virtual void viewCentreFrameChanged(View *, sv_frame_t);
    virtual void viewZoomLevelChanged(View *, int, bool);
    virtual void viewManagerPlaybackFrameChanged(sv_frame_t);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual bool shouldLabelSelections() const { return false; }

    QColor getFillWithin() const;
    QColor getFillWithout() const;
    
    QPoint m_clickPos;
    QPoint m_mousePos;
    bool m_clickedInRange;
    sv_frame_t m_dragCentreFrame;
    QTime m_modelTestTime;
    
    typedef std::set<View *> ViewSet;
    ViewSet m_views;
};

#endif

