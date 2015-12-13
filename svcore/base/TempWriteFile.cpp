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

#include "TempWriteFile.h"

#include "Exceptions.h"

#include <QTemporaryFile>
#include <QDir>
#include <iostream>

TempWriteFile::TempWriteFile(QString target) :
    m_target(target)
{
    QTemporaryFile temp(m_target + ".");
    temp.setAutoRemove(false);
    temp.open(); // creates the file and opens it atomically
    if (temp.error()) {
	cerr << "TempWriteFile: Failed to create temporary file in directory of " << m_target << ": " << temp.errorString() << endl;
	throw FileOperationFailed(temp.fileName(), "creation");
    }
    
    m_temp = temp.fileName();
    temp.close(); // does not remove the file
}

TempWriteFile::~TempWriteFile()
{
    if (m_temp != "") {
	QDir dir(QFileInfo(m_temp).dir());
	dir.remove(m_temp);
    }
}

QString
TempWriteFile::getTemporaryFilename()
{
    return m_temp;
}

void
TempWriteFile::moveToTarget()
{
    if (m_temp == "") return;

    QDir dir(QFileInfo(m_temp).dir());
    // According to  http://doc.trolltech.com/4.4/qdir.html#rename
    // some systems fail, if renaming over an existing file.
    // Therefore, delete first the existing file.
    if (dir.exists(m_target)) dir.remove(m_target);
    if (!dir.rename(m_temp, m_target)) {
	cerr << "TempWriteFile: Failed to rename temporary file " << m_temp << " to target " << m_target << endl;
	throw FileOperationFailed(m_temp, "rename");
    }

    m_temp = "";
}
    
