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

#ifndef _KEY_REFERENCE_H_
#define _KEY_REFERENCE_H_

#include <QObject>
#include <QString>
#include <vector>
#include <map>
#include <QKeySequence>

class QAction;
class QTextEdit;
class QDialog;
class QAbstractButton;

class KeyReference : public QObject
{
    Q_OBJECT

public:
    KeyReference();
    virtual ~KeyReference();

    void setCategory(QString category);

    void registerShortcut(QAction *, QString overrideName = "");
    void registerAlternativeShortcut(QAction *, QString alternative);
    void registerAlternativeShortcut(QAction *, QKeySequence alternative);

    void registerShortcut(QString actionName, QString shortcut, QString tipText);
    void registerAlternativeShortcut(QString actionName, QString alternative);
    void registerAlternativeShortcut(QString actionName, QKeySequence alternative);

    void show();
    void hide();

protected slots:
    void dialogButtonClicked(QAbstractButton *);

protected:
    struct KeyDetails {
        QString actionName;
        QString shortcut;
        QString tip;
        std::vector<QString> alternatives;
    };

    typedef std::vector<KeyDetails> KeyList;
    typedef std::map<QString, KeyList> CategoryMap;
    typedef std::vector<QString> CategoryList;
    
    QString m_currentCategory;
    CategoryMap m_map;
    CategoryList m_categoryOrder;

    QTextEdit *m_text;
    QDialog *m_dialog;
};

#endif
