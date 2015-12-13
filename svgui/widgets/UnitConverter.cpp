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

#include "UnitConverter.h"

#include <QSpinBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QTabWidget>

#include "base/Debug.h"
#include "base/Pitch.h"
#include "base/Preferences.h"

using namespace std;

static QString pianoNotes[] = {
    "C", "C# / Db", "D", "D# / Eb", "E",
    "F", "F# / Gb", "G", "G# / Ab", "A", "A# / Bb", "B"
};

UnitConverter::UnitConverter(QWidget *parent) :
    QDialog(parent)
{
    QGridLayout *maingrid = new QGridLayout;
    setLayout(maingrid);
    
    QTabWidget *tabs = new QTabWidget;
    maingrid->addWidget(tabs, 0, 0);
    
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    maingrid->addWidget(bb, 1, 0);
    connect(bb, SIGNAL(rejected()), this, SLOT(close()));
    
    QFrame *frame = new QFrame;
    tabs->addTab(frame, tr("Pitch"));
    
    QGridLayout *grid = new QGridLayout;
    frame->setLayout(grid);

    m_freq = new QDoubleSpinBox;
    m_freq->setSuffix(QString(" Hz"));
    m_freq->setDecimals(6);
    m_freq->setMinimum(1e-3);
    m_freq->setMaximum(1e6);
    m_freq->setValue(440);
    connect(m_freq, SIGNAL(valueChanged(double)),
	    this, SLOT(freqChanged()));

    // The min and max range values for all the remaining controls are
    // determined by the min and max Hz above
    
    m_midi = new QSpinBox;
    m_midi->setMinimum(-156);
    m_midi->setMaximum(203);
    connect(m_midi, SIGNAL(valueChanged(int)),
	    this, SLOT(midiChanged()));

    m_note = new QComboBox;
    for (int i = 0; i < 12; ++i) {
	m_note->addItem(pianoNotes[i]);
    }
    connect(m_note, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(noteChanged()));

    m_octave = new QSpinBox;
    m_octave->setMinimum(-14);
    m_octave->setMaximum(15);
    connect(m_octave, SIGNAL(valueChanged(int)),
	    this, SLOT(octaveChanged()));

    m_cents = new QDoubleSpinBox;
    m_cents->setSuffix(tr(" cents"));
    m_cents->setDecimals(4);
    m_cents->setMinimum(-50);
    m_cents->setMaximum(50);
    connect(m_cents, SIGNAL(valueChanged(double)),
	    this, SLOT(centsChanged()));
    
    int row = 0;

    grid->addWidget(new QLabel(tr("In 12-tone Equal Temperament:")), row, 0, 1, 9);

    ++row;
    
    grid->setRowMinimumHeight(row, 8);

    ++row;
    
    grid->addWidget(m_freq, row, 0, 2, 1, Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(new QLabel(tr("=")), row, 1, 2, 1, Qt::AlignHCenter | Qt::AlignVCenter);

    grid->addWidget(new QLabel(tr("+")), row, 7, 2, 1, Qt::AlignHCenter | Qt::AlignVCenter);
    grid->addWidget(m_cents, row, 8, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);

    grid->addWidget(new QLabel(tr("Piano note")), row, 2, 1, 2);
    grid->addWidget(m_note, row, 4);
    grid->addWidget(new QLabel(tr("in octave")), row, 5);
    grid->addWidget(m_octave, row, 6);

    ++row;
    
    grid->addWidget(new QLabel(tr("MIDI pitch")), row, 2, 1, 2);
    grid->addWidget(m_midi, row, 4);
    
    ++row;

    grid->setRowStretch(row, 20);
    grid->setRowMinimumHeight(row, 8);

    ++row;

    m_pitchPrefsLabel = new QLabel;
    grid->addWidget(m_pitchPrefsLabel, row, 0, 1, 9);

    ++row;
    
    grid->addWidget
	(new QLabel(tr("Note that only pitches in the range 0 to 127 are valid "
		       "in the MIDI protocol.")),
	 row, 0, 1, 9);

    ++row;
    
    frame = new QFrame;
    tabs->addTab(frame, tr("Tempo"));
    
    grid = new QGridLayout;
    frame->setLayout(grid);

    m_samples = new QDoubleSpinBox;
    m_samples->setSuffix(QString(" samples"));
    m_samples->setDecimals(2);
    m_samples->setMinimum(1);
    m_samples->setMaximum(1e8);
    m_samples->setValue(22050);
    connect(m_samples, SIGNAL(valueChanged(double)),
	    this, SLOT(samplesChanged()));
    
    m_period = new QDoubleSpinBox;
    m_period->setSuffix(QString(" ms"));
    m_period->setDecimals(4);
    m_period->setMinimum(1e-3);
    m_period->setMaximum(100000);
    m_period->setValue(500);
    connect(m_period, SIGNAL(valueChanged(double)),
	    this, SLOT(periodChanged()));

    m_bpm = new QDoubleSpinBox;
    m_bpm->setSuffix(QString(" bpm"));
    m_bpm->setDecimals(4);
    m_bpm->setMinimum(0.1);
    m_bpm->setMaximum(1e6);
    m_bpm->setValue(120);
    connect(m_bpm, SIGNAL(valueChanged(double)),
	    this, SLOT(bpmChanged()));

    m_tempofreq = new QDoubleSpinBox;
    m_tempofreq->setSuffix(QString(" beats/sec"));
    m_tempofreq->setDecimals(4);
    m_tempofreq->setMinimum(1e-3);
    m_tempofreq->setMaximum(1e5);
    m_tempofreq->setValue(0.5);

    connect(m_tempofreq, SIGNAL(valueChanged(double)),
	    this, SLOT(tempofreqChanged()));
	
    m_samplerate = new QComboBox;
    QList<int> rates;
    rates << 8000;
    for (int i = 1; i <= 16; i *= 2) {
	rates << 11025 * i << 12000 * i;
    }
    foreach (int r, rates) {
	m_samplerate->addItem(QString("%1 Hz").arg(r));
    }
    connect(m_samplerate, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(samplerateChanged()));
    m_samplerate->setCurrentText("44100 Hz");
    
    connect(Preferences::getInstance(),
	    SIGNAL(propertyChanged(PropertyContainer::PropertyName)),
	    this, SLOT(preferenceChanged(PropertyContainer::PropertyName)));

    row = 0;

    grid->setRowStretch(row, 20);
    grid->setRowMinimumHeight(row, 8);

    ++row;

    grid->addWidget(new QLabel(tr("Beat period")), row, 0, 2, 1, Qt::AlignVCenter);
    grid->addWidget(m_period, row, 1);
    grid->addWidget(new QLabel(tr("=")), row, 2, 2, 1, Qt::AlignVCenter);

    grid->addWidget(m_tempofreq, row, 3);

    grid->addWidget(new QLabel(tr("at")), row, 4, 2, 1, Qt::AlignVCenter);
    grid->addWidget(m_samplerate, row, 5, 2, 1, Qt::AlignVCenter);

    ++row;
    
    grid->addWidget(m_samples, row, 1);
    grid->addWidget(m_bpm, row, 3);

    ++row;
    
    grid->setRowStretch(row, 20);
    grid->setRowMinimumHeight(row, 8);
    
    updatePitchesFromFreq();
    updatePitchPrefsLabel();
    updateTempiFromSamples();
}

UnitConverter::~UnitConverter()
{
}

void
UnitConverter::setTo(QSpinBox *box, int value)
{
    box->blockSignals(true);
    if (value < box->minimum() || value > box->maximum()) {
	QPalette p;
	p.setColor(QPalette::Text, Qt::red);
	box->setPalette(p);
    } else {
	box->setPalette(QPalette());
    }
    box->setValue(value);
    box->blockSignals(false);
}

void
UnitConverter::setTo(QDoubleSpinBox *box, double value)
{
    box->blockSignals(true);
    if (value < box->minimum() || value > box->maximum()) {
	QPalette p;
	p.setColor(QPalette::Text, Qt::red);
	box->setPalette(p);
    } else {
	box->setPalette(QPalette());
    }
    box->setValue(value);
    box->blockSignals(false);
}

void
UnitConverter::preferenceChanged(PropertyContainer::PropertyName)
{
    updatePitchesFromFreq();
    updatePitchPrefsLabel();
}

void
UnitConverter::updatePitchPrefsLabel()
{
    m_pitchPrefsLabel->setText
	(tr("With concert-A tuning frequency at %1 Hz, and "
	    "middle C residing in octave %2.\n"
	    "(These can be changed in the application preferences.)")
	 .arg(Preferences::getInstance()->getTuningFrequency())
	 .arg(Preferences::getInstance()->getOctaveOfMiddleC()));
}

void
UnitConverter::freqChanged()
{
    updatePitchesFromFreq();
}

void
UnitConverter::midiChanged()
{
    double freq = Pitch::getFrequencyForPitch(m_midi->value(), m_cents->value());
    m_freq->setValue(freq);
}

void
UnitConverter::noteChanged()
{
    int pitch = Pitch::getPitchForNoteAndOctave(m_note->currentIndex(),
						m_octave->value());
    double freq = Pitch::getFrequencyForPitch(pitch, m_cents->value());
    m_freq->setValue(freq);
}

void
UnitConverter::octaveChanged()
{
    int pitch = Pitch::getPitchForNoteAndOctave(m_note->currentIndex(),
						m_octave->value());
    double freq = Pitch::getFrequencyForPitch(pitch, m_cents->value());
    m_freq->setValue(freq);
}

void
UnitConverter::centsChanged()
{
    double freq = Pitch::getFrequencyForPitch(m_midi->value(), m_cents->value());
    m_freq->setValue(freq);
}

void
UnitConverter::updatePitchesFromFreq()
{
    double cents = 0;
    int pitch = Pitch::getPitchForFrequency(m_freq->value(), &cents);
    int note, octave;
    Pitch::getNoteAndOctaveForPitch(pitch, note, octave);

    cerr << "pitch " << pitch << " note " << note << " octave " << octave << " cents " << cents << endl;

    setTo(m_midi, pitch);
    setTo(m_cents, cents);
    setTo(m_octave, octave);

    m_note->blockSignals(true);
    m_note->setCurrentIndex(note);
    m_note->blockSignals(false);
}

void
UnitConverter::samplesChanged()
{
    updateTempiFromSamples();
}

void
UnitConverter::periodChanged()
{
    double rate = getSampleRate();
    double sec = m_period->value() / 1000.0;
    double samples = rate * sec;
    m_samples->setValue(samples);
}

void
UnitConverter::bpmChanged()
{
    double rate = getSampleRate();
    double sec = 60.0 / m_bpm->value();
    double samples = rate * sec;
    m_samples->setValue(samples);
}

void
UnitConverter::tempofreqChanged()
{
    double rate = getSampleRate();
    double samples = rate / m_tempofreq->value();
    m_samples->setValue(samples);
}

void
UnitConverter::samplerateChanged()
{
    // Preserve the beat period in seconds, here, not in samples
    periodChanged();
}

double
UnitConverter::getSampleRate()
{
    return double(atoi(m_samplerate->currentText().toLocal8Bit().data()));
}

void
UnitConverter::updateTempiFromSamples()
{
    double samples = m_samples->value();
    double rate = getSampleRate();

    cerr << samples << " samples at rate " << rate << endl;

    double sec = samples / rate;
    double hz = rate / samples;
    double bpm = 60.0 / sec;

    setTo(m_bpm, bpm);
    setTo(m_period, sec * 1000.0);
    setTo(m_tempofreq, hz);
}

 
