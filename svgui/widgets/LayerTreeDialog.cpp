/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "LayerTreeDialog.h"

#include "LayerTree.h"
#include "view/PaneStack.h"

#include <QTreeView>
#include <QTableView>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QApplication>
#include <QDesktopWidget>

LayerTreeDialog::LayerTreeDialog(PaneStack *stack, QWidget *parent) :
    QDialog(parent),
    m_paneStack(stack)
{
    setWindowTitle(tr("Layer Summary"));

    QGridLayout *grid = new QGridLayout;
    setLayout(grid);
    
    QGroupBox *modelBox = new QGroupBox;
    modelBox->setTitle(tr("Audio Data Sources"));
    grid->addWidget(modelBox, 0, 0);
    grid->setRowStretch(0, 15);

    QGridLayout *subgrid = new QGridLayout;
    modelBox->setLayout(subgrid);

    subgrid->setSpacing(0);
    subgrid->setMargin(5);

    m_modelView = new QTableView;
    subgrid->addWidget(m_modelView);

    m_modelView->verticalHeader()->hide();
#if (QT_VERSION >= 0x050000)
    m_modelView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    m_modelView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    m_modelView->setShowGrid(false);

    m_modelModel = new ModelMetadataModel(m_paneStack, true);
    m_modelView->setModel(m_modelModel);

    QGroupBox *layerBox = new QGroupBox;
    layerBox->setTitle(tr("Panes and Layers"));
    grid->addWidget(layerBox, 1, 0);
    grid->setRowStretch(1, 20);

    subgrid = new QGridLayout;
    layerBox->setLayout(subgrid);

    subgrid->setSpacing(0);
    subgrid->setMargin(5);

    m_layerView = new QTreeView;
#if (QT_VERSION >= 0x050000)
    m_layerView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    m_layerView->header()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    subgrid->addWidget(m_layerView);

    m_layerModel = new LayerTreeModel(m_paneStack);
    m_layerView->setModel(m_layerModel);
    m_layerView->expandAll();

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
    grid->addWidget(bb, 2, 0);
    grid->setRowStretch(2, 0);
    
    QDesktopWidget *desktop = QApplication::desktop();
    QRect available = desktop->availableGeometry();

    int width = available.width() / 2;
    int height = available.height() / 3;
    if (height < 370) {
        if (available.height() > 500) height = 370;
    }
    if (width < 500) {
        if (available.width() > 650) width = 500;
    }

    resize(width, height);
}

LayerTreeDialog::~LayerTreeDialog()
{
    delete m_layerModel;
}

