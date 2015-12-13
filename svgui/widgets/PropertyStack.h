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

#ifndef _PROPERTY_STACK_H_
#define _PROPERTY_STACK_H_

#include <QFrame>
#include <QTabWidget>
#include <vector>

class Layer;
class View;
class PropertyBox;
class PropertyContainer;

class PropertyStack : public QTabWidget
{
    Q_OBJECT

public:
    PropertyStack(QWidget *parent, View *client);
    virtual ~PropertyStack();

    View *getClient() { return m_client; }
    bool containsContainer(PropertyContainer *container) const;
    int getContainerIndex(PropertyContainer *container) const;

signals:
    void viewSelected(View *client);
    void propertyContainerSelected(View *client, PropertyContainer *container);
    void contextHelpChanged(const QString &);

public slots:
    void propertyContainerAdded(PropertyContainer *);
    void propertyContainerRemoved(PropertyContainer *);
    void propertyContainerPropertyChanged(PropertyContainer *);
    void propertyContainerPropertyRangeChanged(PropertyContainer *);
    void propertyContainerNameChanged(PropertyContainer *);

    void showLayer(bool);

    void mouseEnteredTabBar();
    void mouseLeftTabBar();
    void activeTabClicked();

protected slots:
    void selectedContainerChanged(int);

protected:
    View *m_client;
    std::vector<PropertyBox *> m_boxes;

    void repopulate();
    void updateValues(PropertyContainer *);
};

#endif
