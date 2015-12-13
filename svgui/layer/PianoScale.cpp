/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2013 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PianoScale.h"

#include <QPainter>

#include <cmath>

#include "base/Pitch.h"

#include "view/View.h"

void
PianoScale::paintPianoVertical(View *v,
			       QPainter &paint,
			       QRect r,
			       double minf,
			       double maxf)
{
    int x0 = r.x(), y0 = r.y(), x1 = r.x() + r.width(), y1 = r.y() + r.height();

    paint.drawLine(x0, y0, x0, y1);

    int py = y1, ppy = y1;
    paint.setBrush(paint.pen().color());

    for (int i = 0; i < 128; ++i) {

	double f = Pitch::getFrequencyForPitch(i);
	int y = int(lrint(v->getYForFrequency(f, minf, maxf, true)));

	if (y < y0 - 2) break;
	if (y > y1 + 2) {
	    continue;
	}
	
	int n = (i % 12);
	
	if (n == 1) {
	    // C# -- fill the C from here
	    QColor col = Qt::gray;
	    if (i == 61) { // filling middle C
		col = Qt::blue;
		col = col.light(150);
	    }
	    if (ppy - y > 2) {
		paint.fillRect(x0 + 1,
			       y,
			       x1 - x0,
			       (py + ppy) / 2 - y,
			       col);
	    }
	}
	
	if (n == 1 || n == 3 || n == 6 || n == 8 || n == 10) {
	    // black notes
	    paint.drawLine(x0 + 1, y, x1, y);
	    int rh = ((py - y) / 4) * 2;
	    if (rh < 2) rh = 2;
	    paint.drawRect(x0 + 1, y - (py-y)/4, (x1 - x0) / 2, rh);
	} else if (n == 0 || n == 5) {
	    // C, F
	    if (py < y1) {
		paint.drawLine(x0 + 1, (y + py) / 2, x1, (y + py) / 2);
	    }
	}
	
	ppy = py;
	py = y;
    }
}

