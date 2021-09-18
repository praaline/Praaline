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

#include "RangeInputDialog.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

RangeInputDialog::RangeInputDialog(QString title, QString message,
                                   QString unit, double min, double max,
                                   QWidget *parent) :
    QDialog(parent)
{
    QGridLayout *grid = new QGridLayout;
    setLayout(grid);

    setWindowTitle(title);
    
    QLabel *messageLabel = new QLabel;
    messageLabel->setText(message);
    grid->addWidget(messageLabel, 0, 0, 1, 5);

    m_rangeStart = new QDoubleSpinBox;
    m_rangeStart->setDecimals(4);
    m_rangeStart->setMinimum(min);
    m_rangeStart->setMaximum(max);
    m_rangeStart->setSuffix(unit);
    grid->addWidget(m_rangeStart, 1, 1);
    connect(m_rangeStart, SIGNAL(valueChanged(double)),
            this, SLOT(rangeStartChanged(double)));
    
    grid->addWidget(new QLabel(tr(" to ")), 1, 2);

    m_rangeEnd = new QDoubleSpinBox;
    m_rangeEnd->setDecimals(4);
    m_rangeEnd->setMinimum(min);
    m_rangeEnd->setMaximum(max);
    m_rangeEnd->setSuffix(unit);
    grid->addWidget(m_rangeEnd, 1, 3);
    connect(m_rangeEnd, SIGNAL(valueChanged(double)),
            this, SLOT(rangeEndChanged(double)));

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    grid->addWidget(bb, 2, 0, 1, 5);
    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

RangeInputDialog::~RangeInputDialog()
{
}

void
RangeInputDialog::getRange(double &min, double &max)
{
    min = double(m_rangeStart->value());
    max = double(m_rangeEnd->value());

    if (min > max) {
        double tmp = min;
        min = max;
        max = tmp;
    }
}

void
RangeInputDialog::setRange(double start, double end)
{
    if (start > end) {
        double tmp = start;
        start = end;
        end = tmp;
    }

    blockSignals(true);
    m_rangeStart->setValue(start);
    m_rangeEnd->setValue(end);
    blockSignals(false);
}

void
RangeInputDialog::rangeStartChanged(double min)
{
    double max = m_rangeEnd->value();
    if (min > max) {
        double tmp = min;
        min = max;
        max = tmp;
    }
    emit rangeChanged(double(min), double(max));
}


void
RangeInputDialog::rangeEndChanged(double max)
{
    double min = m_rangeStart->value();
    if (min > max) {
        double tmp = min;
        min = max;
        max = tmp;
    }
    emit rangeChanged(double(min), double(max));
}

