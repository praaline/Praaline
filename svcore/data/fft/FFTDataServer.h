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

#ifndef _FFT_DATA_SERVER_H_
#define _FFT_DATA_SERVER_H_

#include "base/BaseTypes.h"
#include "base/Window.h"
#include "base/Thread.h"
#include "base/StorageAdviser.h"

#include "FFTapi.h"
#include "FFTFileCacheReader.h"
#include "FFTFileCacheWriter.h"
#include "FFTMemoryCache.h"

#include <QMutex>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWaitCondition>
#include <QString>

#include <vector>
#include <deque>

class DenseTimeValueModel;
class Model;

class FFTDataServer
{
public:
    static FFTDataServer *getInstance(const DenseTimeValueModel *model,
                                      int channel,
                                      WindowType windowType,
                                      int windowSize,
                                      int windowIncrement,
                                      int fftSize,
                                      bool polar,
                                      StorageAdviser::Criteria criteria =
                                          StorageAdviser::NoCriteria,
                                      sv_frame_t fillFromFrame = 0);

    static FFTDataServer *getFuzzyInstance(const DenseTimeValueModel *model,
                                           int channel,
                                           WindowType windowType,
                                           int windowSize,
                                           int windowIncrement,
                                           int fftSize,
                                           bool polar,
                                           StorageAdviser::Criteria criteria =
                                               StorageAdviser::NoCriteria,
                                           sv_frame_t fillFromFrame = 0);

    static void claimInstance(FFTDataServer *);
    static void releaseInstance(FFTDataServer *);

    static void modelAboutToBeDeleted(Model *);

    const DenseTimeValueModel *getModel() const { return m_model; }
    int        getChannel() const { return m_channel; }
    WindowType getWindowType() const { return m_windower.getType(); }
    int     getWindowSize() const { return m_windowSize; }
    int     getWindowIncrement() const { return m_windowIncrement; }
    int     getFFTSize() const { return m_fftSize; }
    bool       getPolar() const { return m_polar; }

    int     getWidth() const  { return m_width;  }
    int     getHeight() const { return m_height; }

    float      getMagnitudeAt(int x, int y);
    float      getNormalizedMagnitudeAt(int x, int y);
    float      getMaximumMagnitudeAt(int x);
    float      getPhaseAt(int x, int y);
    void       getValuesAt(int x, int y, float &real, float &imaginary);
    bool       isColumnReady(int x);

    bool       getMagnitudesAt(int x, float *values, int minbin = 0, int count = 0, int step = 1);
    bool       getNormalizedMagnitudesAt(int x, float *values, int minbin = 0, int count = 0, int step = 1);
    bool       getPhasesAt(int x, float *values, int minbin = 0, int count = 0, int step = 1);
    bool       getValuesAt(int x, float *reals, float *imaginaries, int minbin = 0, int count = 0, int step = 1);

    void       suspend();
    void       suspendWrites();
    void       resume(); // also happens automatically if new data needed

    // Convenience functions:

    bool isLocalPeak(int x, int y) {
        float mag = getMagnitudeAt(x, y);
        if (y > 0 && mag < getMagnitudeAt(x, y - 1)) return false;
        if (y < getHeight()-1 && mag < getMagnitudeAt(x, y + 1)) return false;
        return true;
    }
    bool isOverThreshold(int x, int y, float threshold) {
        return getMagnitudeAt(x, y) > threshold;
    }

    QString getError() const;
    int getFillCompletion() const;
    sv_frame_t getFillExtent() const;

private:
    FFTDataServer(QString fileBaseName,
                  const DenseTimeValueModel *model,
                  int channel,
                  WindowType windowType,
                  int windowSize,
                  int windowIncrement,
                  int fftSize,
                  bool polar,
                  StorageAdviser::Criteria criteria,
                  sv_frame_t fillFromFrame = 0);

    virtual ~FFTDataServer();

    FFTDataServer(const FFTDataServer &); // not implemented
    FFTDataServer &operator=(const FFTDataServer &); // not implemented

    typedef float fftsample;

    QString m_fileBaseName;
    const DenseTimeValueModel *m_model;
    int m_channel;

    Window<fftsample> m_windower;

    int m_windowSize;
    int m_windowIncrement;
    int m_fftSize;
    bool m_polar;

    int m_width;
    int m_height;
    int m_cacheWidth;
    int m_cacheWidthPower;
    int m_cacheWidthMask;

    struct CacheBlock {
        FFTMemoryCache *memoryCache;
        typedef std::map<QThread *, FFTFileCacheReader *> ThreadReaderMap;
        ThreadReaderMap fileCacheReader;
        FFTFileCacheWriter *fileCacheWriter;
        CacheBlock() : memoryCache(0), fileCacheWriter(0) { }
        ~CacheBlock() {
            delete memoryCache; 
            while (!fileCacheReader.empty()) {
                delete fileCacheReader.begin()->second;
                fileCacheReader.erase(fileCacheReader.begin());
            }
            delete fileCacheWriter;
        }
    };

    typedef std::vector<CacheBlock *> CacheVector;
    CacheVector m_caches;
    QReadWriteLock m_cacheVectorLock; // locks cache lookup, not use
    QMutex m_cacheCreationMutex; // solely to serialise makeCache() calls

    FFTCacheReader *getCacheReader(int x, int &col) {
        Profiler profiler("FFTDataServer::getCacheReader");
        col = x & m_cacheWidthMask;
        int c = x >> m_cacheWidthPower;
        m_cacheVectorLock.lockForRead();
        CacheBlock *cb(m_caches.at(c));
        if (cb) {
            if (cb->memoryCache) {
                m_cacheVectorLock.unlock();
                return cb->memoryCache;
            }
            if (cb->fileCacheWriter) {
                QThread *me = QThread::currentThread();
                CacheBlock::ThreadReaderMap &map = cb->fileCacheReader;
                if (map.find(me) == map.end()) {
                    m_cacheVectorLock.unlock();
                    if (!makeCacheReader(c)) return 0;
                    return getCacheReader(x, col);
                }
                FFTCacheReader *reader = cb->fileCacheReader[me];
                m_cacheVectorLock.unlock();
                return reader;
            }
            // if cb exists but cb->fileCacheWriter doesn't, creation
            // must have failed: don't try again
            m_cacheVectorLock.unlock();
            return 0;
        }
        m_cacheVectorLock.unlock();
        if (!makeCache(c)) return 0;
        return getCacheReader(x, col);
    }
    
    FFTCacheWriter *getCacheWriter(int x, int &col) {
        Profiler profiler("FFTDataServer::getCacheWriter");
        col = x & m_cacheWidthMask;
        int c = x >> m_cacheWidthPower;
        {
            QReadLocker locker(&m_cacheVectorLock);
            CacheBlock *cb(m_caches.at(c));
            if (cb) {
                if (cb->memoryCache) return cb->memoryCache;
                if (cb->fileCacheWriter) return cb->fileCacheWriter;
                // if cb exists, creation must have failed: don't try again
                return 0;
            }
        }
        if (!makeCache(c)) return 0;
        return getCacheWriter(x, col);
    }

    bool haveCache(int x) {
        int c = x >> m_cacheWidthPower;
        return (m_caches.at(c) != 0);
    }
    
    bool makeCache(int c);
    bool makeCacheReader(int c);
    
    StorageAdviser::Criteria m_criteria;

    void getStorageAdvice(int w, int h, bool &memory, bool &compact);
        
    QMutex m_fftBuffersLock;
    QWaitCondition m_condition;

    fftsample *m_fftInput;
    fftf_complex *m_fftOutput;
    float *m_workbuffer;
    fftf_plan m_fftPlan;

    class FillThread : public Thread
    {
    public:
        FillThread(FFTDataServer &server, sv_frame_t fillFromFrame) :
            m_server(server), m_extent(0), m_completion(0),
            m_fillFrom(fillFromFrame) { }

        sv_frame_t getExtent() const { return m_extent; }
        int getCompletion() const { return m_completion ? m_completion : 1; }
        QString getError() const { return m_error; }
        virtual void run();

    protected:
        FFTDataServer &m_server;
        sv_frame_t m_extent;
        int m_completion;
        sv_frame_t m_fillFrom;
        QString m_error;
    };

    bool m_exiting;
    bool m_suspended;
    FillThread *m_fillThread;
    QString m_error;

    void deleteProcessingData();
    void fillColumn(int x);
    void fillComplete();

    QString generateFileBasename() const;
    static QString generateFileBasename(const DenseTimeValueModel *model,
                                        int channel,
                                        WindowType windowType,
                                        int windowSize,
                                        int windowIncrement,
                                        int fftSize,
                                        bool polar);

    typedef std::pair<FFTDataServer *, int> ServerCountPair;
    typedef std::map<QString, ServerCountPair> ServerMap;
    typedef std::deque<FFTDataServer *> ServerQueue;

    static ServerMap m_servers;
    static ServerQueue m_releasedServers; // these are still in m_servers as well, with zero refcount
    static QMutex m_serverMapMutex;
    static FFTDataServer *findServer(QString); // call with serverMapMutex held
    static void purgeLimbo(int maxSize = 3); // call with serverMapMutex held

    static void claimInstance(FFTDataServer *, bool needLock);
    static void releaseInstance(FFTDataServer *, bool needLock);

};

#endif
