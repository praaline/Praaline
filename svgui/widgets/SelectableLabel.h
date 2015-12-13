/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _SELECTABLE_LABEL_H_
#define _SELECTABLE_LABEL_H_

#include <QLabel>

class SelectableLabel : public QLabel
{
    Q_OBJECT

public:
    SelectableLabel(QWidget *parent = 0);
    virtual ~SelectableLabel();

    void setSelectedText(QString);
    void setUnselectedText(QString);

    bool isSelected() const { return m_selected; }

signals:
    void selectionChanged();
    void doubleClicked();

public slots:
    void setSelected(bool);
    void toggle();

protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    void setupStyle();
    QString m_selectedText;
    QString m_unselectedText;
    bool m_selected;
    bool m_swallowRelease;
};

#endif
