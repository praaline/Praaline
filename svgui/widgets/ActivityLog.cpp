/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2009 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ActivityLog.h"

#include <QListView>
#include <QGridLayout>
#include <QStringListModel>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTime>
#include <QApplication>

#include <iostream>

#include "base/Debug.h"

using std::cerr;
using std::endl;

//#define PRINT_ACTIVITY 1

ActivityLog::ActivityLog() : QDialog()
{
    setWindowTitle(tr("Activity Log"));

    QGridLayout *layout = new QGridLayout;
    setLayout(layout);

    layout->addWidget(new QLabel(tr("<p>Activity Log lists your interactions and other events within %1.</p>").arg(QApplication::applicationName())), 0, 0);

    m_listView = new QListView;
    m_model = new QStringListModel;
    m_listView->setModel(m_model);
    layout->addWidget(m_listView, 1, 0);
    layout->setRowStretch(1, 10);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, SIGNAL(rejected()), this, SLOT(hide()));
    layout->addWidget(bb, 2, 0);
}

ActivityLog::~ActivityLog()
{
}

void
ActivityLog::activityHappened(QString name)
{
    name = name.replace("&", "");

#ifdef PRINT_ACTIVITY
    cerr << "ActivityLog: " << name;
    if (name == m_prevName) {
        cerr << " (duplicate)";
    }
    cerr << endl;
#endif

    if (name == m_prevName) {
        return;
    }
    m_prevName = name;
    int row = m_model->rowCount();
    name = tr("%1: %2").arg(QTime::currentTime().toString()).arg(name);
    m_model->insertRows(row, 1);
    QModelIndex ix = m_model->index(row, 0);
    m_model->setData(ix, name);
    if (isVisible()) m_listView->scrollTo(ix);
}

void
ActivityLog::scrollToEnd()
{
    if (m_model->rowCount() == 0 || !isVisible()) return;
    QModelIndex ix = m_model->index(m_model->rowCount()-1, 0);
    m_listView->scrollTo(ix);
}

    
