/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007-2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ProgressDialog.h"

#include <QProgressDialog>
#include <QApplication>
#include <QTimer>

ProgressDialog::ProgressDialog(QString message, bool cancellable,
                               int timeBeforeShow, QWidget *parent) : 
    m_showTimer(0),
    m_timerElapsed(false),
    m_cancelled(false)
{
    m_dialog = new QProgressDialog(message, cancellable ? tr("Cancel") : 0,
                                   0, 100, parent);
    if (timeBeforeShow > 0) {
        m_dialog->hide();
        m_showTimer = new QTimer;
        connect(m_showTimer, SIGNAL(timeout()), this, SLOT(showTimerElapsed()));
        m_showTimer->setSingleShot(true);
        m_showTimer->start(timeBeforeShow);
    } else {
        m_dialog->show();
        m_dialog->raise();
        m_timerElapsed = true;
    }

    if (cancellable) {
        connect(m_dialog, SIGNAL(canceled()), this, SLOT(canceled()));
    }
}

ProgressDialog::~ProgressDialog()
{
    delete m_showTimer;
    delete m_dialog;
}

bool
ProgressDialog::isDefinite() const
{
    return (m_dialog->maximum() > 0);
}

void
ProgressDialog::setDefinite(bool definite)
{
    if (definite) m_dialog->setMaximum(100);
    else m_dialog->setMaximum(0);
}

void
ProgressDialog::setMessage(QString text)
{
    m_dialog->setLabelText(text);
}

void
ProgressDialog::canceled()
{
    m_cancelled = true;
    emit cancelled();
}

bool
ProgressDialog::wasCancelled() const
{
    return m_cancelled;
}

void
ProgressDialog::showTimerElapsed()
{
    m_timerElapsed = true;
    if (m_dialog->value() > 0) {
        emit showing();
        m_dialog->show();
    }
    qApp->processEvents();
}

void
ProgressDialog::setProgress(int percentage)
{
    if (percentage > m_dialog->value()) {

        m_dialog->setValue(percentage);

        if (percentage >= 100 && isDefinite()) {
            m_dialog->hide();
        } else if (m_timerElapsed && !m_dialog->isVisible()) {
            emit showing();
            m_dialog->show();
            m_dialog->raise();
        }

        qApp->processEvents();
    }
}

