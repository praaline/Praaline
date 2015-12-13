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

#ifndef _UNIT_DATABASE_H_
#define _UNIT_DATABASE_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <map>

// This grandly named class is just a list of the names of known scale
// units for the various models, for use as the set of fixed values in
// unit dropdown menus etc.  Of course, the user should be allowed to
// enter their own as well.

class UnitDatabase : public QObject
{
    Q_OBJECT

public:
    static UnitDatabase *getInstance();

    QStringList getKnownUnits() const;
    void registerUnit(QString unit);

    /**
     * Return the reference id for a given unit name.  If registerNew is
     * true and the unit is not known, register it and return its new
     * id.  If register is false and the unit is not known, return -1.
     */
    int getUnitId(QString unit, bool registerNew = true);

    QString getUnitById(int id);

signals:
    void unitDatabaseChanged();

protected:
    UnitDatabase();

    typedef std::map<QString, int> UnitMap;
    UnitMap m_units;
    int m_nextId;

    static UnitDatabase m_instance;
};

#endif

