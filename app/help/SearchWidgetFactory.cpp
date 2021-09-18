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

#include "SearchWidgetFactory.h"
#include "HelpMode.h"

#include <QVBoxLayout>
#include <QtHelp>

SearchWidgetFactory::SearchWidgetFactory(QHelpSearchEngine* help_search_engine, QObject *parent)
    : QObject(parent), d_help_search_engine(help_search_engine)
{
}

QWidget* SearchWidgetFactory::produceWidget() {
    if (!d_combined_widget && d_help_search_engine) {
        d_combined_widget = new QWidget();
        if (d_combined_widget->layout())
            delete d_combined_widget->layout();

        QVBoxLayout* layout = new QVBoxLayout(d_combined_widget);
        QHelpSearchQueryWidget* queryWidget = d_help_search_engine->queryWidget();
        connect(queryWidget,&QHelpSearchQueryWidget::search,this, &SearchWidgetFactory::handleSearchSignal);
        layout->addWidget(queryWidget);
        QHelpSearchResultWidget* result_widget = d_help_search_engine->resultWidget();
        layout->addWidget(result_widget);
        layout->setMargin(0);

        emit newWidgetCreated(result_widget);
    }

    return d_combined_widget;
}

void SearchWidgetFactory::handleSearchSignal() {
    QHelpSearchQueryWidget* queryWidget = qobject_cast<QHelpSearchQueryWidget*> (sender());
    if (queryWidget) {
        d_help_search_engine->search(queryWidget->query());
    }
}

QString SearchWidgetFactory::widgetLabel() const {
    return tr("Search");
}

QList<int> SearchWidgetFactory::startupModes() const {
    QList<int> modes;
    modes << MODE_HELP_ID;
    return modes;
}

Qtilities::CoreGui::Interfaces::IActionProvider* SearchWidgetFactory::actionProvider() const {
    return 0;
}

QList<int> SearchWidgetFactory::destinationModes() const {
    QList<int> modes;
    modes << MODE_HELP_ID;
    return modes;
}

bool SearchWidgetFactory::isExclusive() const {
    return true;
}

