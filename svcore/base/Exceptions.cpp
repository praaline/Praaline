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

#include "Exceptions.h"

#include <iostream>

#include "Debug.h"

FileNotFound::FileNotFound(QString file) throw() :
    m_file(file)
{
    cerr << "ERROR: File not found: "
              << file << endl;
}

const char *
FileNotFound::what() const throw()
{
    return QString("File \"%1\" not found")
        .arg(m_file).toLocal8Bit().data();
}

FailedToOpenFile::FailedToOpenFile(QString file) throw() :
    m_file(file)
{
    cerr << "ERROR: Failed to open file: "
              << file << endl;
}

const char *
FailedToOpenFile::what() const throw()
{
    return QString("Failed to open file \"%1\"")
        .arg(m_file).toLocal8Bit().data();
}

DirectoryCreationFailed::DirectoryCreationFailed(QString directory) throw() :
    m_directory(directory)
{
    cerr << "ERROR: Directory creation failed for directory: "
              << directory << endl;
}

const char *
DirectoryCreationFailed::what() const throw()
{
    return QString("Directory creation failed for \"%1\"")
        .arg(m_directory).toLocal8Bit().data();
}

FileReadFailed::FileReadFailed(QString file) throw() :
    m_file(file)
{
    cerr << "ERROR: File read failed for file: "
              << file << endl;
}

const char *
FileReadFailed::what() const throw()
{
    return QString("File read failed for \"%1\"")
        .arg(m_file).toLocal8Bit().data();
}

FileOperationFailed::FileOperationFailed(QString file, QString op) throw() :
    m_file(file),
    m_operation(op)
{
    cerr << "ERROR: File " << op << " failed for file: "
              << file << endl;
}

const char *
FileOperationFailed::what() const throw()
{
    return QString("File %1 failed for \"%2\"")
        .arg(m_operation).arg(m_file).toLocal8Bit().data();
}

InsufficientDiscSpace::InsufficientDiscSpace(QString directory,
                                             size_t required,
                                             size_t available) throw() :
    m_directory(directory),
    m_required(required),
    m_available(available)
{
    cerr << "ERROR: Not enough disc space available in "
              << directory << ": need " << required
              << ", only have " << available << endl;
}

InsufficientDiscSpace::InsufficientDiscSpace(QString directory) throw() :
    m_directory(directory),
    m_required(0),
    m_available(0)
{
    cerr << "ERROR: Not enough disc space available in "
              << directory << endl;
}

const char *
InsufficientDiscSpace::what() const throw()
{
    if (m_required > 0) {
        return QString("Not enough space available in \"%1\": need %2, have %3")
            .arg(m_directory).arg(m_required).arg(m_available).toLocal8Bit().data();
    } else {
        return QString("Not enough space available in \"%1\"")
            .arg(m_directory).toLocal8Bit().data();
    }
}

AllocationFailed::AllocationFailed(QString purpose) throw() :
    m_purpose(purpose)
{
    cerr << "ERROR: Allocation failed: " << purpose
              << endl;
}

const char *
AllocationFailed::what() const throw()
{
    return QString("Allocation failed: %1")
        .arg(m_purpose).toLocal8Bit().data();
}


