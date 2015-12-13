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

#include "ImageRegionFinder.h"

#include <QImage>
#include <cmath>
#include <stack>
#include <iostream>

ImageRegionFinder::ImageRegionFinder()
{
}

ImageRegionFinder::~ImageRegionFinder()
{
}

QRect
ImageRegionFinder::findRegionExtents(QImage *image, QPoint origin) const
{
    int w = image->width(), h = image->height();

    QImage visited(w, h, QImage::Format_Mono);
    visited.fill(0);

    std::stack<QPoint> s;
    s.push(origin);

    int xmin = origin.x();
    int xmax = xmin;
    int ymin = origin.y();
    int ymax = ymin;

    QRgb opix = image->pixel(origin);

    while (!s.empty()) {

        QPoint p = s.top();
        s.pop();

        visited.setPixel(p, 1);

        int x = p.x(), y = p.y();

        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;

        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;

        std::stack<QPoint> neighbours;

        int similarNeighbourCount = 0;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {

                if ((dx != 0 && dy != 0) ||
                    (dx == 0 && dy == 0)) 
                    continue;

                if (x + dx < 0 || x + dx >= w ||
                    y + dy < 0 || y + dy >= h)
                    continue;

                if (visited.pixelIndex(x + dx, y + dy) != 0)
                    continue;

                if (!similar(opix, image->pixel(x + dx, y + dy))) 
                    continue;

                neighbours.push(QPoint(x + dx, y + dy));
                ++similarNeighbourCount;
            }
        }

        if (similarNeighbourCount >= 2) {
            while (!neighbours.empty()) {
                s.push(neighbours.top());
                neighbours.pop();
            }
        }
    }

    return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
}

bool
ImageRegionFinder::similar(QRgb a, QRgb b) const
{
    if (b == qRgb(0, 0, 0) || b == qRgb(255, 255, 255)) {
        // black and white are boundary cases, don't compare similar
        // to anything -- not even themselves
        return false;
    }

    float ar = float(qRed(a)) / 255.f;
    float ag = float(qGreen(a)) / 255.f;
    float ab = float(qBlue(a)) / 255.f;
    float amag = sqrtf(ar * ar + ag * ag + ab * ab);
    float thresh = amag / 2;

    float dr = float(qRed(a) - qRed(b)) / 255.f;
    float dg = float(qGreen(a) - qGreen(b)) / 255.f;
    float db = float(qBlue(a) - qBlue(b)) / 255.f;
    float dist = sqrtf(dr * dr + dg * dg + db * db);

//    cerr << "thresh=" << thresh << ", dist=" << dist << endl;

    return (dist < thresh);
}

