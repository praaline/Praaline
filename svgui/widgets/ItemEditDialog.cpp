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

#include "ItemEditDialog.h"

#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QDialogButtonBox>

#include <float.h> // for FLT_MIN/MAX


ItemEditDialog::ItemEditDialog(sv_samplerate_t sampleRate, int options,
                               QString valueUnits, QWidget *parent) :
    QDialog(parent),
    m_sampleRate(sampleRate),
    m_defaultFrame(0),
    m_defaultDuration(0),
    m_defaultValue(0),
    m_frameTimeSpinBox(0),
    m_realTimeSecsSpinBox(0),
    m_realTimeUSecsSpinBox(0),
    m_frameDurationSpinBox(0),
    m_realDurationSecsSpinBox(0),
    m_realDurationUSecsSpinBox(0),
    m_valueSpinBox(0),
    m_textField(0)
{
    QGridLayout *grid = new QGridLayout;
    setLayout(grid);

    QGroupBox *timeBox = 0;
    QGroupBox *valueBox = 0;
    QGridLayout *subgrid = 0;

    int row = 0, subrow = 0;

    int singleStep = RealTime::frame2RealTime(2, sampleRate).usec() - 1;

    if ((options & ShowTime) || (options & ShowDuration)) {

        timeBox = new QGroupBox;
        timeBox->setTitle(tr("Timing"));
        grid->addWidget(timeBox, row, 0);

        subgrid = new QGridLayout;
        timeBox->setLayout(subgrid);

        ++row;
    }

    if (options & ShowTime) {

        subgrid->addWidget(new QLabel(tr("Time:")), subrow, 0);

        m_frameTimeSpinBox = new QSpinBox;
        m_frameTimeSpinBox->setMaximum(INT_MAX);
        m_frameTimeSpinBox->setSuffix(tr(" frames"));
        subgrid->addWidget(m_frameTimeSpinBox, subrow, 1, 1, 2);
        connect(m_frameTimeSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(frameTimeChanged(int)));

        ++subrow;

        m_realTimeSecsSpinBox = new QSpinBox;
        m_realTimeSecsSpinBox->setMaximum(999999);
        m_realTimeSecsSpinBox->setSuffix(tr(" sec"));
        subgrid->addWidget(m_realTimeSecsSpinBox, subrow, 1);
        connect(m_realTimeSecsSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(realTimeSecsChanged(int)));

        m_realTimeUSecsSpinBox = new QSpinBox;
        m_realTimeUSecsSpinBox->setMaximum(999999);
        m_realTimeUSecsSpinBox->setSuffix(tr(" usec"));
        m_realTimeUSecsSpinBox->setSingleStep(singleStep);
        subgrid->addWidget(m_realTimeUSecsSpinBox, subrow, 2);
        connect(m_realTimeUSecsSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(realTimeUSecsChanged(int)));

        ++subrow;
    }

    if (options & ShowDuration) {

        subgrid->addWidget(new QLabel(tr("Duration:")), subrow, 0);

        m_frameDurationSpinBox = new QSpinBox;
        m_frameDurationSpinBox->setMaximum(INT_MAX);
        m_frameDurationSpinBox->setSuffix(tr(" frames"));
        subgrid->addWidget(m_frameDurationSpinBox, subrow, 1, 1, 2);
        connect(m_frameDurationSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(frameDurationChanged(int)));

        ++subrow;

        m_realDurationSecsSpinBox = new QSpinBox;
        m_realDurationSecsSpinBox->setMaximum(999999);
        m_realDurationSecsSpinBox->setSuffix(tr(" sec"));
        subgrid->addWidget(m_realDurationSecsSpinBox, subrow, 1);
        connect(m_realDurationSecsSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(realDurationSecsChanged(int)));

        m_realDurationUSecsSpinBox = new QSpinBox;
        m_realDurationUSecsSpinBox->setMaximum(999999);
        m_realDurationUSecsSpinBox->setSuffix(tr(" usec"));
        m_realDurationUSecsSpinBox->setSingleStep(singleStep);
        subgrid->addWidget(m_realDurationUSecsSpinBox, subrow, 2);
        connect(m_realDurationUSecsSpinBox, SIGNAL(valueChanged(int)),
                this, SLOT(realDurationUSecsChanged(int)));

        ++subrow;
    }

    if ((options & ShowValue) || (options & ShowText)) {

        valueBox = new QGroupBox;
        valueBox->setTitle(tr("Properties"));
        grid->addWidget(valueBox, row, 0);

        subgrid = new QGridLayout;
        valueBox->setLayout(subgrid);

        ++row;
    }

    subrow = 0;

    if (options & ShowValue) {
        
        subgrid->addWidget(new QLabel(tr("Value:")), subrow, 0);

        m_valueSpinBox = new QDoubleSpinBox;
        m_valueSpinBox->setSuffix(QString(" %1").arg(valueUnits));
        m_valueSpinBox->setDecimals(10);
        m_valueSpinBox->setMinimum(-1e10);
        m_valueSpinBox->setMaximum(1e10);
        connect(m_valueSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(valueChanged(double)));
        subgrid->addWidget(m_valueSpinBox, subrow, 1);

        ++subrow;
    }

    if (options & ShowText) {
        
        subgrid->addWidget(new QLabel(tr("Text:")), subrow, 0);

        m_textField = new QLineEdit;
        connect(m_textField, SIGNAL(textChanged(QString)),
                this, SLOT(textChanged(QString)));
        subgrid->addWidget(m_textField, subrow, 1);

        ++subrow;
    }

    if (options & ShowText) {
        m_textField->setFocus(Qt::OtherFocusReason);
    } else if (options & ShowValue) {
        m_valueSpinBox->setFocus(Qt::OtherFocusReason);
    }
    
    QDialogButtonBox *bb = new QDialogButtonBox(Qt::Horizontal);
    grid->addWidget(bb, row, 0, 1, 2);
    
    QPushButton *ok = new QPushButton(tr("OK"));
    m_resetButton = new QPushButton(tr("Reset"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    bb->addButton(ok, QDialogButtonBox::AcceptRole);
    bb->addButton(m_resetButton, QDialogButtonBox::ResetRole);
    bb->addButton(cancel, QDialogButtonBox::RejectRole);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_resetButton, SIGNAL(clicked()), this, SLOT(reset()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    m_resetButton->setEnabled(false);
}

void
ItemEditDialog::setFrameTime(sv_frame_t frame)
{
    if (!m_frameTimeSpinBox) return;

    RealTime rt(RealTime::frame2RealTime(frame, m_sampleRate));
    m_realTimeSecsSpinBox->setValue(rt.sec);
    m_realTimeUSecsSpinBox->setValue(rt.usec());
    m_frameTimeSpinBox->setValue(int(frame));
    m_defaultFrame = frame;
    m_resetButton->setEnabled(false);
}

sv_frame_t
ItemEditDialog::getFrameTime() const
{
    return m_frameTimeSpinBox->value();
}

void
ItemEditDialog::setRealTime(RealTime rt)
{
    setFrameTime(RealTime::realTime2Frame(rt, m_sampleRate));
}

RealTime
ItemEditDialog::getRealTime() const
{
    return RealTime::frame2RealTime(getFrameTime(), m_sampleRate);
}

void
ItemEditDialog::setFrameDuration(sv_frame_t duration)
{
    if (!m_frameDurationSpinBox) return;

    RealTime rt(RealTime::frame2RealTime(duration, m_sampleRate));
    m_realDurationSecsSpinBox->setValue(rt.sec);
    m_realDurationUSecsSpinBox->setValue(rt.usec());
    m_frameDurationSpinBox->setValue(int(duration));
    m_defaultDuration = duration;
    m_resetButton->setEnabled(false);
}

sv_frame_t
ItemEditDialog::getFrameDuration() const
{
    return m_frameDurationSpinBox->value();
}

void
ItemEditDialog::setRealDuration(RealTime rt)
{
    setFrameDuration(RealTime::realTime2Frame(rt, m_sampleRate));
}

RealTime
ItemEditDialog::getRealDuration() const
{
    return RealTime::frame2RealTime(getFrameDuration(), m_sampleRate);
}

void
ItemEditDialog::setValue(float v)
{
    if (!m_valueSpinBox) return;

    m_valueSpinBox->setValue(v);
    m_defaultValue = v;
    m_resetButton->setEnabled(false);
}

float
ItemEditDialog::getValue() const
{
    return float(m_valueSpinBox->value());
}

void
ItemEditDialog::setText(QString text)
{
    if (!m_textField) return;

    m_textField->setText(text);
    m_defaultText = text;
    m_resetButton->setEnabled(false);
}

QString
ItemEditDialog::getText() const
{
    return m_textField->text();
}

void
ItemEditDialog::frameTimeChanged(int i)
{
    m_realTimeSecsSpinBox->blockSignals(true);
    m_realTimeUSecsSpinBox->blockSignals(true);

    RealTime rt(RealTime::frame2RealTime(i, m_sampleRate));
    m_realTimeSecsSpinBox->setValue(rt.sec);
    m_realTimeUSecsSpinBox->setValue(rt.usec());

    m_realTimeSecsSpinBox->blockSignals(false);
    m_realTimeUSecsSpinBox->blockSignals(false);
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::realTimeSecsChanged(int i)
{
    RealTime rt = getRealTime();
    rt.sec = i;
    sv_frame_t frame = RealTime::realTime2Frame(rt, m_sampleRate);
    m_frameTimeSpinBox->setValue(int(frame));
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::realTimeUSecsChanged(int i)
{
    RealTime rt = getRealTime();
    rt.nsec = i * 1000;
    sv_frame_t frame = RealTime::realTime2Frame(rt, m_sampleRate);
    m_frameTimeSpinBox->setValue(int(frame));
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::frameDurationChanged(int i)
{
    m_realDurationSecsSpinBox->blockSignals(true);
    m_realDurationUSecsSpinBox->blockSignals(true);

    RealTime rt(RealTime::frame2RealTime(i, m_sampleRate));
    m_realDurationSecsSpinBox->setValue(rt.sec);
    m_realDurationUSecsSpinBox->setValue(rt.usec());

    m_realDurationSecsSpinBox->blockSignals(false);
    m_realDurationUSecsSpinBox->blockSignals(false);
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::realDurationSecsChanged(int i)
{
    RealTime rt = getRealDuration();
    rt.sec = i;
    sv_frame_t frame = RealTime::realTime2Frame(rt, m_sampleRate);
    m_frameDurationSpinBox->setValue(int(frame));
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::realDurationUSecsChanged(int i)
{
    RealTime rt = getRealDuration();
    rt.nsec = i * 1000;
    sv_frame_t frame = RealTime::realTime2Frame(rt, m_sampleRate);
    m_frameDurationSpinBox->setValue(int(frame));
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::valueChanged(double)
{
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::textChanged(QString)
{
    m_resetButton->setEnabled(true);
}

void
ItemEditDialog::reset()
{
    setFrameTime(m_defaultFrame);
    setFrameDuration(m_defaultDuration);
    setValue(m_defaultValue);
    setText(m_defaultText);
    m_resetButton->setEnabled(false);
}

