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

#include "SubdividingMenu.h"

#include <iostream>

#include "base/Debug.h"

using std::set;
using std::map;

SubdividingMenu::SubdividingMenu(int lowerLimit, int upperLimit,
                                 QWidget *parent) :
    QMenu(parent),
    m_lowerLimit(lowerLimit ? lowerLimit : 14),
    m_upperLimit(upperLimit ? upperLimit : (m_lowerLimit * 5) / 2),
    m_entriesSet(false)
{
}

SubdividingMenu::SubdividingMenu(const QString &title, int lowerLimit,
                                 int upperLimit, QWidget *parent) :
    QMenu(title, parent),
    m_lowerLimit(lowerLimit ? lowerLimit : 14),
    m_upperLimit(upperLimit ? upperLimit : (m_lowerLimit * 5) / 2),
    m_entriesSet(false)
{
}

SubdividingMenu::~SubdividingMenu()
{
    for (map<QString, QObject *>::iterator i = m_pendingEntries.begin();
         i != m_pendingEntries.end(); ++i) {
        delete i->second;
    }
}

void
SubdividingMenu::setEntries(const std::set<QString> &entries)
{
    m_entriesSet = true;

    int total = int(entries.size());
        
    if (total < m_upperLimit) return;

    int count = 0;
    QMenu *chunkMenu = new QMenu();
    chunkMenu->setTearOffEnabled(isTearOffEnabled());

    QString firstNameInChunk;
    QChar firstInitialInChunk;
    bool discriminateStartInitial = false;

    for (set<QString>::const_iterator j = entries.begin();
         j != entries.end();
         ++j) {

//        cerr << "SubdividingMenu::setEntries: j -> " << j->toStdString() << endl;

        m_nameToChunkMenuMap[*j] = chunkMenu;

        set<QString>::iterator k = j;
        ++k;

        QChar initial = (*j)[0];

        if (count == 0) {
            firstNameInChunk = *j;
            firstInitialInChunk = initial;
        }

//        cerr << "count = "<< count << ", upper limit = " << m_upperLimit << endl;

        bool lastInChunk = (k == entries.end() ||
                            (count >= m_lowerLimit-1 &&
                             (count == m_upperLimit ||
                              (*k)[0] != initial)));

        ++count;

        if (lastInChunk) {

            bool discriminateEndInitial = (k != entries.end() &&
                                           (*k)[0] == initial);

            bool initialsEqual = (firstInitialInChunk == initial);

            QString from = QString("%1").arg(firstInitialInChunk);
            if (discriminateStartInitial ||
                (discriminateEndInitial && initialsEqual)) {
                from = firstNameInChunk.left(3);
            }

            QString to = QString("%1").arg(initial);
            if (discriminateEndInitial ||
                (discriminateStartInitial && initialsEqual)) {
                to = j->left(3);
            }

            QString menuText;
            
            if (from == to) menuText = from;
            else menuText = tr("%1 - %2").arg(from).arg(to);
            
            discriminateStartInitial = discriminateEndInitial;

            chunkMenu->setTitle(menuText);
                
            QMenu::addMenu(chunkMenu);
            
            chunkMenu = new QMenu();
            chunkMenu->setTearOffEnabled(isTearOffEnabled());
            
            count = 0;
        }
    }
    
    if (count == 0) delete chunkMenu;
}

void
SubdividingMenu::entriesAdded()
{
    if (m_entriesSet) {
        cerr << "ERROR: SubdividingMenu::entriesAdded: setEntries was also called -- should use one mechanism or the other, but not both" << endl;
        return;
    }
    
    set<QString> entries;
    for (map<QString, QObject *>::const_iterator i = m_pendingEntries.begin();
         i != m_pendingEntries.end(); ++i) {
        entries.insert(i->first);
    }
    
    setEntries(entries);

    for (map<QString, QObject *>::iterator i = m_pendingEntries.begin();
         i != m_pendingEntries.end(); ++i) {

        QMenu *menu = dynamic_cast<QMenu *>(i->second);
        if (menu) {
            addMenu(i->first, menu);
            continue;
        }

        QAction *action = dynamic_cast<QAction *>(i->second);
        if (action) {
            addAction(i->first, action);
            continue;
        }
    }

    m_pendingEntries.clear();
}

void
SubdividingMenu::addAction(QAction *action)
{
    QString name = action->text();

    if (!m_entriesSet) {
        m_pendingEntries[name] = action;
        return;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addAction(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        QMenu::addAction(action);
        return;
    }

//    cerr << "SubdividingMenu::addAction(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    m_nameToChunkMenuMap[name]->addAction(action);
}

QAction *
SubdividingMenu::addAction(const QString &name)
{
    if (!m_entriesSet) {
        QAction *action = new QAction(name, this);
        m_pendingEntries[name] = action;
        return action;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addAction(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        return QMenu::addAction(name);
    }

//    cerr << "SubdividingMenu::addAction(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    return m_nameToChunkMenuMap[name]->addAction(name);
}

void
SubdividingMenu::addAction(const QString &name, QAction *action)
{
    if (!m_entriesSet) {
        m_pendingEntries[name] = action;
        return;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addAction(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        QMenu::addAction(action);
        return;
    }

//    cerr << "SubdividingMenu::addAction(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    m_nameToChunkMenuMap[name]->addAction(action);
}

void
SubdividingMenu::addMenu(QMenu *menu)
{
    QString name = menu->title();

    if (!m_entriesSet) {
        m_pendingEntries[name] = menu;
        return;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addMenu(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        QMenu::addMenu(menu);
        return;
    }

//    cerr << "SubdividingMenu::addMenu(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    m_nameToChunkMenuMap[name]->addMenu(menu);
}

QMenu *
SubdividingMenu::addMenu(const QString &name)
{
    if (!m_entriesSet) {
        QMenu *menu = new QMenu(name, this);
        menu->setTearOffEnabled(isTearOffEnabled());
        m_pendingEntries[name] = menu;
        return menu;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addMenu(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        return QMenu::addMenu(name);
    }

//    cerr << "SubdividingMenu::addMenu(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    return m_nameToChunkMenuMap[name]->addMenu(name);
}

void
SubdividingMenu::addMenu(const QString &name, QMenu *menu)
{
    if (!m_entriesSet) {
        m_pendingEntries[name] = menu;
        return;
    }

    if (m_nameToChunkMenuMap.find(name) == m_nameToChunkMenuMap.end()) {
//        cerr << "SubdividingMenu::addMenu(" << name << "): not found in name-to-chunk map, adding to main menu" << endl;
        QMenu::addMenu(menu);
        return;
    }

//    cerr << "SubdividingMenu::addMenu(" << name << "): found in name-to-chunk map for menu " << m_nameToChunkMenuMap[name]->title() << endl;
    m_nameToChunkMenuMap[name]->addMenu(menu);
}

