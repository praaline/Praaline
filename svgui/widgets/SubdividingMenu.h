/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _SUBDIVIDING_MENU_H_
#define _SUBDIVIDING_MENU_H_

#include <QMenu>

#include <QString>
#include <set>
#include <map>

/**
 * A menu that divides its entries into submenus, alphabetically.  For
 * menus that may contain a very large or small number of named items
 * (e.g. plugins).
 *
 * The menu needs to be told, before any of the actions are added,
 * what the set of entry strings will be, so it can determine a
 * reasonable categorisation.  Do this by calling the setEntries()
 * method.  If it isn't practical to do this in advance, then add the
 * entries and call entriesAdded() afterwards instead. 
 */

class SubdividingMenu : public QMenu
{
    Q_OBJECT

public:
    SubdividingMenu(int lowerLimit = 0, int upperLimit = 0,
                    QWidget *parent = 0);
    SubdividingMenu(const QString &title, int lowerLimit = 0,
                    int upperLimit = 0, QWidget *parent = 0);
    virtual ~SubdividingMenu();

    void setEntries(const std::set<QString> &entries);
    void entriesAdded();

    // Action names and strings passed to addAction and addMenu must
    // appear in the set previously given to setEntries.  If you want
    // to use a different string, use the two-argument method and pass
    // the entry string (used to determine which submenu the action
    // ends up on) as the first argument.

    virtual void addAction(QAction *);
    virtual QAction *addAction(const QString &);
    virtual void addAction(const QString &entry, QAction *);
    
    virtual void addMenu(QMenu *);
    virtual QMenu *addMenu(const QString &);
    virtual void addMenu(const QString &entry, QMenu *);

protected:
    std::map<QString, QMenu *> m_nameToChunkMenuMap;

    int m_lowerLimit;
    int m_upperLimit;

    bool m_entriesSet;
    std::map<QString, QObject *> m_pendingEntries;
};

#endif

