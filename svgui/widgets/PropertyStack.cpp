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

#include "PropertyStack.h"
#include "PropertyBox.h"
#include "base/PropertyContainer.h"
#include "view/View.h"
#include "layer/Layer.h"
#include "layer/LayerFactory.h"
#include "widgets/NotifyingTabBar.h"
#include "widgets/IconLoader.h"
#include "base/Command.h"
#include "widgets/CommandHistory.h"
#include "layer/ShowLayerCommand.h"

#include <QIcon>
#include <QTabWidget>

#include <iostream>

//#define DEBUG_PROPERTY_STACK 1

PropertyStack::PropertyStack(QWidget *parent, View *client) :
    QTabWidget(parent),
    m_client(client)
{
    NotifyingTabBar *bar = new NotifyingTabBar();
    bar->setDrawBase(false);

    connect(bar, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredTabBar()));
    connect(bar, SIGNAL(mouseLeft()), this, SLOT(mouseLeftTabBar()));
    connect(bar, SIGNAL(activeTabClicked()), this, SLOT(activeTabClicked()));

    setTabBar(bar);

#if (QT_VERSION >= 0x0402)
    setElideMode(Qt::ElideNone); 
    tabBar()->setUsesScrollButtons(true); 
    tabBar()->setIconSize(QSize(16, 16));
#endif
    
    repopulate();

    connect(this, SIGNAL(currentChanged(int)),
	    this, SLOT(selectedContainerChanged(int)));

    connect(m_client, SIGNAL(propertyContainerAdded(PropertyContainer *)),
	    this, SLOT(propertyContainerAdded(PropertyContainer *)));

    connect(m_client, SIGNAL(propertyContainerRemoved(PropertyContainer *)),
	    this, SLOT(propertyContainerRemoved(PropertyContainer *)));

    connect(m_client, SIGNAL(propertyContainerPropertyChanged(PropertyContainer *)),
	    this, SLOT(propertyContainerPropertyChanged(PropertyContainer *)));

    connect(m_client, SIGNAL(propertyContainerPropertyRangeChanged(PropertyContainer *)),
	    this, SLOT(propertyContainerPropertyRangeChanged(PropertyContainer *)));

    connect(m_client, SIGNAL(propertyContainerNameChanged(PropertyContainer *)),
	    this, SLOT(propertyContainerNameChanged(PropertyContainer *)));

    connect(this, SIGNAL(propertyContainerSelected(View *, PropertyContainer *)),
	    m_client, SLOT(propertyContainerSelected(View *, PropertyContainer *)));
}

PropertyStack::~PropertyStack()
{
}

void
PropertyStack::repopulate()
{
    blockSignals(true);

#ifdef DEBUG_PROPERTY_STACK
    cerr << "PropertyStack[" << this << "]::repopulate" << endl;
#endif
    
    while (count() > 0) {
	removeTab(0);
    }
    for (size_t i = 0; i < m_boxes.size(); ++i) {
	delete m_boxes[i];
    }
    m_boxes.clear();
    
    for (int i = 0; i < m_client->getPropertyContainerCount(); ++i) {

	PropertyContainer *container = m_client->getPropertyContainer(i);
	QString name = container->getPropertyContainerName();
	
#ifdef DEBUG_PROPERTY_STACK
        cerr << "PropertyStack[" << this << "]::repopulate: client " << m_client
             << " returns container " << container << " (name " << name
             << ") at position " << i << endl;
#endif

	PropertyBox *box = new PropertyBox(container);

	connect(box, SIGNAL(showLayer(bool)), this, SLOT(showLayer(bool)));
        connect(box, SIGNAL(contextHelpChanged(const QString &)),
                this, SIGNAL(contextHelpChanged(const QString &)));

        Layer *layer = dynamic_cast<Layer *>(container);
        if (layer) {
            box->layerVisibilityChanged(!layer->isLayerDormant(m_client));
        }

        QString shortName = name;

        if (layer) {
            shortName = LayerFactory::getInstance().getLayerPresentationName(LayerFactory::getInstance().getLayerType(layer));
            if (layer->getLayerPresentationName() != "") {
                name = layer->getLayerPresentationName();
            }
        }

        bool nameDiffers = (name != shortName);
        shortName = QString("&%1 %2").arg(i + 1).arg(shortName);

	QString iconName = container->getPropertyContainerIconName();

        QIcon icon(IconLoader().load(iconName));
	if (icon.isNull()) {
	    addTab(box, shortName);
            if (nameDiffers) {
                setTabToolTip(i, name);
            }
	} else {
	    addTab(box, icon, QString("&%1").arg(i + 1));
	    setTabToolTip(i, name);
	}

	m_boxes.push_back(box);
    }    

    blockSignals(false);
}

bool
PropertyStack::containsContainer(PropertyContainer *pc) const
{
    for (int i = 0; i < m_client->getPropertyContainerCount(); ++i) {
	PropertyContainer *container = m_client->getPropertyContainer(i);
	if (pc == container) return true;
    }

    return false;
}

int
PropertyStack::getContainerIndex(PropertyContainer *pc) const
{
    // This is used to obtain an index to be passed to setCurrentIndex
    // -- which is the index of the property container's box in our
    // stack of boxes. That is not the same thing as the index of the
    // container (i.e. the layer) in the view: the view reorders its
    // containers whenever one is raised to the top, while our boxes
    // remain in the same order. So we must find this container in the
    // box list, not in the view.

    for (int i = 0; in_range_for(m_boxes, i); ++i) {
	PropertyContainer *container = m_boxes[i]->getContainer();
	if (pc == container) {
            return i;
        }
    }

    return false;
}

void
PropertyStack::propertyContainerAdded(PropertyContainer *)
{
    if (sender() != m_client) return;
    repopulate();
}

void
PropertyStack::propertyContainerRemoved(PropertyContainer *)
{
    if (sender() != m_client) return;
    repopulate();
}

void
PropertyStack::propertyContainerPropertyChanged(PropertyContainer *pc)
{
    Layer *layer = dynamic_cast<Layer *>(pc);
    for (unsigned int i = 0; i < m_boxes.size(); ++i) {
	if (pc == m_boxes[i]->getContainer()) {
	    m_boxes[i]->propertyContainerPropertyChanged(pc);
            if (layer) {
                m_boxes[i]->layerVisibilityChanged
                    (!layer->isLayerDormant(m_client));
            }
	}
    }
}

void
PropertyStack::propertyContainerPropertyRangeChanged(PropertyContainer *pc)
{
    for (unsigned int i = 0; i < m_boxes.size(); ++i) {
	if (pc == m_boxes[i]->getContainer()) {
	    m_boxes[i]->propertyContainerPropertyRangeChanged(pc);
	}
    }
}

void
PropertyStack::propertyContainerNameChanged(PropertyContainer *)
{
    if (sender() != m_client) return;
    repopulate();
}

void
PropertyStack::showLayer(bool show)
{
    QObject *obj = sender();
    
    for (unsigned int i = 0; i < m_boxes.size(); ++i) {
	if (obj == m_boxes[i]) {
	    Layer *layer = dynamic_cast<Layer *>(m_boxes[i]->getContainer());
	    if (layer) {
                CommandHistory::getInstance()->addCommand
                    (new ShowLayerCommand(m_client, layer, show,
                                          tr("Change Layer Visibility")));
		return;
	    }
	}
    }
}

void
PropertyStack::selectedContainerChanged(int n)
{
    if (n >= int(m_boxes.size())) return;
    emit propertyContainerSelected(m_client, m_boxes[n]->getContainer());
}

void
PropertyStack::mouseEnteredTabBar()
{
    emit contextHelpChanged(tr("Click to change the current active layer"));
}

void
PropertyStack::mouseLeftTabBar()
{
    emit contextHelpChanged("");
}

void
PropertyStack::activeTabClicked()
{
    emit viewSelected(m_client);
}

