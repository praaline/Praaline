/****************************************************************************
**
** Copyright (c) 2009-2013, Jaco Naudé
**
** This file is part of Qtilities.
**
** For licensing information, please see
** http://jpnaude.github.io/Qtilities/page_licensing.html
**
****************************************************************************/

#include "ContentWidgetFactory.h"
#include "HelpMode.h"

#include <QHBoxLayout>
#include <QHelpContentWidget>
#include <QDebug>

ContentWidgetFactory::ContentWidgetFactory(QHelpEngine* help_engine, QObject *parent)
    : QObject(parent), d_help_engine(help_engine) {

}

ContentWidgetFactory::~ContentWidgetFactory() {

}

QWidget* ContentWidgetFactory::produceWidget() {
    if (!d_content_widget && d_help_engine) {
        QWidget* widget = d_help_engine->contentWidget();
        d_content_widget = widget;
        emit newWidgetCreated(widget);
    }

    return d_content_widget;
}

QString ContentWidgetFactory::widgetLabel() const {
    return tr("Contents");
}

QList<int> ContentWidgetFactory::startupModes() const {
    QList<int> modes;
    modes << MODE_HELP_ID;
    return modes;
}

Qtilities::CoreGui::Interfaces::IActionProvider* ContentWidgetFactory::actionProvider() const {
    return 0;
}

QList<int> ContentWidgetFactory::destinationModes() const {
    QList<int> modes;
    modes << MODE_HELP_ID;
    return modes;
}

bool ContentWidgetFactory::isExclusive() const {
    return true;
}

