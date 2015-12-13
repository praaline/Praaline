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

#include "ColourNameDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QIcon>
#include <QCheckBox>
#include <QPainter>
#include <QPushButton>

ColourNameDialog::ColourNameDialog(QString title, QString message,
				   QColor colour, QString defaultName,
				   QWidget *parent) :
    QDialog(parent),
    m_colour(colour)
{
    setWindowTitle(title);

    QGridLayout *layout = new QGridLayout(this);
    
    QLabel *label = new QLabel(message, this);
    layout->addWidget(label, 0, 0, 1, 2);
    
    m_colourLabel = new QLabel(this);
    layout->addWidget(m_colourLabel, 1, 1);

    m_textField = new QLineEdit(defaultName, this);
    layout->addWidget(m_textField, 1, 0);

    connect(m_textField, SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    
    m_darkBackground = new QCheckBox(this);
    layout->addWidget(m_darkBackground, 2, 0);
    m_darkBackground->setChecked
        (colour.red() + colour.green() + colour.blue() > 384);
    fillColourLabel();

    connect(m_darkBackground, SIGNAL(stateChanged(int)),
            this, SLOT(darkBackgroundChanged(int)));

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    layout->addWidget(bb, 3, 0, 1, 2);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
    m_okButton = bb->button(QDialogButtonBox::Ok);
    m_okButton->setEnabled(defaultName != "");
}

void
ColourNameDialog::showDarkBackgroundCheckbox(QString text)
{
    m_darkBackground->setText(text);
    m_darkBackground->show();
}

bool
ColourNameDialog::isDarkBackgroundChecked() const
{
    return m_darkBackground->isChecked();
}

void
ColourNameDialog::darkBackgroundChanged(int)
{
    fillColourLabel();
}

void
ColourNameDialog::textChanged(const QString &text)
{
    m_okButton->setEnabled(text != "");
}

void
ColourNameDialog::fillColourLabel()
{
    QPixmap pmap(20, 20);
    pmap.fill(m_darkBackground->isChecked() ? Qt::black : Qt::white);
    QPainter paint(&pmap);
    paint.setPen(m_colour);
    paint.setBrush(m_colour);
    paint.drawRect(2, 2, 15, 15);
    m_colourLabel->setPixmap(pmap);
}

QString
ColourNameDialog::getColourName() const
{
    return m_textField->text();
}
