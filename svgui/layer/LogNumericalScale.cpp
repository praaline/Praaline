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

#include "LogNumericalScale.h"
#include "VerticalScaleLayer.h"

#include "base/LogRange.h"

#include <QPainter>

#include <cmath>

#include "view/View.h"

//#define DEBUG_TIME_VALUE_LAYER 1

int
LogNumericalScale::getWidth(View *,
			    QPainter &paint)
{
    return paint.fontMetrics().width("-000.00") + 10;
}

void
LogNumericalScale::paintVertical(View *v,
				 const VerticalScaleLayer *layer,
				 QPainter &paint,
				 int x0,
				 double minlog,
				 double maxlog)
{
    int w = getWidth(v, paint) + x0;

    int n = 10;

    double val = minlog;
    double inc = (maxlog - val) / n; // even increments of log scale

    // smallest increment as displayed
    double minDispInc = LogRange::unmap(minlog + inc) - LogRange::unmap(minlog);

#ifdef DEBUG_TIME_VALUE_LAYER
    cerr << "min = " << minlog << ", max = " << maxlog << ", inc = " << inc << ", minDispInc = " << minDispInc << endl;
#endif

    const int buflen = 40;
    char buffer[buflen];

    double round = 1.f;
    int dp = 0;

    if (minDispInc > 0) {
        int prec = int(trunc(log10(minDispInc)));
        if (prec < 0) dp = -prec;
        round = pow(10.0, prec);
        if (dp > 4) dp = 4;
#ifdef DEBUG_TIME_VALUE_LAYER
        cerr << "round = " << round << ", prec = " << prec << ", dp = " << dp << endl;
#endif
    }

    int prevy = -1;
                
    for (int i = 0; i < n; ++i) {

	int y, ty;
        bool drawText = true;

	if (i == n-1 &&
	    v->height() < paint.fontMetrics().height() * (n*2)) {
	    if (layer->getScaleUnits() != "") drawText = false;
	}

        double dispval = LogRange::unmap(val);
	dispval = floor(dispval / round) * round;

#ifdef DEBUG_TIME_VALUE_LAYER
	cerr << "val = " << val << ", dispval = " << dispval << endl;
#endif

	y = layer->getYForValue(v, dispval);

	ty = y - paint.fontMetrics().height() + paint.fontMetrics().ascent() + 2;
	
	if (prevy >= 0 && (prevy - y) < paint.fontMetrics().height()) {
	    val += inc;
	    continue;
        }

	int digits = int(trunc(log10(dispval)));
	int sf = dp + (digits > 0 ? digits : 0);
	if (sf < 4) sf = 4;
#ifdef DEBUG_TIME_VALUE_LAYER
        cerr << "sf = " << sf << endl;
#endif
	snprintf(buffer, buflen, "%.*g", sf, dispval);

	QString label = QString(buffer);

	paint.drawLine(w - 5, y, w, y);

        if (drawText) {
	    paint.drawText(w - paint.fontMetrics().width(label) - 6,
			   ty, label);
        }

        prevy = y;
	val += inc;
    }
}
