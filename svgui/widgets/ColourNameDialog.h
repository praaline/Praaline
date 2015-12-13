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

#ifndef _COLOUR_NAME_DIALOG_H_
#define _COLOUR_NAME_DIALOG_H_

#include <QDialog>
#include <QColor>
#include <QString>

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;

class ColourNameDialog : public QDialog
{
    Q_OBJECT

public:
    ColourNameDialog(QString title, QString message, QColor colour,
		     QString defaultName,
		     QWidget *parent = 0);

    void showDarkBackgroundCheckbox(QString text);

    QString getColourName() const;
    bool isDarkBackgroundChecked() const;

protected slots:
    void darkBackgroundChanged(int);
    void textChanged(const QString &);

protected:
    QColor m_colour;
    QLabel *m_colourLabel;
    QLineEdit *m_textField;
    QPushButton *m_okButton;
    QCheckBox *m_darkBackground;

    void fillColourLabel();
};

#endif
