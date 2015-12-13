/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _NOTIFYING_TAB_BAR_H_
#define _NOTIFYING_TAB_BAR_H_

#include <QTabBar>

class NotifyingTabBar : public QTabBar
{
    Q_OBJECT

public:
    NotifyingTabBar(QWidget *parent = 0);
    virtual ~NotifyingTabBar();
    
signals:
    void mouseEntered();
    void mouseLeft();
    void activeTabClicked();

protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
};

#endif
