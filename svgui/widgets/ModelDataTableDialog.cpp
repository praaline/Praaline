/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ModelDataTableDialog.h"

#include "data/model/ModelDataTableModel.h"
#include "data/model/TabularModel.h"
#include "data/model/Model.h"

#include "CommandHistory.h"
#include "IconLoader.h"

#include <QTableView>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QToolBar>

#include <iostream>

ModelDataTableDialog::ModelDataTableDialog(TabularModel *model,
                                           QString title, QWidget *parent) :
    QMainWindow(parent),
    m_currentRow(0),
    m_trackPlayback(true)
{
    setWindowTitle(tr("Data Editor"));

    QToolBar *toolbar;

    toolbar = addToolBar(tr("Playback Toolbar"));
    m_playToolbar = toolbar;
    toolbar = addToolBar(tr("Play Mode Toolbar"));
    
    IconLoader il;

    QAction *action = new QAction(il.load("playfollow"), tr("Track Playback"), this);
    action->setStatusTip(tr("Toggle tracking of playback position"));
    action->setCheckable(true);
    action->setChecked(m_trackPlayback);
    connect(action, SIGNAL(triggered()), this, SLOT(togglePlayTracking()));
    toolbar->addAction(action);

    toolbar = addToolBar(tr("Edit Toolbar"));

    action = new QAction(il.load("datainsert"), tr("Insert New Item"), this);
    action->setShortcut(tr("Insert"));
    action->setStatusTip(tr("Insert a new item"));
    connect(action, SIGNAL(triggered()), this, SLOT(insertRow()));
    toolbar->addAction(action);

    action = new QAction(il.load("datadelete"), tr("Delete Selected Items"), this);
    action->setShortcut(tr("Delete"));
    action->setStatusTip(tr("Delete the selected item or items"));
    connect(action, SIGNAL(triggered()), this, SLOT(deleteRows()));
    toolbar->addAction(action);

    CommandHistory::getInstance()->registerToolbar(toolbar);

/*
    action = new QAction(il.load("dataedit"), tr("Edit Selected Item"), this);
    action->setShortcut(tr("Edit"));
    action->setStatusTip(tr("Edit the selected item"));
    connect(action, SIGNAL(triggered()), this, SLOT(editRow()));
    toolbar->addAction(action);
*/

    QFrame *mainFrame = new QFrame;
    setCentralWidget(mainFrame);

    QGridLayout *grid = new QGridLayout;
    mainFrame->setLayout(grid);
    
    QGroupBox *box = new QGroupBox;
    if (title != "") {
        box->setTitle(title);
    } else {
        box->setTitle(tr("Data in Layer"));
    }
    grid->addWidget(box, 0, 0);
    grid->setRowStretch(0, 15);

    QGridLayout *subgrid = new QGridLayout;
    box->setLayout(subgrid);

    subgrid->setSpacing(0);
    subgrid->setMargin(5);

    subgrid->addWidget(new QLabel(tr("Find:")), 1, 0);
    subgrid->addWidget(new QLabel(tr(" ")), 1, 1);
    m_find = new QLineEdit;
    subgrid->addWidget(m_find, 1, 2);
    connect(m_find, SIGNAL(textChanged(const QString &)),
            this, SLOT(searchTextChanged(const QString &)));
    connect(m_find, SIGNAL(returnPressed()),
            this, SLOT(searchRepeated()));

    m_tableView = new QTableView;
    subgrid->addWidget(m_tableView, 0, 0, 1, 3);

    m_tableView->setSortingEnabled(true);
    m_tableView->sortByColumn(0, Qt::AscendingOrder);

    m_table = new ModelDataTableModel(model);
    m_tableView->setModel(m_table);

    m_tableView->horizontalHeader()->setStretchLastSection(true);

    connect(m_tableView, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(viewClicked(const QModelIndex &)));
    connect(m_tableView, SIGNAL(pressed(const QModelIndex &)),
            this, SLOT(viewPressed(const QModelIndex &)));
    connect(m_tableView->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
    connect(m_table, SIGNAL(addCommand(Command *)),
            this, SLOT(addCommand(Command *)));
    connect(m_table, SIGNAL(currentChanged(const QModelIndex &)),
            this, SLOT(currentChangedThroughResort(const QModelIndex &)));
    connect(m_table, SIGNAL(modelRemoved()),
            this, SLOT(modelRemoved()));

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, SIGNAL(rejected()), this, SLOT(close()));
    grid->addWidget(bb, 2, 0);
    grid->setRowStretch(2, 0);
    
    QDesktopWidget *desktop = QApplication::desktop();
    QRect available = desktop->availableGeometry();

    int width = available.width() / 3;
    int height = available.height() / 2;
    if (height < 370) {
        if (available.height() > 500) height = 370;
    }
    if (width < 650) {
        if (available.width() > 750) width = 650;
        else if (width < 500) {
            if (available.width() > 650) width = 500;
        }
    }

    resize(width, height);
}

ModelDataTableDialog::~ModelDataTableDialog()
{
    delete m_table;
}

void
ModelDataTableDialog::userScrolledToFrame(sv_frame_t frame)
{
    QModelIndex index = m_table->getModelIndexForFrame(frame);
    makeCurrent(index.row());
}

void
ModelDataTableDialog::playbackScrolledToFrame(sv_frame_t frame)
{
    if (m_trackPlayback) {
        QModelIndex index = m_table->getModelIndexForFrame(frame);
        makeCurrent(index.row());
    }
}

void
ModelDataTableDialog::searchTextChanged(const QString &text)
{
    QModelIndex mi = m_table->findText(text);
    if (mi.isValid()) {
        makeCurrent(mi.row());
        m_tableView->selectionModel()->setCurrentIndex
            (mi, QItemSelectionModel::ClearAndSelect);
    }
}

void
ModelDataTableDialog::searchRepeated()
{
    QModelIndex mi = m_table->findText(m_find->text());
    if (mi.isValid()) {
        makeCurrent(mi.row());
        m_tableView->selectionModel()->setCurrentIndex
            (mi, QItemSelectionModel::ClearAndSelect);
    }
}

void
ModelDataTableDialog::makeCurrent(int row)
{
    int rh = m_tableView->height() / m_tableView->rowHeight(0);
    int topRow = row - rh/4;
    if (topRow < 0) topRow = 0;
    
    // should only scroll if the desired row is not currently visible

    // should only select if no part of the desired row is currently selected

//    cerr << "rh = " << rh << ", row = " << row << ", scrolling to "
//              << topRow << endl;

    int pos = m_tableView->rowViewportPosition(row);

    if (pos < 0 || pos >= m_tableView->height() - rh) {
        m_tableView->scrollTo(m_table->index(topRow, 0));
    }

    bool haveRowSelected = false;
    for (int i = 0; i < m_table->columnCount(); ++i) {
        if (m_tableView->selectionModel()->isSelected(m_table->index(row, i))) {
            haveRowSelected = true;
            break;
        }
    }

    if (!haveRowSelected) {
        m_tableView->selectionModel()->setCurrentIndex
            (m_table->index(row, 0),
             QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void
ModelDataTableDialog::viewClicked(const QModelIndex &index)
{
//    cerr << "ModelDataTableDialog::viewClicked: " << index.row() << ", " << index.column() << endl;
    emit scrollToFrame(m_table->getFrameForModelIndex(index));
}

void
ModelDataTableDialog::viewPressed(const QModelIndex &)
{
//    cerr << "ModelDataTableDialog::viewPressed: " << index.row() << ", " << index.column() << endl;
}

void
ModelDataTableDialog::currentChanged(const QModelIndex &current,
                                     const QModelIndex &)
{
//    cerr << "ModelDataTableDialog::currentChanged: from "
//              << previous.row() << ", " << previous.column()
//              << " to " << current.row() << ", " << current.column() 
//              << endl;
    m_currentRow = current.row();
    m_table->setCurrentRow(m_currentRow);
}

void
ModelDataTableDialog::insertRow()
{
    m_table->insertRow(m_currentRow);
}

void
ModelDataTableDialog::deleteRows()
{
    // not efficient
    while (m_tableView->selectionModel()->hasSelection()) {
        m_table->removeRow
            (m_tableView->selectionModel()->selection().indexes().begin()->row());
    }
}

void
ModelDataTableDialog::editRow()
{
}

void
ModelDataTableDialog::addCommand(SVCommand *command)
{
    CommandHistory::getInstance()->addCommand(command, false, true);
}

void
ModelDataTableDialog::togglePlayTracking()
{
    m_trackPlayback = !m_trackPlayback;
}

void
ModelDataTableDialog::currentChangedThroughResort(const QModelIndex &index)
{
//    cerr << "ModelDataTableDialog::currentChangedThroughResort: row = " << index.row() << endl;
//  m_tableView->scrollTo(index);
    makeCurrent(index.row());
}

void
ModelDataTableDialog::modelRemoved()
{
    close();
}

    
