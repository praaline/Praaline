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

#ifndef _LABEL_COUNTER_INPUT_DIALOG_H_
#define _LABEL_COUNTER_INPUT_DIALOG_H_

#include <QDialog>
#include "data/model/Labeller.h"

class LabelCounterInputDialog : public QDialog
{
    Q_OBJECT

public:
    LabelCounterInputDialog(Labeller *labeller, QWidget *parent);
    virtual ~LabelCounterInputDialog();

protected slots:
    void counterChanged(int);
    void secondCounterChanged(int);
    void cancelClicked();

protected:
    Labeller *m_labeller;
    int m_origCounter;
    int m_origSecondCounter;
};

#endif
