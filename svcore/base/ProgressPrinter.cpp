/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ProgressPrinter.h"

#include <iostream>

#include "Debug.h"

ProgressPrinter::ProgressPrinter(QString message, QObject *parent) :
    ProgressReporter(parent),
    m_prefix(message),
    m_lastProgress(0),
    m_definite(true)
{
    if (m_prefix.length() > 70) {
        m_prefix = m_prefix.left(70) + "...";
    }
}

ProgressPrinter::~ProgressPrinter()
{
    if (m_lastProgress > 0 && m_lastProgress != 100) {
        cerr << "\r\n";
    }
//    cerr << "(progress printer dtor)" << endl;
}

bool
ProgressPrinter::isDefinite() const
{
    return m_definite;
}

void
ProgressPrinter::setDefinite(bool definite)
{
    m_definite = definite;
}

void
ProgressPrinter::setMessage(QString message)
{
    m_prefix = message;
    if (m_prefix.length() > 70) {
        m_prefix = m_prefix.left(70) + "...";
    }
}

void
ProgressPrinter::done()
{
    cerr << "\r"
              << m_prefix 
              << (m_prefix == "" ? "" : " ")
              << "Done" << endl;
}

void
ProgressPrinter::setProgress(int progress)
{
    if (progress == m_lastProgress) return;
    cerr << "\r"
              << m_prefix 
              << (m_prefix == "" ? "" : " ");
    if (m_definite) {
        cerr << progress << "%";
    } else {
        cerr << "|/-\\"[progress % 4];
    }
    m_lastProgress = progress;
}

