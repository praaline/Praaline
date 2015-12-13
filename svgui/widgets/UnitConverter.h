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

#ifndef UNIT_CONVERTER_H
#define UNIT_CONVERTER_H

#include <QDialog>

#include "base/PropertyContainer.h"

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QLabel;

class UnitConverter : public QDialog
{
    Q_OBJECT

public:
    UnitConverter(QWidget *parent = 0);
    virtual ~UnitConverter();

private slots:
    void freqChanged();
    void midiChanged();
    void noteChanged();
    void octaveChanged();
    void centsChanged();

    void samplesChanged();
    void periodChanged();
    void bpmChanged();
    void tempofreqChanged();
    void samplerateChanged();
    
    void preferenceChanged(PropertyContainer::PropertyName);
    
private:
    QDoubleSpinBox *m_freq;
    QSpinBox *m_midi;
    QComboBox *m_note;
    QSpinBox *m_octave;
    QDoubleSpinBox *m_cents;
    QLabel *m_pitchPrefsLabel;
    void updatePitchesFromFreq();
    void updatePitchPrefsLabel();

    QDoubleSpinBox *m_samples;
    QDoubleSpinBox *m_period;
    QDoubleSpinBox *m_bpm;
    QDoubleSpinBox *m_tempofreq;
    QComboBox *m_samplerate;
    void updateTempiFromSamples();

    double getSampleRate();

    void setTo(QSpinBox *, int);
    void setTo(QDoubleSpinBox *, double);
};

#endif
