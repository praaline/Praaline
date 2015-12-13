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

#include "WindowShapePreview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QString>

#include "data/fft/FFTapi.h"

#include <iostream>

#ifndef __GNUC__
#include <alloca.h>
#endif

WindowShapePreview::WindowShapePreview(QWidget *parent) :
    QFrame(parent),
    m_windowType(HanningWindow)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    setLayout(layout);
    m_windowTimeExampleLabel = new QLabel;
    m_windowFreqExampleLabel = new QLabel;
    layout->addWidget(m_windowTimeExampleLabel);
    layout->addWidget(m_windowFreqExampleLabel);
}

WindowShapePreview::~WindowShapePreview()
{
}

void
WindowShapePreview::updateLabels()
{
    int step = 24;
    float peak = 48;
    int w = step * 4, h = 64;
    WindowType type = m_windowType;
    Window<float> windower = Window<float>(type, step * 2);

    QPixmap timeLabel(w, h + 1);
    timeLabel.fill(Qt::white);
    QPainter timePainter(&timeLabel);

    QPainterPath path;

    path.moveTo(0, float(h) - peak + 1);
    path.lineTo(w, float(h) - peak + 1);

    timePainter.setPen(Qt::gray);
    timePainter.setRenderHint(QPainter::Antialiasing, true);
    timePainter.drawPath(path);
    
    path = QPainterPath();

#ifdef __GNUC__
    float acc[w];
#else
    float *acc = (float *)alloca(w * sizeof(float));
#endif

    for (int i = 0; i < w; ++i) acc[i] = 0.f;
    for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < step * 2; ++i) {
            acc[j * step + i] += windower.getValue(i);
        }
    }
    for (int i = 0; i < w; ++i) {
        int y = h - int(peak * acc[i] + 0.001f) + 1;
        if (i == 0) path.moveTo(i, y);
        else path.lineTo(i, y);
    }

    timePainter.drawPath(path);
    timePainter.setRenderHint(QPainter::Antialiasing, false);

    path = QPainterPath();

    timePainter.setPen(Qt::black);
    
    for (int i = 0; i < step * 2; ++i) {
        int y = h - int(peak * windower.getValue(i) + 0.001) + 1;
        if (i == 0) path.moveTo(i + step, float(y));
        else path.lineTo(i + step, float(y));
    }

    if (type == RectangularWindow) {
        timePainter.drawPath(path);
        path = QPainterPath();
    }

    timePainter.setRenderHint(QPainter::Antialiasing, true);
    path.addRect(0, 0, w, h + 1);
    timePainter.drawPath(path);

    QFont font;
    font.setPixelSize(10);
    font.setItalic(true);
    timePainter.setFont(font);
    QString label = tr("V / time");
    timePainter.drawText(w - timePainter.fontMetrics().width(label) - 4,
                         timePainter.fontMetrics().ascent() + 1, label);

    m_windowTimeExampleLabel->setPixmap(timeLabel);
    
    int fw = 100;

    QPixmap freqLabel(fw, h + 1);
    freqLabel.fill(Qt::white);
    QPainter freqPainter(&freqLabel);
    path = QPainterPath();

    int fftsize = 512;

    float *input = (float *)fftf_malloc(fftsize * sizeof(float));
    fftf_complex *output =
        (fftf_complex *)fftf_malloc(fftsize * sizeof(fftf_complex));
    fftf_plan plan = fftf_plan_dft_r2c_1d(fftsize, input, output,
                                            FFTW_ESTIMATE);
    for (int i = 0; i < fftsize; ++i) input[i] = 0.f;
    for (int i = 0; i < step * 2; ++i) {
        input[fftsize/2 - step + i] = windower.getValue(i);
    }
    
    fftf_execute(plan);
    fftf_destroy_plan(plan);

    float maxdb = 0.f;
    float mindb = 0.f;
    bool first = true;
    for (int i = 0; i < fftsize/2; ++i) {
        float power = output[i][0] * output[i][0] + output[i][1] * output[i][1];
        float db = mindb;
        if (power > 0) {
            db = 20.f * log10f(power);
            if (first || db > maxdb) maxdb = db;
            if (first || db < mindb) mindb = db;
            first = false;
        }
    }

    if (mindb > -80.f) mindb = -80.f;

    // -- no, don't use the actual mindb -- it's easier to compare
    // plots with a fixed min value
    mindb = -170.f;

    float maxval = maxdb + -mindb;

//    float ly = h - ((-80.f + -mindb) / maxval) * peak + 1;

    path.moveTo(0, float(h) - peak + 1);
    path.lineTo(fw, float(h) - peak + 1);

    freqPainter.setPen(Qt::gray);
    freqPainter.setRenderHint(QPainter::Antialiasing, true);
    freqPainter.drawPath(path);
    
    path = QPainterPath();
    freqPainter.setPen(Qt::black);

//    cerr << "maxdb = " << maxdb << ", mindb = " << mindb << ", maxval = " <<maxval << endl;

    for (int i = 0; i < fftsize/2; ++i) {
        float power = output[i][0] * output[i][0] + output[i][1] * output[i][1];
        float db = 20.f * log10f(power);
        float val = db + -mindb;
        if (val < 0) val = 0;
        float norm = val / maxval;
        float x = (float(fw) / float(fftsize/2)) * float(i);
        float y = float(h) - norm * peak + 1;
        if (i == 0) path.moveTo(x, y);
        else path.lineTo(x, y);
    }

    freqPainter.setRenderHint(QPainter::Antialiasing, true);
    path.addRect(0, 0, fw, h + 1);
    freqPainter.drawPath(path);

    fftf_free(input);
    fftf_free(output);

    freqPainter.setFont(font);
    label = tr("dB / freq");
    freqPainter.drawText(fw - freqPainter.fontMetrics().width(label) - 4,
                         freqPainter.fontMetrics().ascent() + 1, label);

    m_windowFreqExampleLabel->setPixmap(freqLabel);
}

void
WindowShapePreview::setWindowType(WindowType type)
{
    if (m_windowType == type) return;
    m_windowType = type;
    updateLabels();
}

