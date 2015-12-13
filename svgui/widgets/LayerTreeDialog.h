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

#ifndef _LAYER_TREE_DIALOG_H_
#define _LAYER_TREE_DIALOG_H_

#include <QDialog>

class ModelMetadataModel;
class LayerTreeModel;
class PaneStack;
class QTreeView;
class QTableView;

class LayerTreeDialog : public QDialog
{
    Q_OBJECT
    
public:
    LayerTreeDialog(PaneStack *stack, QWidget *parent = 0);
    ~LayerTreeDialog();

protected:
    PaneStack *m_paneStack;
    ModelMetadataModel *m_modelModel;
    QTableView *m_modelView;
    LayerTreeModel *m_layerModel;
    QTreeView *m_layerView;
};

#endif
