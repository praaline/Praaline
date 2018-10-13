/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Praaline
    A research tool for spoken language corpora.

    This file (Network Permission Tester) copyright:
    Sonic Visualiser. An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "NetworkPermissionTester.h"

#include "Version.h"

#include <QWidget>
#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>

bool NetworkPermissionTester::havePermission()
{
    QSettings settings;
    settings.beginGroup("Preferences");

    QString tag = QString("network-permission-%1").arg(PRAALINE_VERSION);

    bool permish = false;

    if (settings.contains(tag)) {
    // if (false) { // DEBUG
        permish = settings.value(tag, false).toBool();
    }
    else {
        QDialog d;
        d.setWindowTitle(QCoreApplication::translate("NetworkPermissionTester", "Welcome to Praaline"));

        QGridLayout *layout = new QGridLayout;
        d.setLayout(layout);

        QLabel *label = new QLabel;
        label->setWordWrap(true);
        label->setText(QCoreApplication::translate
         ("NetworkPermissionTester",
          "<h2>Welcome to Praaline!</h2>"
          "<p>Praaline is a program for managing, annotating, visualising and analysing spoken language corpora. "
          "It provided free as open source software under the GNU General Public License.</p>"
          "<p>The visualisation module of Praaline is based on Sonic Visualiser, a program for audio analysis developed in the Centre "
          "for Digital Music at Queen Mary, University of London, and also provided under the GPL license.</p>"
          "<p><b>Before we go on...</b></p>"
          "<p>Praaline would like to make networking connections and open a network port.</p>"
          "<p>This is to:</p>"
          "<ul><li> Find information about available and installed plugins (Praaline and VAMP);</li>"
          "<li> Support the use of Open Sound Control, where configured; and</li>"
          "<li> Tell you when updates are available.</li>"
              "</ul>"
          "<p>No personal information will be sent, no tracking is carried out, and all requests happen in the background without interrupting your work.</p>"
          "<p>We recommend that you allow this, because it makes Praaline more useful. But if you do not wish to do so, please un-check the box below.<br></p>"));
        layout->addWidget(label, 0, 0);

        QCheckBox *cb = new QCheckBox(QCoreApplication::translate("NetworkPermissionTester", "Allow this"));
        cb->setChecked(true);
        layout->addWidget(cb, 1, 0);

        QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok);
        QObject::connect(bb, SIGNAL(accepted()), &d, SLOT(accept()));
        layout->addWidget(bb, 2, 0);

        d.exec();

        permish = cb->isChecked();
        settings.setValue(tag, permish);
    }

    settings.endGroup();

    return permish;
}
