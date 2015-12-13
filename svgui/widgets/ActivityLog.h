/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2009 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _ACTIVITY_LOG_H_
#define _ACTIVITY_LOG_H_

#include <QDialog>
#include <QString>

class QListView;
class QStringListModel;

class ActivityLog : public QDialog
{
    Q_OBJECT

public:
    ActivityLog();
    ~ActivityLog();

public slots:
    void activityHappened(QString);
    void scrollToEnd();

private:
    QListView *m_listView;
    QStringListModel *m_model;
    QString m_prevName;
};

#endif
