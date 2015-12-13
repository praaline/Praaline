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

#include "FileReadThread.h"

#include "base/Profiler.h"
#include "base/Thread.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

//#define DEBUG_FILE_READ_THREAD 1

FileReadThread::FileReadThread() :
    m_nextToken(0),
    m_exiting(false)
{
}

void
FileReadThread::run()
{
    MutexLocker locker(&m_mutex, "FileReadThread::run::m_mutex");

    while (!m_exiting) {
        if (m_queue.empty()) {
            m_condition.wait(&m_mutex, 1000);
        } else {
            process();
        }
        notifyCancelled();
    }

    notifyCancelled();

#ifdef DEBUG_FILE_READ_THREAD
    cerr << "FileReadThread::run() exiting" << endl;
#endif
}

void
FileReadThread::finish()
{
#ifdef DEBUG_FILE_READ_THREAD
    cerr << "FileReadThread::finish()" << endl;
#endif

    {
        MutexLocker locker(&m_mutex, "FileReadThread::finish::m_mutex");

        while (!m_queue.empty()) {
            m_cancelledRequests[m_queue.begin()->first] = m_queue.begin()->second;
            m_newlyCancelled.insert(m_queue.begin()->first);
            m_queue.erase(m_queue.begin());
        }
        
        m_exiting = true;
    }

    m_condition.wakeAll();

#ifdef DEBUG_FILE_READ_THREAD
    cerr << "FileReadThread::finish() exiting" << endl;
#endif
}

int
FileReadThread::request(const Request &request)
{
    int token;

    {
        MutexLocker locker(&m_mutex, "FileReadThread::request::m_mutex");
    
        token = m_nextToken++;
        m_queue[token] = request;
    }

    m_condition.wakeAll();

    return token;
}

void
FileReadThread::cancel(int token)
{
    {
        MutexLocker locker(&m_mutex, "FileReadThread::cancel::m_mutex");

        if (m_queue.find(token) != m_queue.end()) {
            m_cancelledRequests[token] = m_queue[token];
            m_queue.erase(token);
            m_newlyCancelled.insert(token);
        } else if (m_readyRequests.find(token) != m_readyRequests.end()) {
            m_cancelledRequests[token] = m_readyRequests[token];
            m_readyRequests.erase(token);
        } else {
            cerr << "WARNING: FileReadThread::cancel: token " << token << " not found" << endl;
        }
    }

#ifdef DEBUG_FILE_READ_THREAD
    cerr << "FileReadThread::cancel(" << token << ") waking condition" << endl;
#endif

    m_condition.wakeAll();
}

bool
FileReadThread::isReady(int token)
{
    MutexLocker locker(&m_mutex, "FileReadThread::isReady::m_mutex");

    bool ready = m_readyRequests.find(token) != m_readyRequests.end();

    return ready;
}

bool
FileReadThread::isCancelled(int token)
{
    MutexLocker locker(&m_mutex, "FileReadThread::isCancelled::m_mutex");

    bool cancelled = 
        m_cancelledRequests.find(token) != m_cancelledRequests.end() &&
        m_newlyCancelled.find(token) == m_newlyCancelled.end();

    return cancelled;
}

bool
FileReadThread::haveRequest(int token)
{
    MutexLocker locker(&m_mutex, "FileReadThread::haveRequest::m_mutex");

    bool found = false;

    if (m_queue.find(token) != m_queue.end()) {
        found = true;
    } else if (m_cancelledRequests.find(token) != m_cancelledRequests.end()) {
        found = true;
    } else if (m_readyRequests.find(token) != m_readyRequests.end()) {
        found = true;
    }

    return found;
}

bool
FileReadThread::getRequest(int token, Request &request)
{
    MutexLocker locker(&m_mutex, "FileReadThread::getRequest::m_mutex");

    bool found = false;

    if (m_queue.find(token) != m_queue.end()) {
        request = m_queue[token];
        found = true;
    } else if (m_cancelledRequests.find(token) != m_cancelledRequests.end()) {
        request = m_cancelledRequests[token];
        found = true;
    } else if (m_readyRequests.find(token) != m_readyRequests.end()) {
        request = m_readyRequests[token];
        found = true;
    }

    return found;
}

void
FileReadThread::done(int token)
{
    MutexLocker locker(&m_mutex, "FileReadThread::done::m_mutex");

    bool found = false;

    if (m_cancelledRequests.find(token) != m_cancelledRequests.end()) {
        m_cancelledRequests.erase(token);
        m_newlyCancelled.erase(token);
        found = true;
    } else if (m_readyRequests.find(token) != m_readyRequests.end()) {
        m_readyRequests.erase(token);
        found = true;
    } else if (m_queue.find(token) != m_queue.end()) {
        cerr << "WARNING: FileReadThread::done(" << token << "): request is still in queue (wait or cancel it)" << endl;
    }

    if (!found) {
        cerr << "WARNING: FileReadThread::done(" << token << "): request not found" << endl;
    }
}

void
FileReadThread::process()
{
    // entered with m_mutex locked and m_queue non-empty

    Profiler profiler("FileReadThread::process", true);

    int token = m_queue.begin()->first;
    Request request = m_queue.begin()->second;

    m_mutex.unlock();

#ifdef DEBUG_FILE_READ_THREAD
    cerr << "FileReadThread::process: reading " << request.start << ", " << request.size << " on " << request.fd << endl;
#endif

    bool successful = false;
    bool seekFailed = false;
    ssize_t r = 0;

    { 
        MutexLocker rlocker(request.mutex, "FileReadThread::process::request.mutex");

        if (::lseek(request.fd, request.start, SEEK_SET) == (off_t)-1) {
            seekFailed = true;
        } else {
        
            // if request.size is large, we want to avoid making a single
            // system call to read it all as it may block too much
            
            static const size_t blockSize = 256 * 1024;
            
            size_t size = request.size;
            char *destination = request.data;
            
            while (size > 0) {
                size_t readSize = size;
                if (readSize > blockSize) readSize = blockSize;
                ssize_t br = ::read(request.fd, destination, readSize);
                if (br < 0) { 
                    r = br;
                    break;
                } else {
                    r += br;
                    if (br < ssize_t(readSize)) break;
                }
                destination += readSize;
                size -= readSize;
            }
        }
    }

    if (seekFailed) {
        ::perror("Seek failed");
        cerr << "ERROR: FileReadThread::process: seek to "
                  << request.start << " failed" << endl;
        request.size = 0;
    } else {
        if (r < 0) {
            ::perror("ERROR: FileReadThread::process: Read failed");
            cerr << "ERROR: FileReadThread::process: read of "
                      << request.size << " at "
                      << request.start << " failed" << endl;
            request.size = 0;
        } else if (r < ssize_t(request.size)) {
            cerr << "WARNING: FileReadThread::process: read "
                      << request.size << " returned only " << r << " bytes"
                      << endl;
            request.size = r;
            usleep(100000);
        } else {
            successful = true;
        }
    }
        
    // Check that the token hasn't been cancelled and the thread
    // hasn't been asked to finish
    
    m_mutex.lock();

    request.successful = successful;
        
    if (m_queue.find(token) != m_queue.end() && !m_exiting) {
        m_queue.erase(token);
        m_readyRequests[token] = request;
#ifdef DEBUG_FILE_READ_THREAD
        cerr << "FileReadThread::process: done, marking as ready (success = " << m_readyRequests[token].successful << ")" << endl;
#endif
    } else {
#ifdef DEBUG_FILE_READ_THREAD
        if (m_exiting) {
            cerr << "FileReadThread::process: exiting" << endl;
        } else {
            cerr << "FileReadThread::process: request disappeared" << endl;
        }
#endif
    }
}

void
FileReadThread::notifyCancelled()
{
    // entered with m_mutex locked

    while (!m_newlyCancelled.empty()) {

        int token = *m_newlyCancelled.begin();

#ifdef DEBUG_FILE_READ_THREAD
        cerr << "FileReadThread::notifyCancelled: token " << token << endl;
#endif

        m_newlyCancelled.erase(token);
    }
}
        
    
