/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PaintAssistant.h"

#include "base/AudioLevel.h"

#include <QPaintDevice>
#include <QPainter>

#include <iostream>
#include <cmath>

void
PaintAssistant::paintVerticalLevelScale(QPainter &paint, QRect rect,
					double minVal, double maxVal,
                                        Scale scale, int &mult,
                                        std::vector<int> *vy)
{
    static double meterdbs[] = { -40, -30, -20, -15, -10,
                                -5, -3, -2, -1, -0.5, 0 };

    int h = rect.height(), w = rect.width();
    int textHeight = paint.fontMetrics().height();
    int toff = -textHeight/2 + paint.fontMetrics().ascent() + 1;

    int lastLabelledY = -1;

    int n = 10;
    if (vy) vy->clear();

    double step = 0;
    mult = 1;
    if (scale == LinearScale) {
        step = (maxVal - minVal) / n;
        int round = 0, limit = 10000000;
        do {
            round = int(minVal + step * mult);
            mult *= 10;
        } while (!round && mult < limit);
        if (round) {
            mult /= 10;
//            cerr << "\n\nstep goes from " << step;
            step = double(round) / mult;
            n = int(lrint((maxVal - minVal) / step));
            if (mult > 1) {
                mult /= 10;
            }
//            cerr << " to " << step << " (n = " << n << ")" << endl;
        }
    }

    for (int i = 0; i <= n; ++i) {
        
        double val = 0.0, nval = 0.0;
        QString text = "";

        switch (scale) {
                
        case LinearScale:
            val = (minVal + (i * step));
            text = QString("%1").arg(mult * val);
            break;
            
        case MeterScale: // ... min, max
            val = AudioLevel::dB_to_multiplier(meterdbs[i]);
            text = QString("%1").arg(meterdbs[i]);
            if (i == n) text = "0dB";
            if (i == 0) {
                text = "-Inf";
                val = 0.0;
            }
            break;

        case dBScale: // ... min, max
            val = AudioLevel::dB_to_multiplier(-(10*n) + i * 10);
            text = QString("%1").arg(-(10*n) + i * 10);
            if (i == n) text = "0dB";
            if (i == 0) {
                text = "-Inf";
                val = 0.0;
            }
            break;
        }

        if (val < minVal || val > maxVal) continue;

        int y = getYForValue(scale, val, minVal, maxVal, rect.y(), h);
            
        int ny = y;
        if (nval != 0.0) {
            ny = getYForValue(scale, nval, minVal, maxVal, rect.y(), h);
        }

//        cerr << "PaintAssistant::paintVerticalLevelScale: val = "
//                  << val << ", y = " << y << ", h = " << h << endl;

        bool spaceForLabel = (i == 0 ||
                              abs(y - lastLabelledY) >= textHeight - 1);
        
        if (spaceForLabel) {
            
            int tx = 3;
//            if (scale != LinearScale) {
            if (paint.fontMetrics().width(text) < w - 10) {
                tx = w - 10 - paint.fontMetrics().width(text);
            }
            
            int ty = y;

            if (ty < paint.fontMetrics().ascent()) {
                ty = paint.fontMetrics().ascent();
//            } else if (ty > rect.y() + h - paint.fontMetrics().descent()) {
//                ty = rect.y() + h - paint.fontMetrics().descent();
            } else {
                ty += toff;
            }

            paint.drawText(tx, ty, text);
            
            lastLabelledY = ty - toff;
            /*
            if (ny != y) {
                ty = ny;
                if (ty < paint.fontMetrics().ascent()) {
                    ty = paint.fontMetrics().ascent();
                } else if (ty > h - paint.fontMetrics().descent()) {
                    ty = h - paint.fontMetrics().descent();
                } else {
                    ty += toff;
                }
                paint.drawText(tx, ty, text);
            }
            */
            paint.drawLine(w - 7, y, w, y);
            if (vy) vy->push_back(y);

            if (ny != y) {
                paint.drawLine(w - 7, ny, w, ny);
                if (vy) vy->push_back(ny);
            }
            
        } else {
            
            paint.drawLine(w - 4, y, w, y);
            if (vy) vy->push_back(y);

            if (ny != y) {
                paint.drawLine(w - 4, ny, w, ny);
                if (vy) vy->push_back(ny);
            }
        }
    }
}

static int
dBscale(double sample, int m, double maxVal, double minVal) 
{
    if (sample < 0.0) return dBscale(-sample, m, maxVal, minVal);
    double dB = AudioLevel::multiplier_to_dB(sample);
    double mindB = AudioLevel::multiplier_to_dB(minVal);
    double maxdB = AudioLevel::multiplier_to_dB(maxVal);
    if (dB < mindB) return 0;
    if (dB > 0.0) return m;
    return int(((dB - mindB) * m) / (maxdB - mindB) + 0.1);
}

int
PaintAssistant::getYForValue(Scale scale, double value, 
                             double minVal, double maxVal,
                             int minY, int height)
{
    int vy = 0;

//    int m = height/2;
//    int my = minY + m;

    switch (scale) {

    case LinearScale:
//        vy = my - int(m * value);
        vy = minY + height - int(((value - minVal) / (maxVal - minVal)) * height);
        break;

    case MeterScale:
//        vy = my - AudioLevel::multiplier_to_preview(value, m);
        vy = minY + height - AudioLevel::multiplier_to_preview
            ((value - minVal) / (maxVal - minVal), height);
        break;

    case dBScale:
        vy = minY + height - dBscale(value, height, maxVal, minVal);
        break;
    }

    return vy;
}
