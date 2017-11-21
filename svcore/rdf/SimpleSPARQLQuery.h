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

#ifndef _SIMPLE_SPARQL_QUERY_H_
#define _SIMPLE_SPARQL_QUERY_H_

#ifdef NOT_DEFINED

#include <QString>
#include <map>
#include <vector>

#include "base/Debug.h"

class ProgressReporter;

class SimpleSPARQLQuery
{
public:
    enum ValueType { NoValue, URIValue, LiteralValue, BlankValue };

    struct Value {
        Value() : type(NoValue), value() { }
        Value(ValueType t, QString v) : type(t), value(v) { }
        ValueType type;
        QString value;
    };

    typedef std::map<QString, Value> KeyValueMap;
    typedef std::vector<KeyValueMap> ResultList;

    /**
     * QueryType specifies the context in which the query will be
     * evaluated.  SimpleSPARQLQuery maintains a general global data
     * model, into which data can be loaded using addSourceToModel(),
     * as well as permitting one-time queries directly on data sources
     * identified by URL.
     *
     * The query type QueryFromModel indicates a query to be evaluated
     * over the general global model; the query type
     * QueryFromSingleSource indicates that the query should be
     * evaluated in the context of a model generated solely by parsing
     * the FROM url found in the query.
     *
     * Even in QueryFromSingleSource mode, the parsed data remains in
     * memory and will be reused in subsequent queries with the same
     * mode and FROM url.  To release data loaded in this way once all
     * queries across it are complete, pass the said FROM url to
     * closeSingleSource().
     */
    enum QueryType {
        QueryFromModel,
        QueryFromSingleSource
    };

    /**
     * Construct a query of the given type (indicating the data model
     * context for the query) using the given SPARQL query content.
     */
    SimpleSPARQLQuery(QueryType type, QString query);
    ~SimpleSPARQLQuery();

    /**
     * Add the given URI to the general global model used for
     * QueryFromModel queries.
     */
    static bool addSourceToModel(QString sourceUri);

    /**
     * Release any data that has been loaded from the given source as
     * part of a QueryFromSingleSource query with this source in the
     * FROM clause.  Note this will not prevent any subsequent queries
     * on the source from working -- it will just make them slower as
     * the data will need to be re-parsed.
     */
    static void closeSingleSource(QString sourceUri);

    void setProgressReporter(ProgressReporter *reporter);
    bool wasCancelled() const;
    
    ResultList execute();

    bool isOK() const;
    QString getErrorString() const;

    /**
     * Construct and execute a query, and return the first result
     * value for the given binding.
     */
    static Value singleResultQuery(QueryType type,
                                   QString query,
                                   QString binding);

protected:
    class Impl;
    Impl *m_impl;

private:
    SimpleSPARQLQuery(const SimpleSPARQLQuery &); // not provided
    SimpleSPARQLQuery &operator=(const SimpleSPARQLQuery &); // not provided
};

#endif

#endif
