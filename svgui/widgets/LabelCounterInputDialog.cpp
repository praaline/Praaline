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

#include "LabelCounterInputDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>

LabelCounterInputDialog::LabelCounterInputDialog(Labeller *labeller,
						 QWidget *parent) :
    QDialog(parent),
    m_labeller(labeller)
{
    setWindowTitle(tr("Set Counters"));

    QGridLayout *layout = new QGridLayout(this);
    
    QLabel *label = new QLabel(tr("Fine counter (beats):"));
    layout->addWidget(label, 1, 0);

    label = new QLabel(tr("Coarse counter (bars):"));
    layout->addWidget(label, 0, 0);

    QSpinBox *counter = new QSpinBox;
    counter->setMinimum(-10);
    counter->setMaximum(10000);
    counter->setSingleStep(1);
    m_origSecondCounter = m_labeller->getSecondLevelCounterValue();
    counter->setValue(m_origSecondCounter);
    connect(counter, SIGNAL(valueChanged(int)),
            this, SLOT(secondCounterChanged(int)));
    layout->addWidget(counter, 0, 1);

    counter = new QSpinBox;
    counter->setMinimum(-10);
    counter->setMaximum(10000);
    counter->setSingleStep(1);
    m_origCounter = m_labeller->getCounterValue();
    counter->setValue(m_origCounter);
    connect(counter, SIGNAL(valueChanged(int)),
            this, SLOT(counterChanged(int)));
    layout->addWidget(counter, 1, 1);
    
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    layout->addWidget(bb, 2, 0, 1, 2);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(cancelClicked()));
}

LabelCounterInputDialog::~LabelCounterInputDialog()
{
}

void
LabelCounterInputDialog::counterChanged(int value)
{
    m_labeller->setCounterValue(value);
}

void
LabelCounterInputDialog::secondCounterChanged(int value)
{
    m_labeller->setSecondLevelCounterValue(value);
}

void
LabelCounterInputDialog::cancelClicked()
{
    m_labeller->setCounterValue(m_origCounter);
    m_labeller->setSecondLevelCounterValue(m_origSecondCounter);
    reject();
}

