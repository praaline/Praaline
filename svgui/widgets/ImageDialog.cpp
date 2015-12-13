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

#include "ImageDialog.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGroupBox>
#include <QDesktopWidget>
#include <QApplication>
#include <QUrl>
#include <QMessageBox>

#include "ProgressDialog.h"

#include "data/fileio/FileSource.h"
#include "InteractiveFileFinder.h"

#include <iostream>

ImageDialog::ImageDialog(QString title,
                         QString image,
                         QString label,
                         QWidget *parent) :
    QDialog(parent),
    m_imagePreview(0),
    m_remoteFile(0)
{
    setWindowTitle(title);
    
    QGridLayout *grid = new QGridLayout;
    setLayout(grid);

    QGroupBox *databox = new QGroupBox(tr("Image"));

    QGridLayout *subgrid = new QGridLayout;
    databox->setLayout(subgrid);

    int row = 0;

    subgrid->addWidget(new QLabel(tr("Label:")), row, 0);

    m_labelEdit = new QLineEdit;
    subgrid->addWidget(m_labelEdit, row, 1, 1, 2);

    ++row;

    subgrid->addWidget(new QLabel(tr("File or URL:")), row, 0);

    m_imageEdit = new QLineEdit;
    subgrid->addWidget(m_imageEdit, row, 1, 1, 1);

    connect(m_imageEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(imageEditEdited(const QString &)));
    connect(m_imageEdit, SIGNAL(editingFinished()),
            this, SLOT(imageEditEdited()));

    QPushButton *browse = new QPushButton(tr("Browse..."));
    connect(browse, SIGNAL(clicked()), this, SLOT(browseClicked()));
    subgrid->addWidget(browse, row, 2, 1, 1);

    ++row;

    QGroupBox *previewbox = new QGroupBox(tr("Preview"));
    
    subgrid = new QGridLayout;
    previewbox->setLayout(subgrid);

    m_imagePreview = new QLabel;
    m_imagePreview->setAlignment(Qt::AlignCenter);
    subgrid->addWidget(m_imagePreview, 0, 0);

    m_imagePreview->setMinimumSize(QSize(100, 100));

    QDesktopWidget *desktop = QApplication::desktop();
    m_imagePreview->setMaximumSize(QSize((desktop->width() * 2) / 3,
                                         (desktop->height() * 2) / 3));

    grid->addWidget(databox, 0, 0);
    grid->addWidget(previewbox, 1, 0);

    grid->setRowStretch(1, 10);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    grid->addWidget(bb, 2, 0, 1, 1);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));

    m_okButton = bb->button(QDialogButtonBox::Ok);
    m_okButton->setEnabled(false);

    if (image != "") setImage(image);
    if (label != "") setLabel(label);
}

ImageDialog::~ImageDialog()
{
    delete m_remoteFile;
}

QString
ImageDialog::getImage()
{
    return m_loadedImageFile;
}

QPixmap
ImageDialog::getPixmap()
{
    return m_loadedImage;
}

QString
ImageDialog::getLabel()
{
    return m_labelEdit->text();
}

void
ImageDialog::setImage(QString image)
{
    m_imageEdit->setText(image);
    updatePreview();
}

void
ImageDialog::setLabel(QString label)
{
    m_labelEdit->setText(label);
}

void
ImageDialog::resizeEvent(QResizeEvent *)
{
    updatePreview();
}

void
ImageDialog::imageEditEdited(const QString &s)
{
    if (s.startsWith("http:") || s.startsWith("ftp:")) {
        return;
    }
    updatePreview();
}

void
ImageDialog::imageEditEdited()
{
    updatePreview();
}

void
ImageDialog::updatePreview()
{
    if (!m_imagePreview) return;

    QString img = m_imageEdit->text();

    m_okButton->setEnabled(img != "");

    if (img != m_loadedImageFile) {

        QString fileName = img;
        delete m_remoteFile;
        m_remoteFile = 0;

        if (FileSource::isRemote(fileName)) {
            QUrl url(fileName);
            if (!FileSource::canHandleScheme(url)) {
                QMessageBox::critical(this, tr("Unsupported scheme in URL"),
                                      tr("The URL scheme \"%1\" is not supported")
                                      .arg(url.scheme()));
            } else {
                
                ProgressDialog dialog(tr("Opening image URL..."), true, 2000);
                m_remoteFile = new FileSource(url, &dialog);
                m_remoteFile->waitForData();
                if (!m_remoteFile->isOK()) {
                    QMessageBox::critical(this, tr("File download failed"),
                                          tr("Failed to download URL \"%1\": %2")
                                          .arg(url.toString()).arg(m_remoteFile->getErrorString()));
                    delete m_remoteFile;
                    m_remoteFile = 0;
                } else {
                    fileName = m_remoteFile->getLocalFilename();
                }
            }
        }
        
//        cerr << "image filename: \"" << fileName << "\"" << endl;

        m_loadedImage = QPixmap(fileName);
        m_loadedImageFile = img;
    }

    QSize sz(m_imagePreview->size());
    int m = m_imagePreview->margin() * 2;
    sz -= QSize(m, m);

    if (m_loadedImage.isNull()) {
        m_imagePreview->setPixmap(QPixmap());
    } else {
        m_imagePreview->setPixmap(m_loadedImage.scaled
                                  (sz,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation));
    }
}

void
ImageDialog::browseClicked()
{
    QString file =
        InteractiveFileFinder::getInstance()->getOpenFileName(FileFinder::ImageFile);

    if (file != "") {
        setImage(file);
        emit imageChanged(file);
    }
}



