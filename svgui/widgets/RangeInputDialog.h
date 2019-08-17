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

#ifndef _RANGE_INPUT_DIALOG_H_
#define _RANGE_INPUT_DIALOG_H_

#include <QDialog>
#include <QString>

class QDoubleSpinBox;

class RangeInputDialog : public QDialog
{
    Q_OBJECT

public:
    RangeInputDialog(QString title, QString message, QString unit,
                     double min, double max, QWidget *parent = nullptr);
    virtual ~RangeInputDialog();

    void getRange(double &start, double &end);
    
signals:
    void rangeChanged(double start, double end);

public slots:
    void setRange(double start, double end);

protected slots:
    void rangeStartChanged(double);
    void rangeEndChanged(double);

protected:
    QDoubleSpinBox *m_rangeStart;
    QDoubleSpinBox *m_rangeEnd;
};

#endif
