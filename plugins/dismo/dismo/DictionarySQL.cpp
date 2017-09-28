/*
    DisMo Annotator
    Copyright (c) 2012-2014 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QString>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "DictionarySQL.h"
#include "DictionaryQuery.h"
using namespace DisMoAnnotator;

DictionarySQL::DictionarySQL(const QString &filename) :
    m_filename(filename)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(filename);
}

DictionarySQL::~DictionarySQL()
{
    m_database.close();
}

bool DictionarySQL::lookup(QHash<QString, QStringList> &table)
{
    bool ret;

    // Nothing to do if the hashtable is empty
    if (table.isEmpty())
        return true;

    // Create a temporary table with the words to search
    if (m_database.isOpen())
        m_database.close();
    if (!m_database.open())
        return false;

    m_database.transaction();
    QSqlQuery query(m_database);
    query.prepare("CREATE TEMP TABLE IF NOT EXISTS Lookup(Form VARCHAR(128))");
    ret = query.exec();
    if (!ret) { m_database.rollback(); m_database.close(); return false; }
    query.prepare("INSERT INTO Lookup (Form) VALUES (:Form)");
    foreach (QString search, table.keys()) {
        query.bindValue(":Form", search);
        ret = query.exec();
        query.bindValue(":Form", search.toLower());
        ret = query.exec();
    }
    m_database.commit();

    query.setForwardOnly(true);
    query.prepare("SELECT l.Form, d.Lemma, d.POS, d.POSExt, d.Disfluency, d.Discourse FROM Lookup l LEFT JOIN Dictionary d ON l.Form=d.Form");
    query.exec();

    QSqlRecord rec = query.record();
    int index_form = rec.indexOf("Form"); if (index_form < 0) return false;
    int index_lemma = rec.indexOf("Lemma"); if (index_lemma < 0) return false;
    int index_pos = rec.indexOf("POS"); if (index_pos < 0) return false;
    int index_posext = rec.indexOf("POSExt"); if (index_posext < 0) return false;
    int index_disfluency = rec.indexOf("Disfluency"); if (index_disfluency < 0) return false;
    int index_discourse = rec.indexOf("Discourse"); if (index_discourse < 0) return false;
    // Read
    while (query.next()) {
        QString form = query.value(index_form).toString();
        QString lemma = query.value(index_lemma).toString();
        QString pos = query.value(index_pos).toString();
        QString posext = query.value(index_posext).toString();
        QString disfluency = query.value(index_disfluency).toString();
        QString discourse = query.value(index_discourse).toString();
        QString entry = lemma + "#" + pos + "#" + posext + "#" + disfluency + "#" + discourse;
        if (pos.isEmpty())
            table.remove(form);
        else
            table[form].append(entry);
    }
    return true;
}
