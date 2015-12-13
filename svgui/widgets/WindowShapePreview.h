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

#ifndef _WINDOW_SHAPE_PREVIEW_H_
#define _WINDOW_SHAPE_PREVIEW_H_

#include <QFrame>

#include "base/Window.h"

class QLabel;

class WindowShapePreview : public QFrame
{
    Q_OBJECT
    
public:
    WindowShapePreview(QWidget *parent = 0);
    virtual ~WindowShapePreview();

public slots:
    void setWindowType(WindowType type);

protected:
    QLabel *m_windowTimeExampleLabel;
    QLabel *m_windowFreqExampleLabel;
    WindowType m_windowType;

    void updateLabels();
};

#endif
