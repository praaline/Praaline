/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef LEVEL_PAN_WIDGET_H
#define LEVEL_PAN_WIDGET_H

#include <QWidget>

/**
 * A simple widget for coarse level and pan control.
 */

class LevelPanWidget : public QWidget
{
    Q_OBJECT

public:
    LevelPanWidget(QWidget *parent = 0);
    ~LevelPanWidget();
    
    /// Return level as a gain value in the range [0,1]
    float getLevel() const; 
    
    /// Return pan as a value in the range [-1,1]
    float getPan() const;

    /// Find out whether the widget is editable
    bool isEditable() const;

    /// Discover whether the level range includes muting or not
    bool includesMute() const;

    /// Draw a suitably sized copy of the widget's contents to the given device
    void renderTo(QPaintDevice *, QRectF, bool asIfEditable) const;

    QSize sizeHint() const;
                                               
public slots:
    /// Set level in the range [0,1] -- will be rounded
    void setLevel(float);

    /// Set pan in the range [-1,1] -- will be rounded
    void setPan(float);

    /// Specify whether the widget is editable or read-only (default editable)
    void setEditable(bool);

    /// Specify whether the level range should include muting or not
    void setIncludeMute(bool);
    
signals:
    void levelChanged(float);
    void panChanged(float);

protected:
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void wheelEvent(QWheelEvent *ev);
    virtual void paintEvent(QPaintEvent *ev);

    void emitLevelChanged();
    void emitPanChanged();
    
    int m_level;
    int m_pan;
    bool m_editable;
    bool m_includeMute;

    QSizeF cellSize(QRectF) const;
    QPointF cellCentre(QRectF, int level, int pan) const;
    QSizeF cellLightSize(QRectF) const;
    QRectF cellLightRect(QRectF, int level, int pan) const;
    double thinLineWidth(QRectF) const;
    void toCell(QRectF, QPointF loc, int &level, int &pan) const;
};

#endif
