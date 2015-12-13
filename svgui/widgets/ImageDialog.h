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

#ifndef _IMAGE_DIALOG_H_
#define _IMAGE_DIALOG_H_

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;
class FileSource;

class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    ImageDialog(QString title,
                QString image = "",
                QString label = "",
                QWidget *parent = 0);
    virtual ~ImageDialog();

    QString getImage();
    QPixmap getPixmap();
    QString getLabel();

signals:
    void imageChanged(QString image);
    void labelChanged(QString label);

public slots:
    void setImage(QString image);
    void setLabel(QString label);
    void updatePreview();

protected slots:
    void browseClicked();
    void imageEditEdited(const QString &);
    void imageEditEdited();

protected:
    void resizeEvent(QResizeEvent *);

    QLineEdit *m_imageEdit;
    QLineEdit *m_labelEdit;
    QLabel *m_imagePreview;

    QString m_loadedImageFile;
    QPixmap m_loadedImage;

    QPushButton *m_okButton;

    FileSource *m_remoteFile;
};

#endif
