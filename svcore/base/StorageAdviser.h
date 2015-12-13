
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _STORAGE_ADVISER_H_
#define _STORAGE_ADVISER_H_

#include <cstdlib>

/**
 * A utility class designed to help decide whether to store cache data
 * (for example FFT outputs) in memory or on disk in the TempDirectory.
 * This is basically a compendium of simple rules of thumb.
 */

class StorageAdviser
{
public:
    // pass to recommend() zero or more of these OR'd together
    enum Criteria {
        NoCriteria           = 0,
        SpeedCritical        = 1,
        PrecisionCritical    = 2,
        LongRetentionLikely  = 4,
        FrequentLookupLikely = 8
    };

    // recommend() returns one or two of these OR'd together
    enum Recommendation {
        NoRecommendation   = 0,
        UseMemory          = 1, // Disc is strongly contraindicated
        PreferMemory       = 2, // Either would do; memory probably better
        PreferDisc         = 4, // Either would do; disc probably better
        UseDisc            = 8, // Probably won't fit in memory
        ConserveSpace      = 16,// Whatever you choose, keep it compact
        UseAsMuchAsYouLike = 32 // Take my advice and there'll be space for all
    };

    /**
     * Recommend where to store some data, given certain storage and
     * recall criteria.  The minimum size is the approximate amount of
     * data in kilobytes that will be stored if the recommendation is
     * to ConserveSpace; the maximum size is approximately the amount
     * that will be used if UseAsMuchAsYouLike is returned.
     *
     * May throw InsufficientDiscSpace exception if there appears to
     * be nowhere the minimum amount of data can be stored.
     */
    static Recommendation recommend(Criteria criteria,
                                    size_t minimumSize,
                                    size_t maximumSize);

    enum AllocationArea {
        MemoryAllocation,
        DiscAllocation
    };

    /**
     * Specify that we are planning to use a given amount of storage
     * (in kilobytes), but haven't allocated it yet.
     */
    static void notifyPlannedAllocation(AllocationArea area, size_t size);

    /**
     * Specify that we have now allocated, or abandoned the allocation
     * of, the given amount (in kilobytes) of a storage area that was
     * previously notified using notifyPlannedAllocation.
     */
    static void notifyDoneAllocation(AllocationArea area, size_t size);

    /**
     * Force all subsequent recommendations to use the (perhaps
     * partial) specification given here.  If NoRecommendation given
     * here, this will reset to the default free behaviour.
     */
    static void setFixedRecommendation(Recommendation recommendation);

private:
    static size_t m_discPlanned;
    static size_t m_memoryPlanned;
    static Recommendation m_baseRecommendation;
};

#endif

