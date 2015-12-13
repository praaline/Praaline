/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _PLUGIN_RDF_INDEXER_H_
#define _PLUGIN_RDF_INDEXER_H_

#include <QString>
#include <QStringList>
#include <QMutex>
#include <map>
#include <set>

namespace Dataquay {
    class BasicStore;
}

class PluginRDFIndexer
{
public:
    static PluginRDFIndexer *getInstance();

    /**
     * Index all URLs obtained from index files defined in the current
     * settings.  In contrast to indexing URLs that are installed
     * locally alongside plugins, this is not done automatically
     * because it may incur significant processing and networking
     * effort.  It could be called from a background thread at
     * startup, for example.
     *
     * Note that this class has a single mutex, so other functions
     * will block if called from a different thread while this one is
     * running.
     */
    bool indexConfiguredURLs();

    bool indexURL(QString url); // in addition to "installed" URLs

    QString getURIForPluginId(QString pluginId);
    QString getIdForPluginURI(QString uri);

    QStringList getIndexedPluginIds();

    const Dataquay::BasicStore *getIndex();

    ~PluginRDFIndexer();

protected:
    PluginRDFIndexer();
    QMutex m_mutex;

    typedef std::map<QString, QString> StringMap;
    StringMap m_uriToIdMap;
    StringMap m_idToUriMap;

    void indexInstalledURLs();

    bool pullFile(QString path);
    bool pullURL(QString urlString);
    bool reindex();

    Dataquay::BasicStore *m_index;

    static PluginRDFIndexer *m_instance;
};

#endif

