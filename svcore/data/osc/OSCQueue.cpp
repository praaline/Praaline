/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam and QMUL.
*/

#include "OSCQueue.h"

#include "base/Profiler.h"

#include <iostream>
#include <unistd.h>

#define OSC_MESSAGE_QUEUE_SIZE 1023

#ifdef HAVE_LIBLO

void
OSCQueue::oscError(int num, const char *msg, const char *path)
{
    cerr << "ERROR: OSCQueue::oscError: liblo server error " << num
	      << " in path " << path << ": " << msg << endl;
}

int
OSCQueue::oscMessageHandler(const char *path, const char *types, lo_arg **argv,
                            int argc, lo_message, void *user_data)
{
    OSCQueue *queue = static_cast<OSCQueue *>(user_data);

    int target;
    int targetData;
    QString method;

    if (!queue->parseOSCPath(path, target, targetData, method)) {
	return 1;
    }

    OSCMessage message;
    message.setTarget(target);
    message.setTargetData(targetData);
    message.setMethod(method);

    int i = 0;

    while (types && i < argc && types[i]) {

        char type = types[i];
        lo_arg *arg = argv[i];

        switch (type) {
        case 'i': message.addArg(arg->i); break;
            // This conversion fails to compile in 64-bit environments
            // at present, and we don't use the h type anyway so we
            // can safely omit it
//        case 'h': message.addArg(arg->h); break;
        case 'f': message.addArg(arg->f); break;
        case 'd': message.addArg(arg->d); break;
        case 'c': message.addArg(arg->c); break;
        case 't': message.addArg(arg->i); break;
        case 's': message.addArg(&arg->s); break;
        default:  cerr << "WARNING: OSCQueue::oscMessageHandler: "
                            << "Unsupported OSC type '" << type << "'" 
                            << endl;
            break;
        }

	++i;
    }

    queue->postMessage(message);
    return 0;
}

#endif
   
OSCQueue::OSCQueue() :
#ifdef HAVE_LIBLO
    m_thread(0),
#endif
    m_buffer(OSC_MESSAGE_QUEUE_SIZE)
{
    Profiler profiler("OSCQueue::OSCQueue");

#ifdef HAVE_LIBLO
    m_thread = lo_server_thread_new(NULL, oscError);

    lo_server_thread_add_method(m_thread, NULL, NULL,
                                oscMessageHandler, this);

    lo_server_thread_start(m_thread);

    cout << "OSCQueue::OSCQueue: Base OSC URL is "
              << lo_server_thread_get_url(m_thread) << endl;
#endif
}

OSCQueue::~OSCQueue()
{
#ifdef HAVE_LIBLO
    if (m_thread) {
        lo_server_thread_stop(m_thread);
    }
#endif

    while (m_buffer.getReadSpace() > 0) {
        delete m_buffer.readOne();
    }
}

bool
OSCQueue::isOK() const
{
#ifdef HAVE_LIBLO
    return (m_thread != 0);
#else
    return false;
#endif
}

QString
OSCQueue::getOSCURL() const
{
    QString url = "";
#ifdef HAVE_LIBLO
    if (m_thread) url = lo_server_thread_get_url(m_thread);
#endif
    return url;
}

int
OSCQueue::getMessagesAvailable() const
{
    return m_buffer.getReadSpace();
}

OSCMessage
OSCQueue::readMessage()
{
    OSCMessage *message = m_buffer.readOne();
    OSCMessage rmessage = *message;
    delete message;
    return rmessage;
}

void
OSCQueue::postMessage(OSCMessage message)
{
    int count = 0, max = 5;
    while (m_buffer.getWriteSpace() == 0) {
        if (count == max) {
            cerr << "ERROR: OSCQueue::postMessage: OSC message queue is full and not clearing -- abandoning incoming message" << endl;
            return;
        }
        cerr << "WARNING: OSCQueue::postMessage: OSC message queue (capacity " << m_buffer.getSize() << " is full!" << endl;
        cerr << "Waiting for something to be processed" << endl;
#ifdef _WIN32
        Sleep(1);
#else
        sleep(1);
#endif
        count++;
    }

    OSCMessage *mp = new OSCMessage(message);
    m_buffer.write(&mp, 1);
    cerr << "OSCQueue::postMessage: Posted OSC message: target "
              << message.getTarget() << ", target data " << message.getTargetData()
              << ", method " << message.getMethod() << endl;
    emit messagesAvailable();
}

bool
OSCQueue::parseOSCPath(QString path, int &target, int &targetData,
                       QString &method)
{
    while (path.startsWith("/")) {
	path = path.right(path.length()-1);
    }

    int i = 0;

    bool ok = false;
    target = path.section('/', i, i).toInt(&ok);

    if (!ok) {
        target = 0;
    } else {
        ++i;
        targetData = path.section('/', i, i).toInt(&ok);
        if (!ok) {
            targetData = 0;
        } else {
            ++i;
        }
    }

    method = path.section('/', i, -1);

    if (method.contains('/')) {
        cerr << "ERROR: OSCQueue::parseOSCPath: malformed path \""
                  << path << "\" (should be target/data/method or "
                  << "target/method or method, where target and data "
                  << "are numeric)" << endl;
        return false;
    }

    cerr << "OSCQueue::parseOSCPath: good path \"" << path              << "\"" << endl;

    return true;
}

