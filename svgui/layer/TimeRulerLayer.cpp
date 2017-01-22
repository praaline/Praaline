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

#include "TimeRulerLayer.h"

#include "LayerFactory.h"

#include "data/model/Model.h"
#include "base/RealTime.h"
#include "ColourDatabase.h"
#include "view/View.h"

#include <QPainter>
#include <QString>
#include <QStringList>

#include <iostream>
#include <cmath>

//#define DEBUG_TIME_RULER_LAYER 1

TimeRulerLayer::TimeRulerLayer() :
    SingleColourLayer(),
    m_model(0),
    m_labelHeight(LabelTop)
{
    
}

bool TimeRulerLayer::trySetModel(Model *model)
{
    this->setModel(model);
    return true;
}

void TimeRulerLayer::setModel(Model *model)
{
    if (m_model == model) return;
    m_model = model;
    emit modelReplaced();
}

bool
TimeRulerLayer::snapToFeatureFrame(View *v, sv_frame_t &frame, int &resolution, SnapType snap, int y) const
{
    if (!m_model) {
        resolution = 1;
        return false;
    }

    bool q;
    int tick = getMajorTickSpacing(v, q);
    RealTime rtick = RealTime::fromMilliseconds(tick);
    sv_samplerate_t rate = m_model->getSampleRate();
    
    RealTime rt = RealTime::frame2RealTime(frame, rate);
    double ratio = rt / rtick;

    int rounded = int(ratio);
    RealTime rdrt = rtick * rounded;

    sv_frame_t left = RealTime::realTime2Frame(rdrt, rate);
    resolution = int(RealTime::realTime2Frame(rtick, rate));
    sv_frame_t right = left + resolution;

    //    cerr << "TimeRulerLayer::snapToFeatureFrame: type "
    //              << int(snap) << ", frame " << frame << " (time "
    //              << rt << ", tick " << rtick << ", rounded " << rdrt << ") ";

    switch (snap) {

    case SnapLeft:
        frame = left;
        break;

    case SnapRight:
        frame = right;
        break;
        
    case SnapNearest:
    {
        if (labs(frame - left) > labs(right - frame)) {
            frame = right;
        } else {
            frame = left;
        }
        break;
    }

    case SnapNeighbouring:
    {
        int dl = -1, dr = -1;
        int x = v->getXForFrame(frame);

        if (left > v->getStartFrame() &&
                left < v->getEndFrame()) {
            dl = abs(v->getXForFrame(left) - x);
        }

        if (right > v->getStartFrame() &&
                right < v->getEndFrame()) {
            dr = abs(v->getXForFrame(right) - x);
        }

        int fuzz = 2;

        if (dl >= 0 && dr >= 0) {
            if (dl < dr) {
                if (dl <= fuzz) {
                    frame = left;
                }
            } else {
                if (dr < fuzz) {
                    frame = right;
                }
            }
        } else if (dl >= 0) {
            if (dl <= fuzz) {
                frame = left;
            }
        } else if (dr >= 0) {
            if (dr <= fuzz) {
                frame = right;
            }
        }
    }
    }

    //    cerr << " -> " << frame << " (resolution = " << resolution << ")" << endl;

    return true;
}

int
TimeRulerLayer::getMajorTickSpacing(View *v, bool &quarterTicks) const
{
    // return value is in milliseconds

    if (!m_model || !v) return 1000;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return 1000;

    sv_frame_t startFrame = v->getStartFrame();
    sv_frame_t endFrame = v->getEndFrame();

    int minPixelSpacing = 50;

    RealTime rtStart = RealTime::frame2RealTime(startFrame, sampleRate);
    RealTime rtEnd = RealTime::frame2RealTime(endFrame, sampleRate);

    int count = v->width() / minPixelSpacing;
    if (count < 1) count = 1;
    RealTime rtGap = (rtEnd - rtStart) / count;

    int incms;
    quarterTicks = false;

    if (rtGap.sec > 0) {
        incms = 1000;
        int s = rtGap.sec;
        if (s > 0) { incms *= 5; s /= 5; }
        if (s > 0) { incms *= 2; s /= 2; }
        if (s > 0) { incms *= 6; s /= 6; quarterTicks = true; }
        if (s > 0) { incms *= 5; s /= 5; quarterTicks = false; }
        if (s > 0) { incms *= 2; s /= 2; }
        if (s > 0) { incms *= 6; s /= 6; quarterTicks = true; }
        while (s > 0) {
            incms *= 10;
            s /= 10;
            quarterTicks = false;
        }
    } else {
        incms = 1;
        int ms = rtGap.msec();
        if (ms > 0) { incms *= 10; ms /= 10; }
        if (ms > 0) { incms *= 10; ms /= 10; }
        if (ms > 0) { incms *= 5; ms /= 5; }
        if (ms > 0) { incms *= 2; ms /= 2; }
    }

    return incms;
}

void
TimeRulerLayer::paint(View *v, QPainter &paint, QRect rect) const
{
#ifdef DEBUG_TIME_RULER_LAYER
    cerr << "TimeRulerLayer::paint (" << rect.x() << "," << rect.y()
         << ") [" << rect.width() << "x" << rect.height() << "]" << endl;
#endif
    
    if (!m_model || !m_model->isOK()) return;

    sv_samplerate_t sampleRate = m_model->getSampleRate();
    if (!sampleRate) return;

    sv_frame_t startFrame = v->getFrameForX(rect.x() - 50);

#ifdef DEBUG_TIME_RULER_LAYER
    cerr << "start frame = " << startFrame << endl;
#endif

    bool quarter = false;
    int incms = getMajorTickSpacing(v, quarter);

    int ms = int(lrint(1000.0 * (double(startFrame) / double(sampleRate))));
    ms = (ms / incms) * incms - incms;

#ifdef DEBUG_TIME_RULER_LAYER
    cerr << "start ms = " << ms << " at step " << incms << endl;
#endif

    // Calculate the number of ticks per increment -- approximate
    // values for x and frame counts here will do, no rounding issue.
    // We always use the exact incms in our calculations for where to
    // draw the actual ticks or lines.

    int minPixelSpacing = 50;
    sv_frame_t incFrame = lrint((incms * sampleRate) / 1000);
    int incX = int(incFrame / v->getZoomLevel());
    int ticks = 10;
    if (incX < minPixelSpacing * 2) {
        ticks = quarter ? 4 : 5;
    }

    QColor greyColour = getPartialShades(v)[1];

    paint.save();

    // Do not label time zero - we now overlay an opaque area over
    // time < 0 which would cut it in half
    int minlabel = 1; // ms

    while (1) {

        // frame is used to determine where to draw the lines, so it
        // needs to correspond to an exact pixel (so that we don't get
        // a different pixel when scrolling a small amount and
        // re-drawing with a different start frame).

        double dms = ms;
        sv_frame_t frame = lrint((dms * sampleRate) / 1000.0);
        frame /= v->getZoomLevel();
        frame *= v->getZoomLevel(); // so frame corresponds to an exact pixel

        int x = v->getXForFrame(frame);

#ifdef DEBUG_TIME_RULER_LAYER
        cerr << "Considering frame = " << frame << ", x = " << x << endl;
#endif

        if (x >= rect.x() + rect.width() + 50) {
#ifdef DEBUG_TIME_RULER_LAYER
            cerr << "X well out of range, ending here" << endl;
#endif
            break;
        }

        if (x >= rect.x() - 50 && ms >= minlabel) {

            RealTime rt = RealTime::fromMilliseconds(ms);

#ifdef DEBUG_TIME_RULER_LAYER
            cerr << "X in range, drawing line here for time " << rt.toText() << endl;
#endif

            QString text(QString::fromStdString(rt.toText()));
            QFontMetrics metrics = paint.fontMetrics();
            int tw = metrics.width(text);

            if (tw < 50 &&
                    (x < rect.x() - tw/2 ||
                     x >= rect.x() + rect.width() + tw/2)) {
#ifdef DEBUG_TIME_RULER_LAYER
                cerr << "hm, maybe X isn't in range after all (x = " << x << ", tw = " << tw << ", rect.x() = " << rect.x() << ", rect.width() = " << rect.width() << ")" << endl;
#endif
            }

            paint.setPen(greyColour);
            paint.drawLine(x, 0, x, v->height());

            paint.setPen(getBaseQColor());
            paint.drawLine(x, 0, x, 5);
            paint.drawLine(x, v->height() - 6, x, v->height() - 1);

            int y;
            switch (m_labelHeight) {
            default:
            case LabelTop:
                y = 6 + metrics.ascent();
                break;
            case LabelMiddle:
                y = v->height() / 2 - metrics.height() / 2 + metrics.ascent();
                break;
            case LabelBottom:
                y = v->height() - metrics.height() + metrics.ascent() - 6;
            }

            if (v->getViewManager() && v->getViewManager()->getOverlayMode() !=
                    ViewManager::NoOverlays) {

                if (v->getLayer(0) == this) {
                    // backmost layer, don't worry about outlining the text
                    paint.drawText(x+2 - tw/2, y, text);
                } else {
                    v->drawVisibleText(paint, x+2 - tw/2, y, text, View::OutlinedText);
                }
            }
        }

        paint.setPen(greyColour);

        for (int i = 1; i < ticks; ++i) {

            dms = ms + (i * double(incms)) / ticks;
            frame = lrint((dms * sampleRate) / 1000.0);
            frame /= v->getZoomLevel();
            frame *= v->getZoomLevel(); // exact pixel as above

            x = v->getXForFrame(frame);

            if (x < rect.x() || x >= rect.x() + rect.width()) {
#ifdef DEBUG_TIME_RULER_LAYER
                //                cerr << "tick " << i << ": X out of range, going on to next tick" << endl;
#endif
                continue;
            }

#ifdef DEBUG_TIME_RULER_LAYER
            cerr << "tick " << i << " in range, drawing at " << x << endl;
#endif

            int sz = 5;
            if (ticks == 10) {
                if ((i % 2) == 1) {
                    if (i == 5) {
                        paint.drawLine(x, 0, x, v->height());
                    } else sz = 3;
                } else {
                    sz = 7;
                }
            }
            paint.drawLine(x, 0, x, sz);
            paint.drawLine(x, v->height() - sz - 1, x, v->height() - 1);
        }

        ms += incms;
    }

    paint.restore();
}

int
TimeRulerLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = true;
    return ColourDatabase::getInstance()->getColourIndex
            (QString(darkbg ? "White" : "Black"));
}

QString TimeRulerLayer::getLayerPresentationName() const
{
    LayerFactory &factory = LayerFactory::getInstance();
    QString layerName = factory.getLayerPresentationName(factory.getLayerType(this));
    return layerName;
}

void
TimeRulerLayer::toXml(QTextStream &stream,
                      QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent, extraAttributes);
}

void
TimeRulerLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);
}

