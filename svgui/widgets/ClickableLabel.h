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

#ifndef _CLICKABLE_LABEL_H_
#define _CLICKABLE_LABEL_H_

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(const QString &text, QWidget *parent = nullptr) :
        QLabel(text, parent) { }
    ClickableLabel(QWidget *parent = nullptr) : QLabel(parent) { }
    ~ClickableLabel() { }

signals:
    void clicked();

protected:
    virtual void mousePressEvent(QMouseEvent *) {
        emit clicked();
    }
};

#endif
