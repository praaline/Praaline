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

#include "CSVFormatDialog.h"

#include "layer/LayerFactory.h"

#include "TextAbbrev.h"

#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>

#include <iostream>
#include <cmath>

#include "base/Debug.h"

CSVFormatDialog::CSVFormatDialog(QWidget *parent, CSVFormat format,
                                 int maxDisplayCols) :
    QDialog(parent),
    m_format(format),
    m_maxDisplayCols(maxDisplayCols),
    m_fuzzyColumn(-1)
{
    setModal(true);
    setWindowTitle(tr("Select Data Format"));

    QGridLayout *layout = new QGridLayout;

    int row = 0;

    layout->addWidget(new QLabel(tr("Please select the correct data format for this file.")),
		      row++, 0, 1, 4);

    QFrame *exampleFrame = new QFrame;
    exampleFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    exampleFrame->setLineWidth(2);
    QGridLayout *exampleLayout = new QGridLayout;
    exampleLayout->setSpacing(4);
    exampleFrame->setLayout(exampleLayout);

    QPalette palette = exampleFrame->palette();
    palette.setColor(QPalette::Window, palette.color(QPalette::Base));
    exampleFrame->setPalette(palette);

    QFont fp;
    fp.setPointSize(int(floor(fp.pointSize() * 0.9)));
//    fp.setFixedPitch(true);
//    fp.setStyleHint(QFont::TypeWriter);
//    fp.setFamily("Monospaced");
    
    int columns = format.getColumnCount();
    QList<QStringList> example = m_format.getExample();

    for (int i = 0; i < columns; ++i) {

        QComboBox *cpc = new QComboBox;
        m_columnPurposeCombos.push_back(cpc);
        exampleLayout->addWidget(cpc, 0, i);
        connect(cpc, SIGNAL(activated(int)), this, SLOT(columnPurposeChanged(int)));

        if (i == m_maxDisplayCols && columns > i + 2) {
            m_fuzzyColumn = i;
            cpc->addItem(tr("<ignore>"));
            cpc->addItem(tr("Values"));
            cpc->setCurrentIndex
                (m_format.getColumnPurpose(i-1) == CSVFormat::ColumnUnknown ? 0 : 1);
            exampleLayout->addWidget(new QLabel(tr("(%1 more)").arg(columns - i)),
                                     1, i);
            break;
        }

        // NB must be in the same order as the CSVFormat::ColumnPurpose enum
        cpc->addItem(tr("<ignore>")); // ColumnUnknown
        cpc->addItem(tr("Time"));     // ColumnStartTime
        cpc->addItem(tr("End time")); // ColumnEndTime
        cpc->addItem(tr("Duration")); // ColumnDuration
        cpc->addItem(tr("Value"));    // ColumnValue
        cpc->addItem(tr("Pitch"));    // ColumnPitch
        cpc->addItem(tr("Label"));    // ColumnLabel
        cpc->setCurrentIndex(int(m_format.getColumnPurpose(i)));

        for (int j = 0; j < example.size() && j < 6; ++j) {
            if (i >= example[j].size()) {
                continue;
            }
            QLabel *label = new QLabel;
            label->setTextFormat(Qt::PlainText);
            QString text = TextAbbrev::abbreviate(example[j][i], 35);
            label->setText(text);
            label->setFont(fp);
            label->setPalette(palette);
            label->setIndent(8);
            exampleLayout->addWidget(label, j+1, i);
        }
    }

    layout->addWidget(exampleFrame, row, 0, 1, 4);
    layout->setColumnStretch(3, 10);
    layout->setRowStretch(row++, 10);

    layout->addWidget(new QLabel(tr("Timing is specified:")), row, 0);
    
    m_timingTypeCombo = new QComboBox;
    m_timingTypeCombo->addItem(tr("Explicitly, in seconds"));
    m_timingTypeCombo->addItem(tr("Explicitly, in milliseconds"));
    m_timingTypeCombo->addItem(tr("Explicitly, in audio sample frames"));
    m_timingTypeCombo->addItem(tr("Implicitly: rows are equally spaced in time"));
    layout->addWidget(m_timingTypeCombo, row++, 1, 1, 2);
    connect(m_timingTypeCombo, SIGNAL(activated(int)),
	    this, SLOT(timingTypeChanged(int)));
    m_timingTypeCombo->setCurrentIndex
        (m_format.getTimingType() == CSVFormat::ExplicitTiming ?
         m_format.getTimeUnits() == CSVFormat::TimeSeconds ? 0 : 2 : 3);

    m_sampleRateLabel = new QLabel(tr("Audio sample rate (Hz):"));
    layout->addWidget(m_sampleRateLabel, row, 0);
    
    int sampleRates[] = {
	8000, 11025, 12000, 22050, 24000, 32000,
	44100, 48000, 88200, 96000, 176400, 192000
    };

    m_sampleRateCombo = new QComboBox;
    for (int i = 0; i < int(sizeof(sampleRates) / sizeof(sampleRates[0])); ++i) {
	m_sampleRateCombo->addItem(QString("%1").arg(sampleRates[i]));
	if (sampleRates[i] == m_format.getSampleRate()) {
            m_sampleRateCombo->setCurrentIndex(i);
        }
    }
    m_sampleRateCombo->setEditable(true);

    layout->addWidget(m_sampleRateCombo, row++, 1);
    connect(m_sampleRateCombo, SIGNAL(activated(QString)),
	    this, SLOT(sampleRateChanged(QString)));
    connect(m_sampleRateCombo, SIGNAL(editTextChanged(QString)),
	    this, SLOT(sampleRateChanged(QString)));

    m_windowSizeLabel = new QLabel(tr("Frame increment between rows:"));
    layout->addWidget(m_windowSizeLabel, row, 0);

    m_windowSizeCombo = new QComboBox;
    for (int i = 0; i <= 16; ++i) {
	int value = 1 << i;
	m_windowSizeCombo->addItem(QString("%1").arg(value));
	if (value == int(m_format.getWindowSize())) {
            m_windowSizeCombo->setCurrentIndex(i);
        }
    }
    m_windowSizeCombo->setEditable(true);

    layout->addWidget(m_windowSizeCombo, row++, 1);
    connect(m_windowSizeCombo, SIGNAL(activated(QString)),
	    this, SLOT(windowSizeChanged(QString)));
    connect(m_windowSizeCombo, SIGNAL(editTextChanged(QString)),
	    this, SLOT(windowSizeChanged(QString)));

    m_modelLabel = new QLabel;
    QFont f(m_modelLabel->font());
    f.setItalic(true);
    m_modelLabel->setFont(f);
    layout->addWidget(m_modelLabel, row++, 0, 1, 4);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    layout->addWidget(bb, row++, 0, 1, 4);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);

    timingTypeChanged(m_timingTypeCombo->currentIndex());
    updateModelLabel();
}

CSVFormatDialog::~CSVFormatDialog()
{
}

CSVFormat
CSVFormatDialog::getFormat() const
{
    return m_format;
}

void
CSVFormatDialog::updateModelLabel()
{
    LayerFactory &f = LayerFactory::getInstance();

    QString s;
    switch (m_format.getModelType()) {
    case CSVFormat::OneDimensionalModel:
        s = f.getLayerPresentationName(LayerFactory::Type("TimeInstants"));
        break;
    case CSVFormat::TwoDimensionalModel:
        s = f.getLayerPresentationName(LayerFactory::Type("TimeValues"));
        break; 
    case CSVFormat::TwoDimensionalModelWithDuration:
        s = f.getLayerPresentationName(LayerFactory::Type("Regions"));
        break;
    case CSVFormat::TwoDimensionalModelWithDurationAndPitch:
        s = f.getLayerPresentationName(LayerFactory::Type("Notes"));
        break;
    case CSVFormat::ThreeDimensionalModel:
        s = f.getLayerPresentationName(LayerFactory::Type("Colour3DPlot"));
        break;
    }   

    m_modelLabel->setText("\n" + tr("Data will be displayed in a %1 layer.").arg(s));
}

void
CSVFormatDialog::timingTypeChanged(int type)
{
    switch (type) {

    case 0:
	m_format.setTimingType(CSVFormat::ExplicitTiming);
	m_format.setTimeUnits(CSVFormat::TimeSeconds);
	m_sampleRateCombo->setEnabled(false);
	m_sampleRateLabel->setEnabled(false);
	m_windowSizeCombo->setEnabled(false);
	m_windowSizeLabel->setEnabled(false);
	break;

    case 1:
	m_format.setTimingType(CSVFormat::ExplicitTiming);
	m_format.setTimeUnits(CSVFormat::TimeMilliseconds);
	m_sampleRateCombo->setEnabled(true);
	m_sampleRateLabel->setEnabled(true);
	m_windowSizeCombo->setEnabled(false);
	m_windowSizeLabel->setEnabled(false);
	break;

    case 2:
	m_format.setTimingType(CSVFormat::ExplicitTiming);
	m_format.setTimeUnits(CSVFormat::TimeAudioFrames);
	m_sampleRateCombo->setEnabled(true);
	m_sampleRateLabel->setEnabled(true);
	m_windowSizeCombo->setEnabled(false);
	m_windowSizeLabel->setEnabled(false);
	break;

    case 3:
	m_format.setTimingType(CSVFormat::ImplicitTiming);
	m_format.setTimeUnits(CSVFormat::TimeWindows);
	m_sampleRateCombo->setEnabled(true);
	m_sampleRateLabel->setEnabled(true);
	m_windowSizeCombo->setEnabled(true);
	m_windowSizeLabel->setEnabled(true);
	break;
    }
}

void
CSVFormatDialog::sampleRateChanged(QString rateString)
{
    bool ok = false;
    int sampleRate = rateString.toInt(&ok);
    if (ok) m_format.setSampleRate(sampleRate);
}

void
CSVFormatDialog::windowSizeChanged(QString sizeString)
{
    bool ok = false;
    int size = sizeString.toInt(&ok);
    if (ok) m_format.setWindowSize(size);
}

void
CSVFormatDialog::columnPurposeChanged(int p)
{
    QObject *o = sender();

    QComboBox *cb = qobject_cast<QComboBox *>(o);
    if (!cb) return;

    CSVFormat::ColumnPurpose purpose = (CSVFormat::ColumnPurpose)p;

    bool haveStartTime = false;
    bool haveDuration = false;
    bool havePitch = false;
    int valueCount = 0;

    for (int i = 0; i < m_columnPurposeCombos.size(); ++i) {

        CSVFormat::ColumnPurpose cp = m_format.getColumnPurpose(i);

        bool thisChanged = (cb == m_columnPurposeCombos[i]);
        
        if (thisChanged) {

            cerr << "i == " << i << ", fuzzy == " << m_fuzzyColumn
                      << ", p == " << p << endl;

            if (i == m_fuzzyColumn) {
                for (int j = i; j < m_format.getColumnCount(); ++j) {
                    if (p == 0) { // Ignore
                        m_format.setColumnPurpose(j, CSVFormat::ColumnUnknown);
                    } else { // Value
                        m_format.setColumnPurpose(j, CSVFormat::ColumnValue);
                        ++valueCount;
                    }
                }
                continue;
            }

            cp = purpose;

        } else {

            if (i == m_fuzzyColumn) continue;

            // We can only have one ColumnStartTime column, and only
            // one of either ColumnDuration or ColumnEndTime

            if (purpose == CSVFormat::ColumnStartTime) {
                if (cp == purpose) {
                    cp = CSVFormat::ColumnValue;
                }
            } else if (purpose == CSVFormat::ColumnDuration ||
                       purpose == CSVFormat::ColumnEndTime) {
                if (cp == CSVFormat::ColumnDuration ||
                    cp == CSVFormat::ColumnEndTime) {
                    cp = CSVFormat::ColumnValue;
                }
            }

            // And we can only have one label
            if (purpose == CSVFormat::ColumnLabel) {
                if (cp == purpose) {
                    cp = CSVFormat::ColumnUnknown;
                }
            }
        }

        if (cp == CSVFormat::ColumnStartTime) {
            haveStartTime = true;
        }
        if (cp == CSVFormat::ColumnEndTime ||
            cp == CSVFormat::ColumnDuration) {
            haveDuration = true;
        }
        if (cp == CSVFormat::ColumnPitch) {
            havePitch = true;
        }
        if (cp == CSVFormat::ColumnValue) {
            ++valueCount;
        }

        m_columnPurposeCombos[i]->setCurrentIndex(int(cp));
        m_format.setColumnPurpose(i, cp);
    }

    if (!haveStartTime) {
        m_timingTypeCombo->setCurrentIndex(2);
        timingTypeChanged(2);
    }

    if (haveStartTime && haveDuration) {
        if (havePitch) {
            m_format.setModelType(CSVFormat::TwoDimensionalModelWithDurationAndPitch);
        } else {
            m_format.setModelType(CSVFormat::TwoDimensionalModelWithDuration);
        }
    } else {
        if (valueCount > 1) {
            m_format.setModelType(CSVFormat::ThreeDimensionalModel);
        } else if (valueCount > 0) {
            m_format.setModelType(CSVFormat::TwoDimensionalModel);
        } else {
            m_format.setModelType(CSVFormat::OneDimensionalModel);
        }
    }

    updateModelLabel();
}


