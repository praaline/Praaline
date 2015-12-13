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

#ifndef _ZOOM_CONSTRAINT_H_
#define _ZOOM_CONSTRAINT_H_

#include <stdlib.h>

/**
 * ZoomConstraint is a simple interface that describes a limitation on
 * the available zoom sizes for a view, for example based on cache
 * strategy or a (processing) window-size limitation.
 *
 * The default ZoomConstraint imposes no actual constraint except for
 * a nominal maximum.
 */

class ZoomConstraint
{
public:
    virtual ~ZoomConstraint() { }

    enum RoundingDirection {
	RoundDown,
	RoundUp,
	RoundNearest
    };

    /**
     * Given the "ideal" block size (frames per pixel) for a given
     * zoom level, return the nearest viable block size for this
     * constraint.
     *
     * For example, if a block size of 1523 frames per pixel is
     * requested but the underlying model only supports value
     * summaries at powers-of-two block sizes, return 1024 or 2048
     * depending on the rounding direction supplied.
     */
    virtual int getNearestBlockSize(int requestedBlockSize,
				       RoundingDirection = RoundNearest)
	const
    {
	if (requestedBlockSize > getMaxZoomLevel()) return getMaxZoomLevel();
	else return requestedBlockSize;
    }

    /**
     * Return the maximum zoom level within range for this constraint.
     */
    virtual int getMaxZoomLevel() const { return 262144; }
};

#endif

