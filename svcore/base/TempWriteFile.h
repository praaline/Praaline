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

#ifndef _TEMP_WRITE_FILE_H_
#define _TEMP_WRITE_FILE_H_

#include <QTemporaryFile>

/**
 * A class that manages the creation of a temporary file with a given
 * prefix and the renaming of that file to the prefix after use.  For
 * use when saving a file over an existing one, to avoid clobbering
 * the original before the save is complete.
 */

class TempWriteFile
{
public:
    TempWriteFile(QString targetFileName); // may throw FileOperationFailed

    /**
     * Destroy the temporary file object.  If moveToTarget has not
     * been called, the associated temporary file will be deleted
     * without being copied to the target location.
     */
    ~TempWriteFile();

    /**
     * Return the name of the temporary file.  Unless the constructor
     * threw an exception, this file will have been created already
     * (but it will not be open).
     *
     * (If moveToTarget has already been called, return an empty
     * string.)
     */
    QString getTemporaryFilename();

    /**
     * Rename the temporary file to the target filename.
     */
    void moveToTarget();

protected:
    QString m_target;
    QString m_temp;
};


#endif
