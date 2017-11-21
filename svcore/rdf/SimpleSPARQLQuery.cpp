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

#include "SimpleSPARQLQuery.h"
#include "base/ProgressReporter.h"
#include "base/Profiler.h"

#include <QMutex>
#include <QMutexLocker>
#include <QRegExp>

#include <set>

// Rather than including <redland.h> -- for some reason redland.h
// includes <rasqal.h>, while the rasqal header actually gets
// installed as <rasqal/rasqal.h> which breaks the inclusion all over
// the place unless a very clever include path is set
#include <rasqal/rasqal.h>
#include <librdf.h>

//#define DEBUG_SIMPLE_SPARQL_QUERY 1

#include <iostream>

using std::cerr;
using std::endl;

class WredlandWorldWrapper
{
public:
    WredlandWorldWrapper();
    ~WredlandWorldWrapper();

    bool isOK() const;

    bool loadUriIntoDefaultModel(QString uriString, QString &errorString);
        
    librdf_world *getWorld() { return m_world; }
    const librdf_world *getWorld() const { return m_world; }
        
    librdf_model *getDefaultModel() { return m_defaultModel; }
    const librdf_model *getDefaultModel() const { return m_defaultModel; }

    librdf_model *getModel(QString fromUri);
    void freeModel(QString forUri);

private:
    QMutex m_mutex;

    librdf_world *m_world;
    librdf_storage *m_defaultStorage;
    librdf_model *m_defaultModel;

    std::set<QString> m_defaultModelUris;

    std::map<QString, librdf_storage *> m_ownStorageUris;
    std::map<QString, librdf_model *> m_ownModelUris;

    bool loadUri(librdf_model *model, QString uri, QString &errorString);
};

WredlandWorldWrapper::WredlandWorldWrapper() :
    m_world(0), m_defaultStorage(0), m_defaultModel(0)
{
    m_world = librdf_new_world();
    if (!m_world) {
        cerr << "SimpleSPARQLQuery: ERROR: Failed to create LIBRDF world!" << endl;
        return;
    }
    librdf_world_open(m_world);

    m_defaultStorage = librdf_new_storage(m_world, "trees", NULL, NULL);
    if (!m_defaultStorage) {
        std::cerr << "SimpleSPARQLQuery: WARNING: Failed to initialise Redland trees datastore, falling back to memory store" << std::endl;
        m_defaultStorage = librdf_new_storage(m_world, NULL, NULL, NULL);
        if (!m_defaultStorage) {
            std::cerr << "SimpleSPARQLQuery: ERROR: Failed to initialise Redland memory datastore" << std::endl;
            return;
        }                
    }
    m_defaultModel = librdf_new_model(m_world, m_defaultStorage, NULL);
    if (!m_defaultModel) {
        std::cerr << "SimpleSPARQLQuery: ERROR: Failed to initialise Redland data model" << std::endl;
        return;
    }
}

WredlandWorldWrapper::~WredlandWorldWrapper()
{
/*!!! This is a static singleton; destroying it while there are
      queries outstanding can be problematic, it appears, and since
      the storage is non-persistent there shouldn't be any need to
      destroy it explicitly, except for the sake of tidiness.

    while (!m_ownModelUris.empty()) {
        librdf_free_model(m_ownModelUris.begin()->second);
        m_ownModelUris.erase(m_ownModelUris.begin());
    }
    while (!m_ownStorageUris.empty()) {
        librdf_free_storage(m_ownStorageUris.begin()->second);
        m_ownStorageUris.erase(m_ownStorageUris.begin());
    }
    if (m_defaultModel) librdf_free_model(m_defaultModel);
    if (m_defaultStorage) librdf_free_storage(m_defaultStorage);
    if (m_world) librdf_free_world(m_world);
*/
}

bool
WredlandWorldWrapper::isOK() const {
    return (m_defaultModel != 0); 
}

bool
WredlandWorldWrapper::loadUriIntoDefaultModel(QString uriString, QString &errorString)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_defaultModelUris.find(uriString) != m_defaultModelUris.end()) {
        return true;
    }
    
    if (loadUri(m_defaultModel, uriString, errorString)) {
        m_defaultModelUris.insert(uriString);
        return true;
    } else {
        return false;
    }
}

librdf_model *
WredlandWorldWrapper::getModel(QString fromUri)
{
    QMutexLocker locker(&m_mutex);
    if (fromUri == "") {
        return getDefaultModel();
    }
    if (m_ownModelUris.find(fromUri) != m_ownModelUris.end()) {
        return m_ownModelUris[fromUri];
    }
    librdf_storage *storage = librdf_new_storage(m_world, "trees", NULL, NULL);
    if (!storage) { // don't warn here, we probably already did it in main ctor
        storage = librdf_new_storage(m_world, NULL, NULL, NULL);
    }
    librdf_model *model = librdf_new_model(m_world, storage, NULL);
    if (!model) {
        std::cerr << "SimpleSPARQLQuery: ERROR: Failed to create new model" << std::endl;
        librdf_free_storage(storage);
        return 0;
    }
    QString err;
    if (!loadUri(model, fromUri, err)) {
        std::cerr << "SimpleSPARQLQuery: ERROR: Failed to parse into new model: " << err << std::endl;
        librdf_free_model(model);
        librdf_free_storage(storage);
        m_ownModelUris[fromUri] = 0;
        return 0;
    }
    m_ownModelUris[fromUri] = model;
    m_ownStorageUris[fromUri] = storage;
    return model;
}

void
WredlandWorldWrapper::freeModel(QString forUri)
{
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    SVDEBUG << "SimpleSPARQLQuery::freeModel: Model uri = \"" << forUri << "\"" << endl;
#endif

    QMutexLocker locker(&m_mutex);
    if (forUri == "") {
        SVDEBUG << "SimpleSPARQLQuery::freeModel: ERROR: Can't free default model" << endl;
        return;
    }
    if (m_ownModelUris.find(forUri) == m_ownModelUris.end()) {
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
        SVDEBUG << "SimpleSPARQLQuery::freeModel: NOTE: Unknown or already-freed model (uri = \"" << forUri << "\")" << endl;
#endif
        return;
    }

    librdf_model *model = m_ownModelUris[forUri];
    if (model) librdf_free_model(model);
    m_ownModelUris.erase(forUri);

    if (m_ownStorageUris.find(forUri) != m_ownStorageUris.end()) {
        librdf_storage *storage = m_ownStorageUris[forUri];
        if (storage) librdf_free_storage(storage);
        m_ownStorageUris.erase(forUri);
    }        
}

bool
WredlandWorldWrapper::loadUri(librdf_model *model, QString uri, QString &errorString)
{
    librdf_uri *luri = librdf_new_uri
        (m_world, (const unsigned char *)uri.toUtf8().data());
    if (!luri) {
        errorString = "Failed to construct librdf_uri!";
        return false;
    }
    
    librdf_parser *parser = librdf_new_parser(m_world, "guess", NULL, NULL);
    if (!parser) {
        errorString = "Failed to initialise Redland parser";
        return false;
    }

#ifdef DEBUG_SIMPLE_SPARQL_QUERY    
    std::cerr << "About to parse \"" << uri << "\"" << std::endl;
#endif
    
    Profiler p("SimpleSPARQLQuery: Parse URI into LIBRDF model");
    
    if (librdf_parser_parse_into_model(parser, luri, NULL, model)) {
        
        errorString = QString("Failed to parse RDF from URI \"%1\"")
            .arg(uri);
        librdf_free_parser(parser);
        return false;
        
    } else {
        
        librdf_free_parser(parser);
        return true;
    }
}        


class SimpleSPARQLQuery::Impl
{
public:
    Impl(SimpleSPARQLQuery::QueryType, QString query);
    ~Impl();

    static bool addSourceToModel(QString sourceUri);
    static void closeSingleSource(QString sourceUri);

    void setProgressReporter(ProgressReporter *reporter) { m_reporter = reporter; }
    bool wasCancelled() const { return m_cancelled; }

    ResultList execute();

    bool isOK() const;
    QString getErrorString() const;

protected:
    static QMutex m_mutex;

    static WredlandWorldWrapper *m_redland;

    ResultList executeDirectParser();
    ResultList executeDatastore();
    ResultList executeFor(QString modelUri);

    QueryType m_type;
    QString m_query;
    QString m_errorString;
    ProgressReporter *m_reporter;
    bool m_cancelled;
};

WredlandWorldWrapper *SimpleSPARQLQuery::Impl::m_redland = 0;

QMutex SimpleSPARQLQuery::Impl::m_mutex;

SimpleSPARQLQuery::SimpleSPARQLQuery(QueryType type, QString query) :
    m_impl(new Impl(type, query))
{
}

SimpleSPARQLQuery::~SimpleSPARQLQuery() 
{
    delete m_impl;
}

void
SimpleSPARQLQuery::setProgressReporter(ProgressReporter *reporter)
{
    m_impl->setProgressReporter(reporter);
}

bool
SimpleSPARQLQuery::wasCancelled() const
{
    return m_impl->wasCancelled();
}

SimpleSPARQLQuery::ResultList
SimpleSPARQLQuery::execute()
{
    return m_impl->execute();
}

bool
SimpleSPARQLQuery::isOK() const
{
    return m_impl->isOK();
}

QString
SimpleSPARQLQuery::getErrorString() const
{
    return m_impl->getErrorString();
}

bool
SimpleSPARQLQuery::addSourceToModel(QString sourceUri)
{
    return SimpleSPARQLQuery::Impl::addSourceToModel(sourceUri);
}

void
SimpleSPARQLQuery::closeSingleSource(QString sourceUri)
{
    SimpleSPARQLQuery::Impl::closeSingleSource(sourceUri);
}

SimpleSPARQLQuery::Impl::Impl(QueryType type, QString query) :
    m_type(type),
    m_query(query),
    m_reporter(0),
    m_cancelled(false)
{
}

SimpleSPARQLQuery::Impl::~Impl()
{
}

bool
SimpleSPARQLQuery::Impl::isOK() const
{
    return (m_errorString == "");
}

QString
SimpleSPARQLQuery::Impl::getErrorString() const
{
    return m_errorString;
}

SimpleSPARQLQuery::ResultList
SimpleSPARQLQuery::Impl::execute()
{
    ResultList list;

    QMutexLocker locker(&m_mutex);

    if (!m_redland) {
        m_redland = new WredlandWorldWrapper();
    }

    if (!m_redland->isOK()) {
        cerr << "ERROR: SimpleSPARQLQuery::execute: Failed to initialise Redland datastore" << endl;
        return list;
    }

    if (m_type == QueryFromSingleSource) {
        return executeDirectParser();
    } else {
        return executeDatastore();
    }

#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    if (m_errorString != "") {
        std::cerr << "SimpleSPARQLQuery::execute: error returned: \""
                  << m_errorString << "\"" << std::endl;
    }
#endif
}

SimpleSPARQLQuery::ResultList
SimpleSPARQLQuery::Impl::executeDirectParser()
{
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    SVDEBUG << "SimpleSPARQLQuery::executeDirectParser: Query is: \"" << m_query << "\"" << endl;
#endif

    ResultList list;

    Profiler profiler("SimpleSPARQLQuery::executeDirectParser");

    static QRegExp fromRE("from\\s+<([^>]+)>", Qt::CaseInsensitive);
    QString fromUri;

    if (fromRE.indexIn(m_query) < 0) {
        SVDEBUG << "SimpleSPARQLQuery::executeDirectParser: Query contains no FROM clause, nothing to parse from" << endl;
        return list;
    } else {
        fromUri = fromRE.cap(1);
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
        SVDEBUG << "SimpleSPARQLQuery::executeDirectParser: FROM URI is <"
                  << fromUri << ">" << endl;
#endif
    }

    return executeFor(fromUri);
}

SimpleSPARQLQuery::ResultList
SimpleSPARQLQuery::Impl::executeDatastore()
{
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    SVDEBUG << "SimpleSPARQLQuery::executeDatastore: Query is: \"" << m_query << "\"" << endl;
#endif

    ResultList list;

    Profiler profiler("SimpleSPARQLQuery::executeDatastore");

    return executeFor("");
}

SimpleSPARQLQuery::ResultList
SimpleSPARQLQuery::Impl::executeFor(QString modelUri)
{
    ResultList list;
    librdf_query *query;

#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    static std::map<QString, int> counter;
    if (counter.find(m_query) == counter.end()) counter[m_query] = 1;
    else ++counter[m_query];
    std::cerr << "Counter for this query: " << counter[m_query] << std::endl;
    std::cerr << "Base URI is: \"" << modelUri << "\"" << std::endl;
#endif

    {
        Profiler p("SimpleSPARQLQuery: Prepare LIBRDF query");
        query = librdf_new_query
            (m_redland->getWorld(), "sparql", NULL,
             (const unsigned char *)m_query.toUtf8().data(), NULL);
    }
    
    if (!query) {
        m_errorString = "Failed to construct query";
        return list;
    }

    librdf_query_results *results;
    {
        Profiler p("SimpleSPARQLQuery: Execute LIBRDF query");
        results = librdf_query_execute(query, m_redland->getModel(modelUri));
    }

    if (!results) {
        m_errorString = "RDF query failed";
        librdf_free_query(query);
        return list;
    }

    if (!librdf_query_results_is_bindings(results)) {
        m_errorString = "RDF query returned non-bindings results";
        librdf_free_query_results(results);
        librdf_free_query(query);
        return list;
    }
    
    int resultCount = 0;
    int resultTotal = librdf_query_results_get_count(results); // probably wrong
    m_cancelled = false;

    while (!librdf_query_results_finished(results)) {

        int count = librdf_query_results_get_bindings_count(results);

        KeyValueMap resultmap;

        for (int i = 0; i < count; ++i) {

            const char *name =
                librdf_query_results_get_binding_name(results, i);

            if (!name) {
                std::cerr << "WARNING: Result " << i << " of query has no name" << std::endl;
                continue;
            }

            librdf_node *node =
                librdf_query_results_get_binding_value(results, i);

            QString key = (const char *)name;

            if (!node) {
#ifdef DEBUG_SIMPLE_SPARQL_QUERY
                std::cerr << i << ". " << key << " -> (nil)" << std::endl;
#endif
                resultmap[key] = Value();
                continue;
            }

            ValueType type = LiteralValue;
            QString text;

            if (librdf_node_is_resource(node)) {

                type = URIValue;
                librdf_uri *uri = librdf_node_get_uri(node);
                const char *us = (const char *)librdf_uri_as_string(uri);

                if (!us) {
                    std::cerr << "WARNING: Result " << i << " of query claims URI type, but has null URI" << std::endl;
                } else {
                    text = us;
                }

            } else if (librdf_node_is_literal(node)) {

                type = LiteralValue;

                const char *lit = (const char *)librdf_node_get_literal_value(node);
                if (!lit) {
                    std::cerr << "WARNING: Result " << i << " of query claims literal type, but has no literal" << std::endl;
                } else {
                    text = lit;
                }

            } else if (librdf_node_is_blank(node)) {

                type = BlankValue;

                const char *lit = (const char *)librdf_node_get_literal_value(node);
                if (lit) text = lit;

            } else {

                cerr << "SimpleSPARQLQuery: LIBRDF query returned unknown node type (not resource, literal, or blank)" << endl;
            }

#ifdef DEBUG_SIMPLE_SPARQL_QUERY
            cerr << i << ". " << key << " -> " << text << " (type " << type << ")" << endl;
#endif

            resultmap[key] = Value(type, text);

//            librdf_free_node(node);
        }

        list.push_back(resultmap);

        librdf_query_results_next(results);

        resultCount++;

        if (m_reporter) {
            if (resultCount >= resultTotal) {
                if (m_reporter->isDefinite()) m_reporter->setDefinite(false);
                m_reporter->setProgress(resultCount);
            } else {
                m_reporter->setProgress((resultCount * 100) / resultTotal);
            }

            if (m_reporter->wasCancelled()) {
                m_cancelled = true;
                break;
            }
        }
    }

    librdf_free_query_results(results);
    librdf_free_query(query);

#ifdef DEBUG_SIMPLE_SPARQL_QUERY
    SVDEBUG << "SimpleSPARQLQuery::executeDatastore: All results retrieved (" << resultCount << " of them)" << endl;
#endif

    return list;
}

bool
SimpleSPARQLQuery::Impl::addSourceToModel(QString sourceUri)
{
    QString err;

    QMutexLocker locker(&m_mutex);

    if (!m_redland) {
        m_redland = new WredlandWorldWrapper();
    }

    if (!m_redland->isOK()) {
        std::cerr << "SimpleSPARQLQuery::addSourceToModel: Failed to initialise Redland datastore" << std::endl;
        return false;
    }

    if (!m_redland->loadUriIntoDefaultModel(sourceUri, err)) {
        std::cerr << "SimpleSPARQLQuery::addSourceToModel: Failed to add source URI \"" << sourceUri << ": " << err << std::endl;
        return false;
    }
    return true;
}

void
SimpleSPARQLQuery::Impl::closeSingleSource(QString sourceUri)
{
    QMutexLocker locker(&m_mutex);

    m_redland->freeModel(sourceUri);
}

SimpleSPARQLQuery::Value
SimpleSPARQLQuery::singleResultQuery(QueryType type,
                                     QString query, QString binding)
{
    SimpleSPARQLQuery q(type, query);
    ResultList results = q.execute();
    if (!q.isOK()) {
        cerr << "SimpleSPARQLQuery::singleResultQuery: ERROR: "
             << q.getErrorString() << endl;
        return Value();
    }
    if (results.empty()) {
        return Value();
    }
    for (int i = 0; i < results.size(); ++i) {
        if (results[i].find(binding) != results[i].end() &&
            results[i][binding].type != NoValue) {
            return results[i][binding];
        }
    }
    return Value();
}



