/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2009 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Dense3DModelPeakCache.h"

#include "base/Profiler.h"

Dense3DModelPeakCache::Dense3DModelPeakCache(DenseThreeDimensionalModel *source,
					     int columnsPerPeak) :
    m_source(source),
    m_resolution(columnsPerPeak)
{
    m_coverage.resize(1); // otherwise it is simply invalid

    m_cache = new EditableDenseThreeDimensionalModel
        (source->getSampleRate(),
         getResolution(),
         source->getHeight(),
         EditableDenseThreeDimensionalModel::NoCompression,
         false);

    connect(source, &Model::modelChanged,
            this, &Dense3DModelPeakCache::sourceModelChanged);
    connect(source, SIGNAL(aboutToBeDeleted()),
            this, SLOT(sourceModelAboutToBeDeleted()));

}

Dense3DModelPeakCache::~Dense3DModelPeakCache()
{
    delete m_cache;
}

bool
Dense3DModelPeakCache::isColumnAvailable(int column) const
{
    if (!m_source) return false;
    if (haveColumn(column)) return true;
    for (int i = m_resolution; i > 0; ) {
        --i;
        if (!m_source->isColumnAvailable(column * m_resolution + i)) {
            return false;
        }
    }
    return true;
}

Dense3DModelPeakCache::Column
Dense3DModelPeakCache::getColumn(int column) const
{
    Profiler profiler("Dense3DModelPeakCache::getColumn");
    if (!m_source) return Column();
    if (!haveColumn(column)) fillColumn(column);
    return m_cache->getColumn(column);
}

float
Dense3DModelPeakCache::getValueAt(int column, int n) const
{
    if (!m_source) return 0.f;
    if (!haveColumn(column)) fillColumn(column);
    return m_cache->getValueAt(column, n);
}

void
Dense3DModelPeakCache::sourceModelChanged()
{
    if (!m_source) return;
    if (m_coverage.size() > 0) {
        // The last peak may have come from an incomplete read, which
        // may since have been filled, so reset it
        m_coverage.reset(m_coverage.size()-1);
    }
    m_coverage.resize(getWidth()); // retaining data
}

void
Dense3DModelPeakCache::sourceModelAboutToBeDeleted()
{
    m_source = 0;
}

bool
Dense3DModelPeakCache::haveColumn(int column) const
{
    return column < (int)m_coverage.size() && m_coverage.get(column);
}

void
Dense3DModelPeakCache::fillColumn(int column) const
{
    Profiler profiler("Dense3DModelPeakCache::fillColumn");

    if (column >= (int)m_coverage.size()) {
        // see note in sourceModelChanged
        if (m_coverage.size() > 0) m_coverage.reset(m_coverage.size()-1);
        m_coverage.resize(column + 1);
    }

    Column peak;
    for (int i = 0; i < int(m_resolution); ++i) {
        Column here = m_source->getColumn(column * m_resolution + i);
        if (i == 0) {
            peak = here;
        } else {
            for (int j = 0; j < (int)peak.size() && j < (int)here.size(); ++j) {
                if (here[j] > peak[j]) peak[j] = here[j];
            }
        }
    }

    m_cache->setColumn(column, peak);
    m_coverage.set(column);
}


