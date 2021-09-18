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

#include "System.h"

#include <QStringList>
#include <QString>

#include <stdint.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/statvfs.h>
#include <locale.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/param.h>
#include <sys/sysctl.h>
#endif

#include <limits.h>
#include <cstdlib>

#include <iostream>

#ifdef __APPLE__
extern "C" {
void *
rpl_realloc (void *p, size_t n)
{
    p = realloc(p, n);
    if (p == 0 && n == 0)
    {
    p = malloc(0);
    }
    return p;
}
}
#endif

#ifdef _WIN32

extern "C" {

/* usleep is now in mingw
void usleep(unsigned long usec)
{
    ::Sleep(usec / 1000);
}
*/

int gettimeofday(struct timeval *tv, void *tz)
{
    union { 
	long long ns100;  
	FILETIME ft; 
    } now; 
    
    ::GetSystemTimeAsFileTime(&now.ft); 
    tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL); 
    tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL); 
    return 0;
}

}

#endif

ProcessStatus
GetProcessStatus(int pid)
{
#ifdef _WIN32
    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!handle) {
        return ProcessNotRunning;
    } else {
        CloseHandle(handle);
        return ProcessRunning;
    }
#else
    if (kill(getpid(), 0) == 0) {
        if (kill(pid, 0) == 0) {
            return ProcessRunning;
        } else {
            return ProcessNotRunning;
        }
    } else {
        return UnknownProcessStatus;
    }
#endif
}

#ifdef _WIN32
/*  MEMORYSTATUSEX is missing from older Windows headers, so define a
    local replacement.  This trick from MinGW source code.  Ugh */
typedef struct
{
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORDLONG ullTotalPhys;
    DWORDLONG ullAvailPhys;
    DWORDLONG ullTotalPageFile;
    DWORDLONG ullAvailPageFile;
    DWORDLONG ullTotalVirtual;
    DWORDLONG ullAvailVirtual;
    DWORDLONG ullAvailExtendedVirtual;
} lMEMORYSTATUSEX;
typedef WINBOOL (WINAPI *PFN_MS_EX) (lMEMORYSTATUSEX*);
#endif

void
GetRealMemoryMBAvailable(ssize_t &available, ssize_t &total)
{
    available = -1;
    total = -1;

#ifdef _WIN32

    static bool checked = false;
    static bool exFound = false;
    static PFN_MS_EX ex;

    if (!checked) {

        HMODULE h = GetModuleHandleA("kernel32.dll");

        if (h) {
            if ((ex = (PFN_MS_EX)GetProcAddress(h, "GlobalMemoryStatusEx"))) {
                exFound = true;
            }
        }
        
        checked = true;
    }

    DWORDLONG wavail = 0;
    DWORDLONG wtotal = 0;

    if (exFound) {

        lMEMORYSTATUSEX lms;
	lms.dwLength = sizeof(lms);
	if (!ex(&lms)) {
            cerr << "WARNING: GlobalMemoryStatusEx failed: error code "
                      << GetLastError() << endl;
            return;
        }
        wavail = lms.ullAvailPhys;
        wtotal = lms.ullTotalPhys;

    } else {

        /* Fall back to GlobalMemoryStatus which is always available.
           but returns wrong results for physical memory > 4GB  */

	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	wavail = ms.dwAvailPhys;
        wtotal = ms.dwTotalPhys;
    }

    DWORDLONG size = wavail / 1048576;
    if (size > INT_MAX) size = INT_MAX;
    available = ssize_t(size);

    size = wtotal / 1048576;
    if (size > INT_MAX) size = INT_MAX;
    total = ssize_t(size);

    return;

#else
#ifdef __APPLE__

    unsigned int val;
    int mib[2];
    size_t size_sys;
    
    mib[0] = CTL_HW;

    mib[1] = HW_PHYSMEM;
    size_sys = sizeof(val);
    sysctl(mib, 2, &val, &size_sys, NULL, 0);
    if (val) total = val / 1048576;

    mib[1] = HW_USERMEM;
    size_sys = sizeof(val);
    sysctl(mib, 2, &val, &size_sys, NULL, 0);
    if (val) available = val / 1048576;

    return;

#else

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) return;

    char buf[256];
    while (!feof(meminfo)) {
        fgets(buf, 256, meminfo);
        bool isMemFree = (strncmp(buf, "MemFree:", 8) == 0);
        bool isMemTotal = (!isMemFree && (strncmp(buf, "MemTotal:", 9) == 0));
        if (isMemFree || isMemTotal) {
            QString line = QString(buf).trimmed();
            QStringList elements = line.split(' ', Qt::SkipEmptyParts);
            QString unit = "kB";
            if (elements.size() > 2) unit = elements[2];
            int size = elements[1].toInt();
//            cerr << "have size \"" << size << "\", unit \""
//                      << unit << "\"" << endl;
            if (unit.toLower() == "gb") size = size * 1024;
            else if (unit.toLower() == "mb") size = size;
            else if (unit.toLower() == "kb") size = size / 1024;
            else size = size / 1048576;

            if (isMemFree) available = size;
            else total = size;
        }
        if (available != -1 && total != -1) {
            fclose(meminfo);
            return;
        }
    }
    fclose(meminfo);

    return;

#endif
#endif
}

ssize_t
GetDiscSpaceMBAvailable(const char *path)
{
#ifdef _WIN32
    ULARGE_INTEGER available, total, totalFree;
    if (GetDiskFreeSpaceExA(path, &available, &total, &totalFree)) {
	  __int64 a = available.QuadPart;
        a /= 1048576;
        if (a > INT_MAX) a = INT_MAX;
        return ssize_t(a);
    } else {
        cerr << "WARNING: GetDiskFreeSpaceEx failed: error code "
                  << GetLastError() << endl;
        return -1;
    }
#else
    struct statvfs buf;
    if (!statvfs(path, &buf)) {
        // do the multiplies and divides in this order to reduce the
        // likelihood of arithmetic overflow
//        cerr << "statvfs(" << path << ") says available: " << buf.f_bavail << ", block size: " << buf.f_bsize << endl;
        uint64_t available = ((buf.f_bavail / 1024) * buf.f_bsize) / 1024;
        if (available > INT_MAX) available = INT_MAX;
        return ssize_t(available);
    } else {
        perror("statvfs failed");
        return -1;
    }
#endif
}

#ifdef _WIN32
extern void SystemMemoryBarrier()
{
#ifdef __MSVC__
    MemoryBarrier();
#else /* mingw */
    LONG Barrier = 0;
    __asm__ __volatile__("xchgl %%eax,%0 "
                         : "=r" (Barrier));
#endif
}
#else /* !_WIN32 */
#if !defined(__APPLE__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ == 0))
void
SystemMemoryBarrier()
{
    pthread_mutex_t dummy = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&dummy);
    pthread_mutex_unlock(&dummy);
}
#endif /* !defined(__APPLE__) etc */
#endif /* !_WIN32 */


static char *startupLocale = 0;

void
StoreStartupLocale()
{
    char *loc = setlocale(LC_ALL, 0);
    if (!loc) return;
    if (startupLocale) free(startupLocale);
    startupLocale = strdup(loc);
}

void
RestoreStartupLocale()
{
    if (!startupLocale) {
        setlocale(LC_ALL, "");
    } else {
        setlocale(LC_ALL, startupLocale);
    }
}

double mod(double x, double y) { return x - (y * floor(x / y)); }
float modf(float x, float y) { return x - (y * floorf(x / y)); }

double princarg(double a) { return mod(a + M_PI, -2 * M_PI) + M_PI; }
float princargf(float a) { return float(princarg(a)); }

