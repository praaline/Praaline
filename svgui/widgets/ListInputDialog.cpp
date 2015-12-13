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

#include "ListInputDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>

ListInputDialog::ListInputDialog(QWidget *parent, const QString &title,
				 const QString &labelText, const QStringList &list,
				 int current) :
    QDialog(parent),
    m_strings(list)
{
    setWindowTitle(title);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    
    QLabel *label = new QLabel(labelText, this);
    vbox->addWidget(label);
    vbox->addStretch(1);

    int count = 0;
    for (QStringList::const_iterator i = list.begin(); i != list.end(); ++i) {
        QRadioButton *radio = new QRadioButton(*i);
        if (current == count++) radio->setChecked(true);
        m_radioButtons.push_back(radio);
        vbox->addWidget(radio);
    }

    vbox->addStretch(1);

    m_footnote = new QLabel;
    vbox->addWidget(m_footnote);
    m_footnote->hide();
    
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    vbox->addWidget(bb);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

ListInputDialog::~ListInputDialog()
{
}

QString
ListInputDialog::getCurrentString() const
{
    for (size_t i = 0; i < m_radioButtons.size(); ++i) {
        if (m_radioButtons[i]->isChecked()) {
            return m_strings[int(i)];
        }
    }
    return "";
}

void
ListInputDialog::setItemAvailability(int item, bool available)
{
    m_radioButtons[item]->setEnabled(available);
}

void
ListInputDialog::setFootnote(QString footnote)
{
    m_footnote->setText(footnote);
    m_footnote->show();
}

QString
ListInputDialog::getItem(QWidget *parent, const QString &title,
                         const QString &label, const QStringList &list,
                         int current, bool *ok)
{
    ListInputDialog dialog(parent, title, label, list, current);
    
    bool accepted = (dialog.exec() == QDialog::Accepted);
    if (ok) *ok = accepted;

    return dialog.getCurrentString();
}

