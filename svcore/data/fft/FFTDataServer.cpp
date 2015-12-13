/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "FFTDataServer.h"

#include "FFTFileCacheReader.h"
#include "FFTFileCacheWriter.h"
#include "FFTMemoryCache.h"

#include "model/DenseTimeValueModel.h"

#include "system/System.h"

#include "base/StorageAdviser.h"
#include "base/Exceptions.h"
#include "base/Profiler.h"
#include "base/Thread.h" // for debug mutex locker

#include <QWriteLocker>

#include <stdexcept>

//#define DEBUG_FFT_SERVER 1
//#define DEBUG_FFT_SERVER_FILL 1

#ifdef DEBUG_FFT_SERVER_FILL
#ifndef DEBUG_FFT_SERVER
#define DEBUG_FFT_SERVER 1
#endif
#endif


FFTDataServer::ServerMap FFTDataServer::m_servers;
FFTDataServer::ServerQueue FFTDataServer::m_releasedServers;
QMutex FFTDataServer::m_serverMapMutex;

FFTDataServer *
FFTDataServer::getInstance(const DenseTimeValueModel *model,
                           int channel,
                           WindowType windowType,
                           int windowSize,
                           int windowIncrement,
                           int fftSize,
                           bool polar,
                           StorageAdviser::Criteria criteria,
                           sv_frame_t fillFromFrame)
{
    QString n = generateFileBasename(model,
                                     channel,
                                     windowType,
                                     windowSize,
                                     windowIncrement,
                                     fftSize,
                                     polar);

    FFTDataServer *server = 0;
    
    MutexLocker locker(&m_serverMapMutex, "FFTDataServer::getInstance::m_serverMapMutex");

    if ((server = findServer(n))) {
        return server;
    }

    QString npn = generateFileBasename(model,
                                       channel,
                                       windowType,
                                       windowSize,
                                       windowIncrement,
                                       fftSize,
                                       !polar);

    if ((server = findServer(npn))) {
        return server;
    }

    try {
        server = new FFTDataServer(n,
                                   model,
                                   channel,
                                   windowType,
                                   windowSize,
                                   windowIncrement,
                                   fftSize,
                                   polar,
                                   criteria,
                                   fillFromFrame);
    } catch (InsufficientDiscSpace) {
        delete server;
        server = 0;
    }

    if (server) {
        m_servers[n] = ServerCountPair(server, 1);
    }

    return server;
}

FFTDataServer *
FFTDataServer::getFuzzyInstance(const DenseTimeValueModel *model,
                                int channel,
                                WindowType windowType,
                                int windowSize,
                                int windowIncrement,
                                int fftSize,
                                bool polar,
                                StorageAdviser::Criteria criteria,
                                sv_frame_t fillFromFrame)
{
    // Fuzzy matching:
    // 
    // -- if we're asked for polar and have non-polar, use it (and
    // vice versa).  This one is vital, and we do it for non-fuzzy as
    // well (above).
    //
    // -- if we're asked for an instance with a given fft size and we
    // have one already with a multiple of that fft size but the same
    // window size and type (and model), we can draw the results from
    // it (e.g. the 1st, 2nd, 3rd etc bins of a 512-sample FFT are the
    // same as the the 1st, 5th, 9th etc of a 2048-sample FFT of the
    // same window plus zero padding).
    //
    // -- if we're asked for an instance with a given window type and
    // size and fft size and we have one already the same but with a
    // smaller increment, we can draw the results from it (provided
    // our increment is a multiple of its)
    //
    // The FFTModel knows how to interpret these things.  In
    // both cases we require that the larger one is a power-of-two
    // multiple of the smaller (e.g. even though in principle you can
    // draw the results at increment 256 from those at increment 768
    // or 1536, the model doesn't support this).

    {
        MutexLocker locker(&m_serverMapMutex, "FFTDataServer::getFuzzyInstance::m_serverMapMutex");

        ServerMap::iterator best = m_servers.end();
        int bestdist = -1;
    
        for (ServerMap::iterator i = m_servers.begin(); i != m_servers.end(); ++i) {

            FFTDataServer *server = i->second.first;

            if (server->getModel() == model &&
                (server->getChannel() == channel || model->getChannelCount() == 1) &&
                server->getWindowType() == windowType &&
                server->getWindowSize() == windowSize &&
                server->getWindowIncrement() <= windowIncrement &&
                server->getFFTSize() >= fftSize) {
                
                if ((windowIncrement % server->getWindowIncrement()) != 0) continue;
                int ratio = windowIncrement / server->getWindowIncrement();
                bool poweroftwo = true;
                while (ratio > 1) {
                    if (ratio & 0x1) {
                        poweroftwo = false;
                        break;
                    }
                    ratio >>= 1;
                }
                if (!poweroftwo) continue;

                if ((server->getFFTSize() % fftSize) != 0) continue;
                ratio = server->getFFTSize() / fftSize;
                while (ratio > 1) {
                    if (ratio & 0x1) {
                        poweroftwo = false;
                        break;
                    }
                    ratio >>= 1;
                }
                if (!poweroftwo) continue;
                
                int distance = 0;
                
                if (server->getPolar() != polar) distance += 1;
                
                distance += ((windowIncrement / server->getWindowIncrement()) - 1) * 15;
                distance += ((server->getFFTSize() / fftSize) - 1) * 10;
                
                if (server->getFillCompletion() < 50) distance += 100;

#ifdef DEBUG_FFT_SERVER
                std::cerr << "FFTDataServer::getFuzzyInstance: Distance for server " << server << " is " << distance << ", best is " << bestdist << std::endl;
#endif
                
                if (bestdist == -1 || distance < bestdist) {
                    bestdist = distance;
                    best = i;
                }
            }
        }

        if (bestdist >= 0) {
            FFTDataServer *server = best->second.first;
#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::getFuzzyInstance: We like server " << server << " (with distance " << bestdist << ")" << std::endl;
#endif
            claimInstance(server, false);
            return server;
        }
    }

    // Nothing found, make a new one

    return getInstance(model,
                       channel,
                       windowType,
                       windowSize,
                       windowIncrement,
                       fftSize,
                       polar,
                       criteria,
                       fillFromFrame);
}

FFTDataServer *
FFTDataServer::findServer(QString n)
{    
#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::findServer(\"" << n << "\")" << std::endl;
#endif

    if (m_servers.find(n) != m_servers.end()) {

        FFTDataServer *server = m_servers[n].first;

#ifdef DEBUG_FFT_SERVER
        std::cerr << "FFTDataServer::findServer(\"" << n << "\"): found " << server << std::endl;
#endif

        claimInstance(server, false);

        return server;
    }

#ifdef DEBUG_FFT_SERVER
        std::cerr << "FFTDataServer::findServer(\"" << n << "\"): not found" << std::endl;
#endif

    return 0;
}

void
FFTDataServer::claimInstance(FFTDataServer *server)
{
    claimInstance(server, true);
}

void
FFTDataServer::claimInstance(FFTDataServer *server, bool needLock)
{
    MutexLocker locker(needLock ? &m_serverMapMutex : 0,
                       "FFTDataServer::claimInstance::m_serverMapMutex");

#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::claimInstance(" << server << ")" << std::endl;
#endif

    for (ServerMap::iterator i = m_servers.begin(); i != m_servers.end(); ++i) {
        if (i->second.first == server) {

            for (ServerQueue::iterator j = m_releasedServers.begin();
                 j != m_releasedServers.end(); ++j) {

                if (*j == server) {
#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::claimInstance: found in released server list, removing from it" << std::endl;
#endif
                    m_releasedServers.erase(j);
                    break;
                }
            }

            ++i->second.second;

#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::claimInstance: new refcount is " << i->second.second << std::endl;
#endif

            return;
        }
    }
    
    cerr << "ERROR: FFTDataServer::claimInstance: instance "
              << server << " unknown!" << endl;
}

void
FFTDataServer::releaseInstance(FFTDataServer *server)
{
    releaseInstance(server, true);
}

void
FFTDataServer::releaseInstance(FFTDataServer *server, bool needLock)
{    
    MutexLocker locker(needLock ? &m_serverMapMutex : 0,
                       "FFTDataServer::releaseInstance::m_serverMapMutex");

#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::releaseInstance(" << server << ")" << std::endl;
#endif

    // -- if ref count > 0, decrement and return
    // -- if the instance hasn't been used at all, delete it immediately 
    // -- if fewer than N instances (N = e.g. 3) remain with zero refcounts,
    //    leave them hanging around
    // -- if N instances with zero refcounts remain, delete the one that
    //    was last released first
    // -- if we run out of disk space when allocating an instance, go back
    //    and delete the spare N instances before trying again
    // -- have an additional method to indicate that a model has been
    //    destroyed, so that we can delete all of its fft server instances

    for (ServerMap::iterator i = m_servers.begin(); i != m_servers.end(); ++i) {
        if (i->second.first == server) {
            if (i->second.second == 0) {
                cerr << "ERROR: FFTDataServer::releaseInstance("
                          << server << "): instance not allocated" << endl;
            } else if (--i->second.second == 0) {
/*!!!
                if (server->m_lastUsedCache == -1) { // never used
#ifdef DEBUG_FFT_SERVER
                    std::cerr << "FFTDataServer::releaseInstance: instance "
                              << server << " has never been used, erasing"
                              << std::endl;
#endif
                    delete server;
                    m_servers.erase(i);
                } else {
*/
#ifdef DEBUG_FFT_SERVER
                    std::cerr << "FFTDataServer::releaseInstance: instance "
                              << server << " no longer in use, marking for possible collection"
                              << std::endl;
#endif
                    bool found = false;
                    for (ServerQueue::iterator j = m_releasedServers.begin();
                         j != m_releasedServers.end(); ++j) {
                        if (*j == server) {
                            cerr << "ERROR: FFTDataServer::releaseInstance("
                                      << server << "): server is already in "
                                      << "released servers list" << endl;
                            found = true;
                        }
                    }
                    if (!found) m_releasedServers.push_back(server);
                    server->suspend();
                    purgeLimbo();
//!!!                }
            } else {
#ifdef DEBUG_FFT_SERVER
                    std::cerr << "FFTDataServer::releaseInstance: instance "
                              << server << " now has refcount " << i->second.second
                              << std::endl;
#endif
            }
            return;
        }
    }

    cerr << "ERROR: FFTDataServer::releaseInstance(" << server << "): "
              << "instance not found" << endl;
}

void
FFTDataServer::purgeLimbo(int maxSize)
{
#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::purgeLimbo(" << maxSize << "): "
              << m_releasedServers.size() << " candidates" << std::endl;
#endif

    while (int(m_releasedServers.size()) > maxSize) {

        FFTDataServer *server = *m_releasedServers.begin();

        bool found = false;

#ifdef DEBUG_FFT_SERVER
        std::cerr << "FFTDataServer::purgeLimbo: considering candidate "
                  << server << std::endl;
#endif

        for (ServerMap::iterator i = m_servers.begin(); i != m_servers.end(); ++i) {

            if (i->second.first == server) {
                found = true;
                if (i->second.second > 0) {
                    cerr << "ERROR: FFTDataServer::purgeLimbo: Server "
                              << server << " is in released queue, but still has non-zero refcount "
                              << i->second.second << endl;
                    // ... so don't delete it
                    break;
                }
#ifdef DEBUG_FFT_SERVER
                std::cerr << "FFTDataServer::purgeLimbo: looks OK, erasing it"
                          << std::endl;
#endif

                m_servers.erase(i);
                delete server;
                break;
            }
        }

        if (!found) {
            cerr << "ERROR: FFTDataServer::purgeLimbo: Server "
                      << server << " is in released queue, but not in server map!"
                      << endl;
            delete server;
        }

        m_releasedServers.pop_front();
    }

#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::purgeLimbo(" << maxSize << "): "
              << m_releasedServers.size() << " remain" << std::endl;
#endif

}

void
FFTDataServer::modelAboutToBeDeleted(Model *model)
{
    MutexLocker locker(&m_serverMapMutex,
                       "FFTDataServer::modelAboutToBeDeleted::m_serverMapMutex");

#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::modelAboutToBeDeleted(" << model << ")"
              << std::endl;
#endif

    for (ServerMap::iterator i = m_servers.begin(); i != m_servers.end(); ++i) {
        
        FFTDataServer *server = i->second.first;

        if (server->getModel() == model) {

#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::modelAboutToBeDeleted: server is "
                      << server << std::endl;
#endif

            if (i->second.second > 0) {
                cerr << "WARNING: FFTDataServer::modelAboutToBeDeleted: Model " << model << " (\"" << model->objectName() << "\") is about to be deleted, but is still being referred to by FFT server " << server << " with non-zero refcount " << i->second.second << endl;
                server->suspendWrites();
                return;
            }
            for (ServerQueue::iterator j = m_releasedServers.begin();
                 j != m_releasedServers.end(); ++j) {
                if (*j == server) {
#ifdef DEBUG_FFT_SERVER
                    std::cerr << "FFTDataServer::modelAboutToBeDeleted: erasing from released servers" << std::endl;
#endif
                    m_releasedServers.erase(j);
                    break;
                }
            }
#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::modelAboutToBeDeleted: erasing server" << std::endl;
#endif
            m_servers.erase(i);
            delete server;
            return;
        }
    }
}

FFTDataServer::FFTDataServer(QString fileBaseName,
                             const DenseTimeValueModel *model,
                             int channel,
			     WindowType windowType,
			     int windowSize,
			     int windowIncrement,
			     int fftSize,
                             bool polar,
                             StorageAdviser::Criteria criteria,
                             sv_frame_t fillFromFrame) :
    m_fileBaseName(fileBaseName),
    m_model(model),
    m_channel(channel),
    m_windower(windowType, windowSize),
    m_windowSize(windowSize),
    m_windowIncrement(windowIncrement),
    m_fftSize(fftSize),
    m_polar(polar),
    m_width(0),
    m_height(0),
    m_cacheWidth(0),
    m_cacheWidthPower(0),
    m_cacheWidthMask(0),
    m_criteria(criteria),
    m_fftInput(0),
    m_exiting(false),
    m_suspended(true), //!!! or false?
    m_fillThread(0)
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "])::FFTDataServer" << endl;
#endif

    //!!! end is not correct until model finished reading -- what to do???

    sv_frame_t start = m_model->getStartFrame();
    sv_frame_t end = m_model->getEndFrame();

    m_width = int((end - start) / m_windowIncrement) + 1;
    m_height = m_fftSize / 2 + 1; // DC == 0, Nyquist == fftsize/2

#ifdef DEBUG_FFT_SERVER 
    cerr << "FFTDataServer(" << this << "): dimensions are "
              << m_width << "x" << m_height << endl;
#endif

    int maxCacheSize = 20 * 1024 * 1024;
    int columnSize = int(m_height * sizeof(fftsample) * 2 + sizeof(fftsample));
    if (m_width * columnSize < maxCacheSize * 2) m_cacheWidth = m_width;
    else m_cacheWidth = maxCacheSize / columnSize;
    
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << "): cache width nominal "
              << m_cacheWidth << ", actual ";
#endif
    
    int bits = 0;
    while (m_cacheWidth > 1) { m_cacheWidth >>= 1; ++bits; }
    m_cacheWidthPower = bits + 1;
    m_cacheWidth = 2;
    while (bits) { m_cacheWidth <<= 1; --bits; }
    m_cacheWidthMask = m_cacheWidth - 1;

#ifdef DEBUG_FFT_SERVER
    cerr << m_cacheWidth << " (power " << m_cacheWidthPower << ", mask "
              << m_cacheWidthMask << ")" << endl;
#endif

    if (m_criteria == StorageAdviser::NoCriteria) {

        // assume "spectrogram" criteria for polar ffts, and "feature
        // extraction" criteria for rectangular ones.

        if (m_polar) {
            m_criteria = StorageAdviser::Criteria
                (StorageAdviser::SpeedCritical |
                 StorageAdviser::LongRetentionLikely);
        } else {
            m_criteria = StorageAdviser::Criteria
                (StorageAdviser::PrecisionCritical);
        }
    }

    for (int i = 0; i <= m_width / m_cacheWidth; ++i) {
        m_caches.push_back(0);
    }

    m_fftInput = (fftsample *)
        fftf_malloc(fftSize * sizeof(fftsample));

    m_fftOutput = (fftf_complex *)
        fftf_malloc((fftSize/2 + 1) * sizeof(fftf_complex));

    m_workbuffer = (float *)
        fftf_malloc((fftSize+2) * sizeof(float));

    m_fftPlan = fftf_plan_dft_r2c_1d(m_fftSize,
                                     m_fftInput,
                                     m_fftOutput,
                                     FFTW_MEASURE);

    if (!m_fftPlan) {
        cerr << "ERROR: fftf_plan_dft_r2c_1d(" << m_windowSize << ") failed!" << endl;
        throw(0);
    }

    m_fillThread = new FillThread(*this, fillFromFrame);
}

FFTDataServer::~FFTDataServer()
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "])::~FFTDataServer()" << endl;
#endif

    m_suspended = false;
    m_exiting = true;
    m_condition.wakeAll();
    if (m_fillThread) {
        m_fillThread->wait();
        delete m_fillThread;
    }

//    MutexLocker locker(&m_writeMutex,
//                       "FFTDataServer::~FFTDataServer::m_writeMutex");

    QMutexLocker mlocker(&m_fftBuffersLock);
    QWriteLocker wlocker(&m_cacheVectorLock);

    for (CacheVector::iterator i = m_caches.begin(); i != m_caches.end(); ++i) {
        if (*i) {
            delete *i;
        }
    }

    deleteProcessingData();
}

void
FFTDataServer::deleteProcessingData()
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): deleteProcessingData" << endl;
#endif
    if (m_fftInput) {
        fftf_destroy_plan(m_fftPlan);
        fftf_free(m_fftInput);
        fftf_free(m_fftOutput);
        fftf_free(m_workbuffer);
    }
    m_fftInput = 0;
}

void
FFTDataServer::suspend()
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): suspend" << endl;
#endif
    Profiler profiler("FFTDataServer::suspend", false);

    QMutexLocker locker(&m_fftBuffersLock);
    m_suspended = true;
}

void
FFTDataServer::suspendWrites()
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): suspendWrites" << endl;
#endif
    Profiler profiler("FFTDataServer::suspendWrites", false);

    m_suspended = true;
}

void
FFTDataServer::resume()
{
#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): resume" << endl;
#endif
    Profiler profiler("FFTDataServer::resume", false);

    m_suspended = false;
    if (m_fillThread) {
        if (m_fillThread->isFinished()) {
            delete m_fillThread;
            m_fillThread = 0;
            deleteProcessingData();
        } else if (!m_fillThread->isRunning()) {
            m_fillThread->start();
        } else {
            m_condition.wakeAll();
        }
    }
}

void
FFTDataServer::getStorageAdvice(int w, int h,
                                bool &memoryCache, bool &compactCache)
{
    if (w < 0 || h < 0) throw std::domain_error("width & height must be non-negative");
    size_t cells = size_t(w) * h;
    size_t minimumSize = (cells / 1024) * sizeof(uint16_t); // kb
    size_t maximumSize = (cells / 1024) * sizeof(float); // kb

    // We don't have a compact rectangular representation, and compact
    // of course is never precision-critical

    bool canCompact = true;
    if ((m_criteria & StorageAdviser::PrecisionCritical) || !m_polar) {
        canCompact = false;
        minimumSize = maximumSize; // don't use compact
    }
    
    StorageAdviser::Recommendation recommendation;

    try {

        recommendation =
            StorageAdviser::recommend(m_criteria, minimumSize, maximumSize);

    } catch (InsufficientDiscSpace s) {

        // Delete any unused servers we may have been leaving around
        // in case we wanted them again

        purgeLimbo(0);

        // This time we don't catch InsufficientDiscSpace -- we
        // haven't allocated anything yet and can safely let the
        // exception out to indicate to the caller that we can't
        // handle it.

        recommendation =
            StorageAdviser::recommend(m_criteria, minimumSize, maximumSize);
    }

//    cerr << "Recommendation was: " << recommendation << endl;

    memoryCache = false;

    if ((recommendation & StorageAdviser::UseMemory) ||
        (recommendation & StorageAdviser::PreferMemory)) {
        memoryCache = true;
    }

    compactCache = canCompact &&
        (recommendation & StorageAdviser::ConserveSpace);

#ifdef DEBUG_FFT_SERVER
    cerr << "FFTDataServer: memory cache = " << memoryCache << ", compact cache = " << compactCache << endl;
    
    cerr << "Width " << w << " of " << m_width << ", height " << h << ", size " << w * h << endl;
#endif
}

bool
FFTDataServer::makeCache(int c)
{
    // Creating the cache could take a significant amount of time.  We
    // don't want to block readers on m_cacheVectorLock while this is
    // happening, but we do want to block any further calls to
    // makeCache.  So we use this lock solely to serialise this
    // particular function -- it isn't used anywhere else.

    QMutexLocker locker(&m_cacheCreationMutex);

    m_cacheVectorLock.lockForRead();
    if (m_caches[c]) {
        // someone else must have created the cache between our
        // testing for it and taking the mutex
        m_cacheVectorLock.unlock();
        return true;
    }
    m_cacheVectorLock.unlock();

    // Now m_cacheCreationMutex is held, but m_cacheVectorLock is not
    // -- readers can proceed, but callers to this function will block

    CacheBlock *cb = new CacheBlock;

    QString name = QString("%1-%2").arg(m_fileBaseName).arg(c);

    int width = m_cacheWidth;
    if (c * m_cacheWidth + width > m_width) {
        width = m_width - c * m_cacheWidth;
    }

    bool memoryCache = false;
    bool compactCache = false;

    getStorageAdvice(width, m_height, memoryCache, compactCache);

    bool success = false;

    if (memoryCache) {

        try {

            cb->memoryCache = new FFTMemoryCache
                (compactCache ? FFTCache::Compact :
                      m_polar ? FFTCache::Polar :
                                FFTCache::Rectangular,
                 width, m_height);

            success = true;

        } catch (std::bad_alloc) {

            delete cb->memoryCache;
            cb->memoryCache = 0;
            
            cerr << "WARNING: Memory allocation failed when creating"
                      << " FFT memory cache no. " << c << " of " << width 
                      << "x" << m_height << " (of total width " << m_width
                      << "): falling back to disc cache" << endl;

            memoryCache = false;
        }
    }

    if (!memoryCache) {

        try {
        
            cb->fileCacheWriter = new FFTFileCacheWriter
                (name,
                 compactCache ? FFTCache::Compact :
                      m_polar ? FFTCache::Polar :
                                FFTCache::Rectangular,
                 width, m_height);

            success = true;

        } catch (std::exception &e) {

            delete cb->fileCacheWriter;
            cb->fileCacheWriter = 0;
            
            cerr << "ERROR: Failed to construct disc cache for FFT data: "
                      << e.what() << endl;

            throw;
        }
    }

    m_cacheVectorLock.lockForWrite();

    m_caches[c] = cb;

    m_cacheVectorLock.unlock();

    return success;
}
 
bool
FFTDataServer::makeCacheReader(int c)
{
    // preconditions: m_caches[c] exists and contains a file writer;
    // m_cacheVectorLock is not locked by this thread
#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::makeCacheReader(" << c << ")" << std::endl;
#endif

    QThread *me = QThread::currentThread();
    QWriteLocker locker(&m_cacheVectorLock);
    CacheBlock *cb(m_caches.at(c));
    if (!cb || !cb->fileCacheWriter) return false;

    try {
        
        cb->fileCacheReader[me] = new FFTFileCacheReader(cb->fileCacheWriter);

    } catch (std::exception &e) {

        delete cb->fileCacheReader[me];
        cb->fileCacheReader.erase(me);
            
        cerr << "ERROR: Failed to construct disc cache reader for FFT data: "
                  << e.what() << endl;
        return false;
    }

    // erase a reader that looks like it may no longer going to be
    // used by this thread for a while (leaving alone the current
    // and previous cache readers)
    int deleteCandidate = c - 2;
    if (deleteCandidate < 0) deleteCandidate = c + 2;
    if (deleteCandidate >= (int)m_caches.size()) {
        return true;
    }

    cb = m_caches.at(deleteCandidate);
    if (cb && cb->fileCacheReader.find(me) != cb->fileCacheReader.end()) {
#ifdef DEBUG_FFT_SERVER
        std::cerr << "FFTDataServer::makeCacheReader: Deleting probably unpopular reader " << deleteCandidate << " for this thread (as I create reader " << c << ")" << std::endl;
#endif
        delete cb->fileCacheReader[me];
        cb->fileCacheReader.erase(me);
    }
            
    return true;
}
       
float
FFTDataServer::getMagnitudeAt(int x, int y)
{
    Profiler profiler("FFTDataServer::getMagnitudeAt", false);

    if (x >= m_width || y >= m_height) return 0;

    float val = 0;

    try {
        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return 0;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getMagnitudeAt: filling");
#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::getMagnitudeAt: calling fillColumn("
                  << x << ")" << std::endl;
#endif
            fillColumn(x);
        }

        val = cache->getMagnitudeAt(col, y);

    } catch (std::exception &e) {
        m_error = e.what();
    }

    return val;
}

bool
FFTDataServer::getMagnitudesAt(int x, float *values, int minbin, int count, int step)
{
    Profiler profiler("FFTDataServer::getMagnitudesAt", false);

    if (x >= m_width) return false;

    if (minbin >= m_height) minbin = m_height - 1;
    if (count == 0) count = (m_height - minbin) / step;
    else if (minbin + count * step > m_height) {
        count = (m_height - minbin) / step;
    }

    try {
        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return false;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getMagnitudesAt: filling");
            fillColumn(x);
        }

        cache->getMagnitudesAt(col, values, minbin, count, step);

    } catch (std::exception &e) {
        m_error = e.what();
        return false;
    }

    return true;
}

float
FFTDataServer::getNormalizedMagnitudeAt(int x, int y)
{
    Profiler profiler("FFTDataServer::getNormalizedMagnitudeAt", false);

    if (x >= m_width || y >= m_height) return 0;

    float val = 0;

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return 0;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getNormalizedMagnitudeAt: filling");
            fillColumn(x);
        }
        val = cache->getNormalizedMagnitudeAt(col, y);

    } catch (std::exception &e) {
        m_error = e.what();
    }

    return val;
}

bool
FFTDataServer::getNormalizedMagnitudesAt(int x, float *values, int minbin, int count, int step)
{
    Profiler profiler("FFTDataServer::getNormalizedMagnitudesAt", false);

    if (x >= m_width) return false;

    if (minbin >= m_height) minbin = m_height - 1;
    if (count == 0) count = (m_height - minbin) / step;
    else if (minbin + count * step > m_height) {
        count = (m_height - minbin) / step;
    }

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return false;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getNormalizedMagnitudesAt: filling");
            fillColumn(x);
        }
        
        for (int i = 0; i < count; ++i) {
            values[i] = cache->getNormalizedMagnitudeAt(col, i * step + minbin);
        }
        
    } catch (std::exception &e) {
        m_error = e.what();
        return false;
    }

    return true;
}

float
FFTDataServer::getMaximumMagnitudeAt(int x)
{
    Profiler profiler("FFTDataServer::getMaximumMagnitudeAt", false);

    if (x >= m_width) return 0;

    float val = 0;

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return 0;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getMaximumMagnitudeAt: filling");
            fillColumn(x);
        }
        val = cache->getMaximumMagnitudeAt(col);

    } catch (std::exception &e) {
        m_error = e.what();
    }

    return val;
}

float
FFTDataServer::getPhaseAt(int x, int y)
{
    Profiler profiler("FFTDataServer::getPhaseAt", false);

    if (x >= m_width || y >= m_height) return 0;

    float val = 0;

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return 0;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getPhaseAt: filling");
            fillColumn(x);
        }
        val = cache->getPhaseAt(col, y);

    } catch (std::exception &e) {
        m_error = e.what();
    }

    return val;
}

bool
FFTDataServer::getPhasesAt(int x, float *values, int minbin, int count, int step)
{
    Profiler profiler("FFTDataServer::getPhasesAt", false);

    if (x >= m_width) return false;

    if (minbin >= m_height) minbin = m_height - 1;
    if (count == 0) count = (m_height - minbin) / step;
    else if (minbin + count * step > m_height) {
        count = (m_height - minbin) / step;
    }

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return false;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getPhasesAt: filling");
            fillColumn(x);
        }
        
        for (int i = 0; i < count; ++i) {
            values[i] = cache->getPhaseAt(col, i * step + minbin);
        }

    } catch (std::exception &e) {
        m_error = e.what();
        return false;
    }

    return true;
}

void
FFTDataServer::getValuesAt(int x, int y, float &real, float &imaginary)
{
    Profiler profiler("FFTDataServer::getValuesAt", false);

    if (x >= m_width || y >= m_height) {
        real = 0;
        imaginary = 0;
        return;
    }

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);

        if (!cache) {
            real = 0;
            imaginary = 0;
            return;
        }

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getValuesAt: filling");
#ifdef DEBUG_FFT_SERVER
            std::cerr << "FFTDataServer::getValuesAt(" << x << ", " << y << "): filling" << std::endl;
#endif
            fillColumn(x);
        }        

        cache->getValuesAt(col, y, real, imaginary);

    } catch (std::exception &e) {
        m_error = e.what();
    }
}

bool
FFTDataServer::getValuesAt(int x, float *reals, float *imaginaries, int minbin, int count, int step)
{
    Profiler profiler("FFTDataServer::getValuesAt", false);

    if (x >= m_width) return false;

    if (minbin >= m_height) minbin = m_height - 1;
    if (count == 0) count = (m_height - minbin) / step;
    else if (minbin + count * step > m_height) {
        count = (m_height - minbin) / step;
    }

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return false;

        if (!cache->haveSetColumnAt(col)) {
            Profiler profiler("FFTDataServer::getValuesAt: filling");
            fillColumn(x);
        }

        for (int i = 0; i < count; ++i) {
            cache->getValuesAt(col, i * step + minbin, reals[i], imaginaries[i]);
        }

    } catch (std::exception &e) {
        m_error = e.what();
        return false;
    }

    return true;
}

bool
FFTDataServer::isColumnReady(int x)
{
    Profiler profiler("FFTDataServer::isColumnReady", false);

    if (x >= m_width) return true;

    if (!haveCache(x)) {
/*!!!
        if (m_lastUsedCache == -1) {
            if (m_suspended) {
                std::cerr << "FFTDataServer::isColumnReady(" << x << "): no cache, calling resume" << std::endl;
                resume();
            }
            m_fillThread->start();
        }
*/
        return false;
    }

    try {

        int col;
        FFTCacheReader *cache = getCacheReader(x, col);
        if (!cache) return true;

        return cache->haveSetColumnAt(col);

    } catch (std::exception &e) {
        m_error = e.what();
        return false;
    }
}    

void
FFTDataServer::fillColumn(int x)
{
    Profiler profiler("FFTDataServer::fillColumn", false);

    if (!m_model->isReady()) {
        cerr << "WARNING: FFTDataServer::fillColumn(" 
                  << x << "): model not yet ready" << endl;
        return;
    }
/*
    if (!m_fftInput) {
        cerr << "WARNING: FFTDataServer::fillColumn(" << x << "): "
                  << "input has already been completed and discarded?"
                  << endl;
        return;
    }
*/
    if (x >= m_width) {
        cerr << "WARNING: FFTDataServer::fillColumn(" << x << "): "
                  << "x > width (" << x << " > " << m_width << ")"
                  << endl;
        return;
    }

    int col;
#ifdef DEBUG_FFT_SERVER_FILL
    cout << "FFTDataServer::fillColumn(" << x << ")" << endl;
#endif
    FFTCacheWriter *cache = getCacheWriter(x, col);
    if (!cache) return;

    int winsize = m_windowSize;
    int fftsize = m_fftSize;
    int hs = fftsize/2;

    sv_frame_t pfx = 0;
    int off = (fftsize - winsize) / 2;

    sv_frame_t startFrame = m_windowIncrement * sv_frame_t(x);
    sv_frame_t endFrame = startFrame + m_windowSize;

    startFrame -= winsize / 2;
    endFrame   -= winsize / 2;

#ifdef DEBUG_FFT_SERVER_FILL
    std::cerr << "FFTDataServer::fillColumn: requesting frames "
              << startFrame + pfx << " -> " << endFrame << " ( = "
              << endFrame - (startFrame + pfx) << ") at index "
              << off + pfx << " in buffer of size " << m_fftSize
              << " with window size " << m_windowSize 
              << " from channel " << m_channel << std::endl;
#endif

    QMutexLocker locker(&m_fftBuffersLock);

    // We may have been called from a function that wanted to obtain a
    // column using an FFTCacheReader.  Before calling us, it checked
    // whether the column was available already, and the reader
    // reported that it wasn't.  Now we test again, with the mutex
    // held, to avoid a race condition in case another thread has
    // called fillColumn at the same time.
    if (cache->haveSetColumnAt(x & m_cacheWidthMask)) {
        return;
    }

    if (!m_fftInput) {
        cerr << "WARNING: FFTDataServer::fillColumn(" << x << "): "
                  << "input has already been completed and discarded?"
                  << endl;
        return;
    }

    for (int i = 0; i < off; ++i) {
        m_fftInput[i] = 0.0;
    }

    for (int i = 0; i < off; ++i) {
        m_fftInput[fftsize - i - 1] = 0.0;
    }

    if (startFrame < 0) {
	pfx = -startFrame;
	for (int i = 0; i < pfx; ++i) {
	    m_fftInput[off + i] = 0.0;
	}
    }

    sv_frame_t count = 0;
    if (endFrame > startFrame + pfx) count = endFrame - (startFrame + pfx);

    sv_frame_t got = m_model->getData(m_channel, startFrame + pfx,
                                      count, m_fftInput + off + pfx);

    while (got + pfx < winsize) {
	m_fftInput[off + got + pfx] = 0.0;
	++got;
    }

    if (m_channel == -1) {
	int channels = m_model->getChannelCount();
	if (channels > 1) {
	    for (int i = 0; i < winsize; ++i) {
		m_fftInput[off + i] /= float(channels);
	    }
	}
    }

    m_windower.cut(m_fftInput + off);

    for (int i = 0; i < hs; ++i) {
	fftsample temp = m_fftInput[i];
	m_fftInput[i] = m_fftInput[i + hs];
	m_fftInput[i + hs] = temp;
    }

    fftf_execute(m_fftPlan);

    float factor = 0.f;

    if (cache->getStorageType() == FFTCache::Compact ||
        cache->getStorageType() == FFTCache::Polar) {

        for (int i = 0; i <= hs; ++i) {
            fftsample real = m_fftOutput[i][0];
            fftsample imag = m_fftOutput[i][1];
            float mag = sqrtf(real * real + imag * imag);
            m_workbuffer[i] = mag;
            m_workbuffer[i + hs + 1] = atan2f(imag, real);
            if (mag > factor) factor = mag;
        }

    } else {

        for (int i = 0; i <= hs; ++i) {
            m_workbuffer[i] = m_fftOutput[i][0];
            m_workbuffer[i + hs + 1] = m_fftOutput[i][1];
        }
    }

    Profiler subprof("FFTDataServer::fillColumn: set to cache");

    if (cache->getStorageType() == FFTCache::Compact ||
        cache->getStorageType() == FFTCache::Polar) {
            
        cache->setColumnAt(col,
                           m_workbuffer,
                           m_workbuffer + hs + 1,
                           factor);

    } else {

        cache->setColumnAt(col,
                           m_workbuffer,
                           m_workbuffer + hs + 1);
    }

    if (m_suspended) {
//        std::cerr << "FFTDataServer::fillColumn(" << x << "): calling resume" << std::endl;
//        resume();
    }
}    

void
FFTDataServer::fillComplete()
{
    for (int i = 0; i < int(m_caches.size()); ++i) {
        if (!m_caches[i]) continue;
        if (m_caches[i]->memoryCache) {
            m_caches[i]->memoryCache->allColumnsWritten();
        }
        if (m_caches[i]->fileCacheWriter) {
            m_caches[i]->fileCacheWriter->allColumnsWritten();
        }
    }
}

QString
FFTDataServer::getError() const
{
    if (m_error != "") return m_error;
    else if (m_fillThread) return m_fillThread->getError();
    else return "";
}

int
FFTDataServer::getFillCompletion() const 
{
    if (m_fillThread) return m_fillThread->getCompletion();
    else return 100;
}

sv_frame_t
FFTDataServer::getFillExtent() const
{
    if (m_fillThread) return m_fillThread->getExtent();
    else return m_model->getEndFrame();
}

QString
FFTDataServer::generateFileBasename() const
{
    return generateFileBasename(m_model, m_channel, m_windower.getType(),
                                m_windowSize, m_windowIncrement, m_fftSize,
                                m_polar);
}

QString
FFTDataServer::generateFileBasename(const DenseTimeValueModel *model,
                                    int channel,
                                    WindowType windowType,
                                    int windowSize,
                                    int windowIncrement,
                                    int fftSize,
                                    bool polar)
{
    return QString("%1-%2-%3-%4-%5-%6%7")
        .arg(XmlExportable::getObjectExportId(model))
        .arg(channel + 1)
        .arg((int)windowType)
        .arg(windowSize)
        .arg(windowIncrement)
        .arg(fftSize)
        .arg(polar ? "-p" : "-r");
}

void
FFTDataServer::FillThread::run()
{
#ifdef DEBUG_FFT_SERVER_FILL
    std::cerr << "FFTDataServer::FillThread::run()" << std::endl;
#endif
    
    m_extent = 0;
    m_completion = 0;
    
    while (!m_server.m_model->isReady() && !m_server.m_exiting) {
#ifdef DEBUG_FFT_SERVER_FILL
        std::cerr << "FFTDataServer::FillThread::run(): waiting for model " << m_server.m_model << " to be ready" << std::endl;
#endif
        sleep(1);
    }
    if (m_server.m_exiting) return;

    sv_frame_t start = m_server.m_model->getStartFrame();
    sv_frame_t end = m_server.m_model->getEndFrame();
    sv_frame_t remainingEnd = end;

    int counter = 0;
    int updateAt = 1;
    int maxUpdateAt = int(end / m_server.m_windowIncrement) / 20;
    if (maxUpdateAt < 100) maxUpdateAt = 100;

    if (m_fillFrom > start) {

        for (sv_frame_t f = m_fillFrom; f < end; f += m_server.m_windowIncrement) {
	    
            try {
                m_server.fillColumn(int((f - start) / m_server.m_windowIncrement));
            } catch (std::exception &e) {
                std::cerr << "FFTDataServer::FillThread::run: exception: " << e.what() << std::endl;
                m_error = e.what();
                m_server.fillComplete();
                m_completion = 100;
                m_extent = end;
                return;
            }

            if (m_server.m_exiting) return;

            while (m_server.m_suspended) {
#ifdef DEBUG_FFT_SERVER
                cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): suspended, waiting..." << endl;
#endif
                MutexLocker locker(&m_server.m_fftBuffersLock,
                                   "FFTDataServer::run::m_fftBuffersLock [1]");
                if (m_server.m_suspended && !m_server.m_exiting) {
                    m_server.m_condition.wait(&m_server.m_fftBuffersLock, 10000);
                }
#ifdef DEBUG_FFT_SERVER
                cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): waited" << endl;
#endif
                if (m_server.m_exiting) return;
            }

            if (++counter == updateAt) {
                m_extent = f;
                m_completion = int(100 * fabsf(float(f - m_fillFrom) /
                                                  float(end - start)));
                counter = 0;
                if (updateAt < maxUpdateAt) {
                    updateAt *= 2;
                    if (updateAt > maxUpdateAt) updateAt = maxUpdateAt;
                }
            }
        }

        remainingEnd = m_fillFrom;
        if (remainingEnd > start) --remainingEnd;
        else remainingEnd = start;
    }

    int baseCompletion = m_completion;

    for (sv_frame_t f = start; f < remainingEnd; f += m_server.m_windowIncrement) {

        try {
            m_server.fillColumn(int((f - start) / m_server.m_windowIncrement));
        } catch (std::exception &e) {
            std::cerr << "FFTDataServer::FillThread::run: exception: " << e.what() << std::endl;
            m_error = e.what();
            m_server.fillComplete();
            m_completion = 100;
            m_extent = end;
            return;
        }

        if (m_server.m_exiting) return;

        while (m_server.m_suspended) {
#ifdef DEBUG_FFT_SERVER
            cerr << "FFTDataServer(" << this << " [" << (void *)QThread::currentThreadId() << "]): suspended, waiting..." << endl;
#endif
            {
                MutexLocker locker(&m_server.m_fftBuffersLock,
                                   "FFTDataServer::run::m_fftBuffersLock [2]");
                if (m_server.m_suspended && !m_server.m_exiting) {
                    m_server.m_condition.wait(&m_server.m_fftBuffersLock, 10000);
                }
            }
            if (m_server.m_exiting) return;
        }
		    
        if (++counter == updateAt) {
            m_extent = f;
            m_completion = baseCompletion +
                int(100 * fabsf(float(f - start) /
                                   float(end - start)));
            counter = 0;
            if (updateAt < maxUpdateAt) {
                updateAt *= 2;
                if (updateAt > maxUpdateAt) updateAt = maxUpdateAt;
            }
        }
    }

    m_server.fillComplete();
    m_completion = 100;
    m_extent = end;

#ifdef DEBUG_FFT_SERVER
    std::cerr << "FFTDataServer::FillThread::run exiting" << std::endl;
#endif
}

