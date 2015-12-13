/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Praaline
    A research tool for spoken language corpora.

    This file (Network Permission Tester) copyright:
    Sonic Visualiser. An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef NETWORKPERMISSIONTESTER_H
#define NETWORKPERMISSIONTESTER_H

class NetworkPermissionTester
{
public:
    NetworkPermissionTester() { }
    bool havePermission();
};

#endif // NETWORKPERMISSIONTESTER_H
