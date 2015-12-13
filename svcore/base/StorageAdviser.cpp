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

#include "StorageAdviser.h"

#include "Exceptions.h"
#include "TempDirectory.h"

#include "system/System.h"

#include <iostream>

//#define DEBUG_STORAGE_ADVISER 1

size_t StorageAdviser::m_discPlanned = 0;
size_t StorageAdviser::m_memoryPlanned = 0;

StorageAdviser::Recommendation
StorageAdviser::m_baseRecommendation = StorageAdviser::NoRecommendation;

StorageAdviser::Recommendation
StorageAdviser::recommend(Criteria criteria,
			  size_t minimumSize,
			  size_t maximumSize)
{
#ifdef DEBUG_STORAGE_ADVISER
    cerr << "StorageAdviser::recommend: Criteria " << criteria 
              << ", minimumSize " << minimumSize
              << ", maximumSize " << maximumSize << endl;
#endif

    if (m_baseRecommendation != NoRecommendation) {
        return m_baseRecommendation; // for now
    }

    QString path;
    try {
        path = TempDirectory::getInstance()->getPath();
    } catch (std::exception e) {
        cerr << "StorageAdviser::recommend: ERROR: Failed to get temporary directory path: " << e.what() << endl;
        return Recommendation(UseMemory | ConserveSpace);
    }
    ssize_t discFree = GetDiscSpaceMBAvailable(path.toLocal8Bit());
    ssize_t memoryFree, memoryTotal;
    GetRealMemoryMBAvailable(memoryFree, memoryTotal);

    if (discFree > ssize_t(m_discPlanned / 1024 + 1)) {
        discFree -= m_discPlanned / 1024 + 1;
    } else if (discFree > 0) { // can also be -1 for unknown
        discFree = 0;
    }

    if (memoryFree > ssize_t(m_memoryPlanned / 1024 + 1)) {
        memoryFree -= m_memoryPlanned / 1024 + 1;
    } else if (memoryFree > 0) { // can also be -1 for unknown
        memoryFree = 0;
    }

#ifdef DEBUG_STORAGE_ADVISER
    cerr << "Disc space: " << discFree << ", memory free: " << memoryFree << ", memory total: " << memoryTotal << ", min " << minimumSize << ", max " << maximumSize << endl;
#endif

    //!!! We have a potentially serious problem here if multiple
    //recommendations are made in advance of any of the resulting
    //allocations, as the allocations that have been recommended for
    //won't be taken into account in subsequent recommendations.

    enum StorageStatus {
        Unknown,
        Insufficient,
        Marginal,
        Sufficient
    };

    StorageStatus memoryStatus = Unknown;
    StorageStatus discStatus = Unknown;

    ssize_t minmb = ssize_t(minimumSize / 1024 + 1);
    ssize_t maxmb = ssize_t(maximumSize / 1024 + 1);

    if (memoryFree == -1) memoryStatus = Unknown;
    else if (memoryFree < memoryTotal / 3) memoryStatus = Insufficient;
    else if (minmb > (memoryFree * 3) / 4) memoryStatus = Insufficient;
    else if (maxmb > (memoryFree * 3) / 4) memoryStatus = Marginal;
    else if (minmb > (memoryFree / 3)) memoryStatus = Marginal;
    else if (memoryTotal == -1 ||
             minmb > (memoryTotal / 10)) memoryStatus = Marginal;
    else memoryStatus = Sufficient;

    if (discFree == -1) discStatus = Unknown;
    else if (minmb > (discFree * 3) / 4) discStatus = Insufficient;
    else if (maxmb > (discFree / 4)) discStatus = Marginal;
    else if (minmb > (discFree / 10)) discStatus = Marginal;
    else discStatus = Sufficient;

#ifdef DEBUG_STORAGE_ADVISER
    cerr << "Memory status: " << memoryStatus << ", disc status "
              << discStatus << endl;
#endif

    int recommendation = NoRecommendation;

    if (memoryStatus == Insufficient || memoryStatus == Unknown) {

        recommendation |= UseDisc;

        if (discStatus == Insufficient && minmb > discFree) {
            throw InsufficientDiscSpace(path, minmb, discFree);
        }

        if (discStatus == Insufficient || discStatus == Marginal) {
            recommendation |= ConserveSpace;
        } else if (discStatus == Unknown && !(criteria & PrecisionCritical)) {
            recommendation |= ConserveSpace;
        } else {
            recommendation |= UseAsMuchAsYouLike;
        }

    } else if (memoryStatus == Marginal) {

        if (((criteria & SpeedCritical) ||
             (criteria & FrequentLookupLikely)) &&
            !(criteria & PrecisionCritical) &&
            !(criteria & LongRetentionLikely)) {

            // requirements suggest a preference for memory

            if (discStatus != Insufficient) {
                recommendation |= PreferMemory;
            } else {
                recommendation |= UseMemory;
            }

            recommendation |= ConserveSpace;

        } else {

            if (discStatus == Insufficient) {
                recommendation |= (UseMemory | ConserveSpace);
            } else if (discStatus == Marginal) {
                recommendation |= (PreferMemory | ConserveSpace);
            } else if (discStatus == Unknown) {
                recommendation |= (PreferDisc | ConserveSpace);
            } else {
                recommendation |= (UseDisc | UseAsMuchAsYouLike);
            }
        }    

    } else {

        if (discStatus == Insufficient) {
            recommendation |= (UseMemory | ConserveSpace);
        } else if (discStatus != Sufficient) {
            recommendation |= (PreferMemory | ConserveSpace);
        } else {

            if ((criteria & SpeedCritical) ||
                (criteria & FrequentLookupLikely)) {
                recommendation |= PreferMemory;
                if (criteria & PrecisionCritical) {
                    recommendation |= UseAsMuchAsYouLike;
                } else {
                    recommendation |= ConserveSpace;
                }
            } else {
                recommendation |= PreferDisc;
                recommendation |= UseAsMuchAsYouLike;
            }
        }
    }

    return Recommendation(recommendation);
}

void
StorageAdviser::notifyPlannedAllocation(AllocationArea area, size_t size)
{
    if (area == MemoryAllocation) m_memoryPlanned += size;
    else if (area == DiscAllocation) m_discPlanned += size;
//    cerr << "storage planned up: memory: " << m_memoryPlanned << ", disc "
//              << m_discPlanned << endl;
}

void
StorageAdviser::notifyDoneAllocation(AllocationArea area, size_t size)
{
    if (area == MemoryAllocation) {
        if (m_memoryPlanned > size) m_memoryPlanned -= size;
        else m_memoryPlanned = 0;
    } else if (area == DiscAllocation) {
        if (m_discPlanned > size) m_discPlanned -= size; 
        else m_discPlanned = 0;
    }
//    cerr << "storage planned down: memory: " << m_memoryPlanned << ", disc "
//              << m_discPlanned << endl;
}

void
StorageAdviser::setFixedRecommendation(Recommendation recommendation)
{
    m_baseRecommendation = recommendation;
}

