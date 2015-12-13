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

#include <iostream>

#include <qglobal.h>

#ifdef Q_OS_WIN32

#include <fcntl.h>

// required for SetDllDirectory
#define _WIN32_WINNT 0x0502
#include <windows.h>

// Set default file open mode to binary
//#undef _fmode
//int _fmode = _O_BINARY;

void redirectStderr()
{
#ifdef NO_PROBABLY_NOT
    HANDLE stderrHandle = GetStdHandle(STD_ERROR_HANDLE);
    if (!stderrHandle) return;

    AllocConsole();

    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(stderrHandle, &info);
    info.dwSize.Y = 1000;
    SetConsoleScreenBufferSize(stderrHandle, info.dwSize);

    int h = _open_osfhandle((long)stderrHandle, _O_TEXT);
    if (h) {
        FILE *fd = _fdopen(h, "w");
        if (fd) {
            *stderr = *fd;
            setvbuf(stderr, NULL, _IONBF, 0);
        }
    }
#endif
}

#endif

extern void svSystemSpecificInitialisation()
{
#ifdef Q_OS_WIN32
    redirectStderr();

    // Remove the CWD from the DLL search path, just in case
    SetDllDirectory(L"");
    putenv("PATH=");
#else
#endif
}



