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

#include "KeyReference.h"

#include <QAction>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QDesktopWidget>

KeyReference::KeyReference() :
    m_text(0),
    m_dialog(0)
{
}

KeyReference::~KeyReference()
{
    delete m_dialog;
}

void
KeyReference::setCategory(QString category)
{
    if (m_map.find(category) == m_map.end()) {
        m_categoryOrder.push_back(category);
        m_map[category] = KeyList();
    }
    m_currentCategory = category;
}

void
KeyReference::registerShortcut(QAction *action, QString overrideName)
{
    QString name = action->text();
    if (overrideName != "") name = overrideName;

    QString shortcut = action->shortcut().toString(QKeySequence::NativeText);
    QString tip = action->statusTip();

    registerShortcut(name, shortcut, tip);
}

void
KeyReference::registerShortcut(QString name, QString shortcut, QString tip)
{
    name.replace(tr("&"), "");
            
    KeyList &list = m_map[m_currentCategory];

    for (KeyList::iterator i = list.begin(); i != list.end(); ++i) {
        if (i->actionName == name) {
            i->shortcut = shortcut;
            i->tip = tip;
            i->alternatives.clear();
            return;
        }
    }

    KeyDetails details;
    details.actionName = name;
    details.shortcut = shortcut;
    details.tip = tip;

    list.push_back(details);
}

void
KeyReference::registerAlternativeShortcut(QAction *action, QString alternative)
{
    QString name = action->text();
    registerAlternativeShortcut(name, alternative);
}

void
KeyReference::registerAlternativeShortcut(QAction *action, QKeySequence shortcut)
{
    QString name = action->text();
    registerAlternativeShortcut(name, shortcut.toString(QKeySequence::NativeText));
}

void
KeyReference::registerAlternativeShortcut(QString name, QString alternative)
{
    name.replace(tr("&"), "");

    KeyList &list = m_map[m_currentCategory];

    for (KeyList::iterator i = list.begin(); i != list.end(); ++i) {
        if (i->actionName == name) {
            i->alternatives.push_back(alternative);
            return;
        }
    }
}

void
KeyReference::registerAlternativeShortcut(QString name, QKeySequence shortcut)
{
    registerAlternativeShortcut(name, shortcut.toString(QKeySequence::NativeText));
}

void
KeyReference::show()
{
    if (m_dialog) {
        m_dialog->show();
        m_dialog->raise();
        return;
    }

    QString text;
    
    QColor bgcolor = QApplication::palette().window().color();
    bool darkbg = (bgcolor.red() + bgcolor.green() + bgcolor.blue() < 384);

    text += QString("<center><table bgcolor=\"%1\">")
        .arg(darkbg ? "#121212" : "#e8e8e8");
        
    for (CategoryList::iterator i = m_categoryOrder.begin();
         i != m_categoryOrder.end(); ++i) {

        QString category = *i;
        KeyList &list = m_map[category];

        text += QString("<tr><td bgcolor=\"%1\" colspan=3 align=\"center\"><br><b>%2</b><br></td></tr>\n").arg(darkbg ? "#303030" : "#d0d0d0").arg(category);

        for (KeyList::iterator j = list.begin(); j != list.end(); ++j) {

            QString actionName = j->actionName;

            QString shortcut = j->shortcut;
            shortcut.replace(" ", "&nbsp;");

            QString tip = j->tip;
            if (tip != "") tip = QString("<i>%1</i>").arg(tip);

            QString altdesc;
            if (!j->alternatives.empty()) {
                for (std::vector<QString>::iterator k = j->alternatives.begin();
                     k != j->alternatives.end(); ++k) {
                    QString alt = *k;
                    alt.replace(" ", "&nbsp;");
                    altdesc += tr("<i>or</i>&nbsp;<b>%1</b>").arg(alt);
                }
                altdesc = tr("</b>&nbsp;(%1)<b>").arg(altdesc);
            }

            text += QString("<tr><td width=\"12%\">&nbsp;<b>%1%2</b></td><td>&nbsp;%3</td><td>%4</td></tr>\n")
                .arg(shortcut).arg(altdesc).arg(actionName).arg(tip);
        }
    }

    text += "</table></center>\n";

    m_text = new QTextEdit;
    m_text->setHtml(text);
    m_text->setReadOnly(true);

    m_dialog = new QDialog;
    m_dialog->setWindowTitle(tr("%1: Key and Mouse Reference")
                             .arg(QApplication::applicationName()));

    QVBoxLayout *layout = new QVBoxLayout;
    m_dialog->setLayout(layout);
    layout->addWidget(m_text);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, SIGNAL(clicked(QAbstractButton *)), this, SLOT(dialogButtonClicked(QAbstractButton *)));
    layout->addWidget(bb);

    m_dialog->show();
    
    QDesktopWidget *desktop = QApplication::desktop();
    QRect available = desktop->availableGeometry();

    int width = available.width() * 3 / 5;
    int height = available.height() * 2 / 3;
    if (height < 450) {
        if (available.height() > 500) height = 450;
    }
    if (width < 600) {
        if (available.width() > 650) width = 600;
    }

    m_dialog->resize(width, height);
    m_dialog->raise();
}

void
KeyReference::dialogButtonClicked(QAbstractButton *)
{
    // only button is Close
    m_dialog->hide();
}

void
KeyReference::hide()
{
    if (m_dialog) {
        m_dialog->hide();
    }
}
