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

#ifndef _PROPERTY_BOX_H_
#define _PROPERTY_BOX_H_

#include "base/PropertyContainer.h"

#include <QFrame>
#include <map>

class QLayout;
class QWidget;
class QGridLayout;
class QVBoxLayout;
class QLabel;
class LEDButton;

class PropertyBox : public QFrame
{
    Q_OBJECT

public:
    PropertyBox(PropertyContainer *);
    ~PropertyBox();

    PropertyContainer *getContainer() { return m_container; }

signals:
    void changePlayGainDial(int);
    void changePlayPanDial(int);
    void showLayer(bool);
    void contextHelpChanged(const QString &);

public slots:
    void propertyContainerPropertyChanged(PropertyContainer *);
    void propertyContainerPropertyRangeChanged(PropertyContainer *);
    void playClipChanged(QString);
    void layerVisibilityChanged(bool);

protected slots:
    void propertyControllerChanged(int);
    void propertyControllerChanged(bool);

    void playAudibleChanged(bool);
    void playAudibleButtonChanged(bool);
    void playGainChanged(float);
    void playGainDialChanged(int);
    void playPanChanged(float);
    void playPanDialChanged(int);

    void populateViewPlayFrame();

    void unitDatabaseChanged();
    void colourDatabaseChanged();

    void editPlayParameters();

    void mouseEnteredWidget();
    void mouseLeftWidget();

protected:
    void updatePropertyEditor(PropertyContainer::PropertyName,
                              bool rangeChanged = false);
    void updateContextHelp(QObject *o);
    void addNewColour();

    QLabel *m_nameWidget;
    QWidget *m_mainWidget;
    QGridLayout *m_layout;
    PropertyContainer *m_container;
    QFrame *m_viewPlayFrame;
    QVBoxLayout *m_mainBox;
    LEDButton *m_showButton;
    LEDButton *m_playButton;
    std::map<QString, QGridLayout *> m_groupLayouts;
    std::map<QString, QWidget *> m_propertyControllers;
};

#endif
