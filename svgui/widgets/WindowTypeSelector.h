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

#ifndef _WINDOW_TYPE_SELECTOR_H_
#define _WINDOW_TYPE_SELECTOR_H_

#include <QFrame>

#include "base/Window.h"

class WindowShapePreview;
class QComboBox;

class WindowTypeSelector : public QFrame
{
    Q_OBJECT

public:
    WindowTypeSelector(WindowType defaultType);
    WindowTypeSelector(); // get window type from preferences
    virtual ~WindowTypeSelector();

    WindowType getWindowType() const;

signals:
    void windowTypeChanged(WindowType type);

public slots:
    void setWindowType(WindowType type);

protected slots:
    void windowIndexChanged(int index);

protected:
    QComboBox *m_windowCombo;
    WindowShapePreview *m_windowShape;
    WindowType *m_windows;
    WindowType m_windowType;

    void init(WindowType type);
};

#endif
