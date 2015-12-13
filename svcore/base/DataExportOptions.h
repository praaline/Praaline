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

#ifndef DATA_EXPORT_OPTIONS_H
#define DATA_EXPORT_OPTIONS_H

enum DataExportOption
{
    DataExportDefaults = 0x0,
    DataExportFillGaps = 0x1,
    DataExportOmitLevels = 0x2,
};

typedef int DataExportOptions;

#endif
