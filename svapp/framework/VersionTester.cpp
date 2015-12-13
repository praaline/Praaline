/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2009 Chris Cannam.
*/

#include "VersionTester.h"
#include "base/Debug.h"

#include <iostream>

#include <QNetworkAccessManager>


VersionTester::VersionTester(QString hostname, QString versionFilePath,
			     QString myVersion) :
    m_myVersion(myVersion),
    m_reply(0),
    m_httpFailed(false),
    m_nm(new QNetworkAccessManager)
{
    QUrl url(QString("http://%1/%2").arg(hostname).arg(versionFilePath));
    cerr << "VersionTester: URL is " << url << endl;
    m_reply = m_nm->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(finished()));
}

VersionTester::~VersionTester()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
    delete m_nm;
}

bool
VersionTester::isVersionNewerThan(QString a, QString b)
{
    QRegExp re("[._-]");
    QStringList alist = a.split(re, QString::SkipEmptyParts);
    QStringList blist = b.split(re, QString::SkipEmptyParts);
    int ae = alist.size();
    int be = blist.size();
    int e = std::max(ae, be);
    for (int i = 0; i < e; ++i) {
        int an = 0, bn = 0;
        if (i < ae) {
            an = alist[i].toInt();
            if (an == 0 && alist[i] != "0") {
                an = -1; // non-numeric field -> "-pre1" etc
            }
        }
        if (i < be) {
            bn = blist[i].toInt();
            if (bn == 0 && blist[i] != "0") {
                bn = -1;
            }
        }
        if (an < bn) return false;
        if (an > bn) return true;
    }
    return false;
}

void
VersionTester::error(QNetworkReply::NetworkError)
{
    cerr << "VersionTester: error: " << m_reply->errorString() << endl;
    m_httpFailed = true;
}

void
VersionTester::finished()
{
    QNetworkReply *r = m_reply;
    m_reply = 0;

    r->deleteLater();
    if (m_httpFailed) return;

    int status = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status / 100 != 2) {
        cerr << "VersionTester: error: http status = " << status << endl;
        return;
    }

    QByteArray responseData = r->readAll();
    QString str = QString::fromUtf8(responseData.data());
    QStringList lines = str.split('\n', QString::SkipEmptyParts);
    if (lines.empty()) return;

    QString latestVersion = lines[0];
    cerr << "Comparing current version \"" << m_myVersion << "\" with latest version \"" << latestVersion << "\"" << endl;
    if (isVersionNewerThan(latestVersion, m_myVersion)) {
        cerr << "Latest version \"" << latestVersion << "\" is newer than current version \"" << m_myVersion << "\"" << endl;
        emit newerVersionAvailable(latestVersion);
    }
}


