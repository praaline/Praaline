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

#include "UnitDatabase.h"

UnitDatabase
UnitDatabase::m_instance;

UnitDatabase *
UnitDatabase::getInstance()
{
    return &m_instance;
}

UnitDatabase::UnitDatabase() :
    m_nextId(0)
{
}

QStringList
UnitDatabase::getKnownUnits() const
{
    QStringList list;
    for (UnitMap::const_iterator i = m_units.begin(); i != m_units.end(); ++i) {
        list.push_back(i->first);
    }
    return list;
}

void
UnitDatabase::registerUnit(QString unit)
{
    if (m_units.find(unit) == m_units.end()) {
        m_units[unit] = m_nextId++;
        emit unitDatabaseChanged();
    }
}

int
UnitDatabase::getUnitId(QString unit, bool registerNew)
{
    if (m_units.find(unit) == m_units.end()) {
        if (registerNew) registerUnit(unit);
        else return -1;
    }
    return m_units[unit];
}

QString
UnitDatabase::getUnitById(int id)
{
    for (UnitMap::const_iterator i = m_units.begin(); i != m_units.end(); ++i) {
        if (i->second == id) return i->first;
    }
    return "";
}

