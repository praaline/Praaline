/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Dataquay

    A C++/Qt library for simple RDF datastore management.
    Copyright 2009-2012 Chris Cannam.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the name of Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#ifdef USE_REDLAND

#include "BasicStore.h"
#include "RDFException.h"

#include <redland.h>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QHash>
#include <QFile>
#include <QCryptographicHash>
#include <QReadWriteLock>

#include "../Debug.h"

#include <cstdlib>
#include <iostream>

namespace Dataquay
{

class BasicStore::D
{
public:
    D() : m_storage(0), m_model(0), m_counter(0) {
        m_prefixes["rdf"] = Uri("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
        m_prefixes["xsd"] = Uri("http://www.w3.org/2001/XMLSchema#");
        clear();
    }

    ~D() {
        QMutexLocker locker(&m_librdfLock);
        if (m_model) librdf_free_model(m_model);
        if (m_storage) librdf_free_storage(m_storage);
    }
    
    QString getNewString() const {
        QString s =
            QString::fromLocal8Bit
            (QCryptographicHash::hash
             (QString("%1").arg(random() + time(0)).toLocal8Bit(),
              QCryptographicHash::Sha1).toHex())
            .left(12);
        // This may be used as the whole of a name in some contexts,
        // so it must not start with a digit
        if (s[0].isDigit()) {
            s = "x" + s.right(s.length()-1);
        }
        return s;
    }
    
    void collision() const {
        // If we get a collision when generating a "random" string,
        // seed the random number generator (it probably means the
        // generator hasn't been seeded at all).  But only once.
        static QMutex m;
        static bool seeded = false;
        static QMutexLocker l(&m);
        if (!seeded) return;
        srandom(time(0));
        seeded = true;
    }

    void setBaseUri(Uri baseUri) {
        QMutexLocker plocker(&m_prefixLock);
        m_baseUri = baseUri;
        m_prefixes[""] = m_baseUri;
    }
    
    Uri getBaseUri() const {
        return m_baseUri;
    }

    void clear() {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::clear" << endl;
        if (m_model) librdf_free_model(m_model);
        if (m_storage) librdf_free_storage(m_storage);
        m_storage = librdf_new_storage(m_w.getWorld(), "trees", 0, 0);
        if (!m_storage) {
            DEBUG << "Failed to create RDF trees storage, falling back to default storage type" << endl;
            m_storage = librdf_new_storage(m_w.getWorld(), 0, 0, 0);
            if (!m_storage) throw RDFInternalError("Failed to create RDF data storage");
        }
        m_model = librdf_new_model(m_w.getWorld(), m_storage, 0);
        if (!m_model) throw RDFInternalError("Failed to create RDF data model");
    }

    void addPrefix(QString prefix, Uri uri) {
        QMutexLocker plocker(&m_prefixLock);
        m_prefixes[prefix] = uri;
    }

    bool add(Triple t) {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::add: " << t << endl;
        return doAdd(t);
    }

    bool remove(Triple t) {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::remove: " << t << endl;
        if (t.a.type == Node::Nothing || 
            t.b.type == Node::Nothing ||
            t.c.type == Node::Nothing) {
            Triples tt = doMatch(t);
            if (tt.empty()) return false;
            DEBUG << "BasicStore::remove: Removing " << tt.size() << " triple(s)" << endl;
            for (int i = 0; i < tt.size(); ++i) {
                if (!doRemove(tt[i])) {
                    DEBUG << "Failed to remove matched triple in remove() with wildcards; triple was: " << tt[i] << endl;
                    throw RDFInternalError("Failed to remove matched statement in remove() with wildcards");
                }
            }
            return true;
        } else {
            return doRemove(t);
        }
    }

    void change(ChangeSet cs) {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::change: " << cs.size() << " changes" << endl;
        for (int i = 0; i < cs.size(); ++i) {
            ChangeType type = cs[i].first;
            Triple triple = cs[i].second;
            switch (type) {
            case AddTriple:
                if (!doAdd(triple)) {
                    throw RDFException("Change add failed: triple is already in store", triple);
                }
                break;
            case RemoveTriple:
                if (!doRemove(cs[i].second)) {
                    throw RDFException("Change remove failed: triple is not in store", triple);
                }
                break;
            }
        }
    }

    void revert(ChangeSet cs) {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::revert: " << cs.size() << " changes" << endl;
        for (int i = cs.size()-1; i >= 0; --i) {
            ChangeType type = cs[i].first;
            Triple triple = cs[i].second;
            switch (type) {
            case AddTriple:
                if (!doRemove(triple)) {
                    throw RDFException("Revert of add failed: triple is not in store", triple);
                }
                break;
            case RemoveTriple:
                if (!doAdd(triple)) {
                    throw RDFException("Revert of remove failed: triple is already in store", triple);
                }
                break;
            }
        }
    }

    bool contains(Triple t) const {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::contains: " << t << endl;
        librdf_statement *statement = tripleToStatement(t);
        if (!checkComplete(statement)) {
            librdf_free_statement(statement);
            throw RDFException("Failed to test for triple (statement is incomplete)");
        }
        if (!librdf_model_contains_statement(m_model, statement)) {
            librdf_free_statement(statement);
            return false;
        } else {
            librdf_free_statement(statement);
            return true;
        }
    }
    
    Triples match(Triple t) const {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::match: " << t << endl;
        Triples result = doMatch(t);
#ifndef NDEBUG
        DEBUG << "BasicStore::match result (size " << result.size() << "):" << endl;
        for (int i = 0; i < result.size(); ++i) {
            DEBUG << i << ". " << result[i] << endl;
        }
#endif
        return result;
    }

    Node complete(Triple t) const {
        int count = 0, match = 0;
        if (t.a == Node()) { ++count; match = 0; }
        if (t.b == Node()) { ++count; match = 1; }
        if (t.c == Node()) { ++count; match = 2; }
        if (count != 1) {
            throw RDFException("Cannot complete triple unless it has only a single wildcard node", t);
        }
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::complete: " << t << endl;
        Triples result = doMatch(t, true);
        if (result.empty()) return Node();
        else switch (match) {
            case 0: return result[0].a;
            case 1: return result[0].b;
            case 2: return result[0].c;
            default: return Node();
            }
    }

    Triple matchOnce(Triple t) const {
        if (t.c != Node() && t.b != Node() && t.a != Node()) {
            // triple is complete: short-circuit to a single lookup
            if (contains(t)) return t;
            else return Triple();
        }
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::matchOnce: " << t << endl;
        Triples result = doMatch(t, true);
#ifndef NDEBUG
        DEBUG << "BasicStore::matchOnce result:" << endl;
        for (int i = 0; i < result.size(); ++i) {
            DEBUG << i << ". " << result[i] << endl;
        }
#endif
        if (result.empty()) return Triple();
        else return result[0];
    }

    ResultSet query(QString sparql) const {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::query: " << sparql << endl;
        ResultSet rs = runQuery(sparql);
        return rs;
    }

    Node queryOnce(QString sparql, QString bindingName) const {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::queryOnce: " << bindingName << " from " << sparql << endl;
        ResultSet rs = runQuery(sparql);
        if (rs.empty()) return Node();
        for (ResultSet::const_iterator i = rs.begin(); i != rs.end(); ++i) {
            Dictionary::const_iterator j = i->find(bindingName);
            if (j == i->end()) continue;
            if (j->type == Node::Nothing) continue;
            return *j;
        }
        return Node();
    }

    Uri getUniqueUri(QString prefix) const {
        QMutexLocker locker(&m_librdfLock);
        DEBUG << "BasicStore::getUniqueUri: prefix " << prefix << endl;
        int base = (int)(long)this; // we don't care at all about overflow
        bool good = false;
        Uri uri;
        while (!good) {
            QString s = getNewString();
            uri = expand(prefix + s);
            Triples t = doMatch(Triple(uri, Node(), Node()), true);
            if (t.empty()) good = true;
            else collision();
        }
        return uri;
    }

    Uri expand(QString shrt) const {

        if (shrt == "a") {
            return Uri::rdfTypeUri();
        }

        int index = shrt.indexOf(':');
        if (index == 0) {
            // starts with colon
            return Uri(m_baseUri.toString() + shrt.right(shrt.length() - 1));
        } else if (index > 0) {
            // colon appears in middle somewhere
            if (index + 2 < shrt.length() &&
                shrt[index+1] == '/' &&
                shrt[index+2] == '/') {
                // we have found "://", this is a scheme, therefore
                // the uri is already expanded
                return Uri(shrt);
            }
        } else {
            // no colon present, no possibility of expansion
            return Uri(shrt);
        }

        // fall through only for colon in middle and no "://" found,
        // i.e. a plausible prefix appears

        QString prefix = shrt.left(index);
        QString expanded;

        m_prefixLock.lock();
        PrefixMap::const_iterator pi = m_prefixes.find(prefix);
        if (pi != m_prefixes.end()) {
            expanded = pi.value().toString() +
                shrt.right(shrt.length() - (index + 1));
        } else {
            expanded = shrt;
        }
        m_prefixLock.unlock();

        return Uri(expanded);
    }

    Node addBlankNode() {
        QMutexLocker locker(&m_librdfLock);
        librdf_node *node = librdf_new_node_from_blank_identifier(m_w.getWorld(), 0);
        if (!node) throw RDFInternalError("Failed to create new blank node");
        return lrdfNodeToNode(node);
    }

    void save(QString filename) const {

        QMutexLocker wlocker(&m_librdfLock);
        QMutexLocker plocker(&m_prefixLock);

        DEBUG << "BasicStore::save(" << filename << ")" << endl;

        librdf_uri *base_uri = uriToLrdfUri(m_baseUri);
        librdf_serializer *s = librdf_new_serializer(m_w.getWorld(), "turtle", 0, 0);
        if (!s) throw RDFInternalError("Failed to construct RDF serializer");

        for (PrefixMap::const_iterator i = m_prefixes.begin();
             i != m_prefixes.end(); ++i) {
            QByteArray b = i.key().toUtf8();
            librdf_serializer_set_namespace(s, uriToLrdfUri(i.value()), b.data());
        }
        librdf_serializer_set_namespace(s, uriToLrdfUri(m_baseUri), "");

        QFile f(filename);
        if (!f.exists()) {
            if (!f.open(QFile::WriteOnly)) {
                throw RDFException("Failed to open file for writing", filename);
            }
            f.close();
        }

        QString tmpFilename = QString("%1.part").arg(filename);
        QByteArray b = QFile::encodeName(tmpFilename);
        const char *lname = b.data();
        
        if (librdf_serializer_serialize_model_to_file(s, lname, base_uri, m_model)) {
            librdf_free_serializer(s);
            QFile::remove(tmpFilename);
            throw RDFException("Failed to export RDF model to temporary file",
                               tmpFilename);
        }

        librdf_free_serializer(s);

        // New file is now completed; the following is scruffy, but
        // that shouldn't really matter now

        if (!QFile::remove(filename)) {
            // Not necessarily fatal
            DEBUG << "BasicStore::save: Failed to remove former save file "
                  << filename << endl;
        }
        if (!QFile::rename(tmpFilename, filename)) {
            throw RDFException("Failed to rename temporary file to save file",
                               filename);
        }
    }

    void import(QUrl url, ImportDuplicatesMode idm, QString format) {

        QMutexLocker wlocker(&m_librdfLock);
        QMutexLocker plocker(&m_prefixLock);

        QString base = m_baseUri.toString();
        if (base == "") {
            // No base URI in store: use file URL as base
            base = url.toString();
        }

        // Redland doesn't like file://x for relative file paths, it
        // insists on file:x (the opposite of Serd)
        QString fileUri = Uri(url).toString();
        if (fileUri.startsWith("file://") && !fileUri.startsWith("file:///")) {
            fileUri = "file:" + fileUri.right(fileUri.length()-7);
        }

        librdf_uri *luri = librdf_new_uri
            (m_w.getWorld(), (const unsigned char *)fileUri.toUtf8().data());
        librdf_uri *base_uri = uriToLrdfUri(Uri(base));

        if (format == "") format = "guess";

        librdf_parser *parser = librdf_new_parser
            (m_w.getWorld(), format.toLocal8Bit().data(), NULL, NULL);
        if (!parser) {
            throw RDFInternalError("Failed to construct RDF parser");
        }

        if (idm == ImportPermitDuplicates) {
            // The normal Redland behaviour, so the easy case.
            if (librdf_parser_parse_into_model
                (parser, luri, base_uri, m_model)) {
                librdf_free_parser(parser);
                DEBUG << "librdf_parser_parse_into_model failed" << endl;
                DEBUG << "luri = " << (const char *)librdf_uri_as_string(luri) << ", base_uri = " << (const char *)librdf_uri_as_string(base_uri) << endl;
                throw RDFException("Failed to import model from URL",
                                   url.toString());
            }
        } else { // ImportFailOnDuplicates and ImportIgnoreDuplicates modes

            // This is complicated by our desire to avoid storing any
            // duplicate triples on import, and optionally to be able
            // to fail if any are found.  So we import into a separate
            // model and then transfer over.  Not very efficient, but
            // scalability is not generally the primary concern for us

            librdf_storage *is = librdf_new_storage(m_w.getWorld(), "trees", 0, 0);
            if (!is) is = librdf_new_storage(m_w.getWorld(), 0, 0, 0);
            if (!is) {
                librdf_free_parser(parser);
                throw RDFInternalError("Failed to create import RDF data storage");
            }
            librdf_model *im = librdf_new_model(m_w.getWorld(), is, 0);
            if (!im) {
                librdf_free_storage(is);
                librdf_free_parser(parser);
                throw RDFInternalError("Failed to create import RDF data model");
            }

            librdf_stream *stream = 0;
            librdf_statement *all = 0;

            try { // so as to free parser and im on exception

                //!!! This appears to be returning success even on a
                //!!! syntax error -- can this be correct?
                if (librdf_parser_parse_into_model(parser, luri, base_uri, im)) {
                    DEBUG << "librdf_parser_parse_into_model failed" << endl;
                    DEBUG << "luri = " << (const char *)librdf_uri_as_string(luri) << ", base_uri = " << (const char *)librdf_uri_as_string(base_uri) << endl;
                    throw RDFException("Failed to import model from URL",
                                       url.toString());
                }
                all = tripleToStatement(Triple());

                if (idm == ImportFailOnDuplicates) {
                    // Need to query twice, first time to check for dupes
                    stream = librdf_model_find_statements(im, all);
                    if (!stream) {
                        throw RDFInternalError("Failed to list imported RDF model in duplicates check");
                    }
                    while (!librdf_stream_end(stream)) {
                        librdf_statement *current = librdf_stream_get_object(stream);
                        if (!current) continue;
                        if (librdf_model_contains_statement(m_model, current)) {
                            throw RDFDuplicateImportException("Duplicate statement encountered on import in ImportFailOnDuplicates mode");
                        }
                        librdf_stream_next(stream);
                    }
                    librdf_free_stream(stream);
                    stream = 0;
                }

                // Now import.  Have to do this "manually" because librdf
                // may allow duplicates and we want to avoid them
                stream = librdf_model_find_statements(im, all);
                if (!stream) {
                    throw RDFInternalError("Failed to list imported RDF model");
                }
                while (!librdf_stream_end(stream)) {
                    librdf_statement *current = librdf_stream_get_object(stream);
                    if (!current) continue;
                    if (idm == ImportFailOnDuplicates || // (already tested if so)
                        !librdf_model_contains_statement(m_model, current)) {
                        librdf_model_add_statement(m_model, current);
                    }
                    librdf_stream_next(stream);
                }
                librdf_free_stream(stream);
                stream = 0;

            } catch (...) {
                if (stream) librdf_free_stream(stream);
                if (all) librdf_free_statement(all);
                librdf_free_parser(parser);
                librdf_free_model(im);
                librdf_free_storage(is);
                throw;
            }

            librdf_free_model(im);
            librdf_free_storage(is);
        }

        int namespaces = librdf_parser_get_namespaces_seen_count(parser);
        DEBUG << "Parser found " << namespaces << " namespaces" << endl;
        for (int i = 0; i < namespaces; ++i) {
            const char *pfx = librdf_parser_get_namespaces_seen_prefix(parser, i);
            librdf_uri *uri = librdf_parser_get_namespaces_seen_uri(parser, i);
            QString qpfx = QString::fromUtf8(pfx);
            Uri quri;
            try {
                quri = lrdfUriToUri(uri);
            } catch (RDFIncompleteURI &) {
                continue;
            }
            DEBUG << "namespace " << i << ": " << qpfx << " -> " << quri << endl;
            // don't call addPrefix; it tries to lock the mutex,
            // and anyway we want to add the prefix only if it
            // isn't already there (to avoid surprisingly changing
            // a prefix in unusual cases, or changing the base URI)
            if (m_prefixes.find(qpfx) == m_prefixes.end()) {
                m_prefixes[qpfx] = quri;
            }
        }

        librdf_free_parser(parser);
    }

private:
    class World
    {
    public:
        World() {
            QMutexLocker locker(&m_mutex);
            if (!m_world) {
                m_world = librdf_new_world();
                librdf_world_open(m_world);
            }
            ++m_refcount;
        }
        ~World() {
            QMutexLocker locker(&m_mutex);
            if (--m_refcount == 0) {
                DEBUG << "Freeing world" << endl;
                librdf_free_world(m_world);
                m_world = 0;
            }
        }
        
        librdf_world *getWorld() const { return m_world; }
        
    private:
        static QMutex m_mutex;
        static librdf_world *m_world;
        static int m_refcount;
    };

    World m_w;
    librdf_storage *m_storage;
    librdf_model *m_model;
    static QMutex m_librdfLock; // assume the worst

    typedef QHash<QString, Uri> PrefixMap;
    Uri m_baseUri;
    PrefixMap m_prefixes;
    mutable QMutex m_prefixLock; // also protects m_baseUri

    mutable int m_counter;

    bool doAdd(Triple t) {
        librdf_statement *statement = tripleToStatement(t);
        if (!checkComplete(statement)) {
            librdf_free_statement(statement);
            throw RDFException("Failed to add triple (statement is incomplete)");
        }
        if (librdf_model_contains_statement(m_model, statement)) {
            librdf_free_statement(statement);
            return false;
        }
        if (librdf_model_add_statement(m_model, statement)) {
            librdf_free_statement(statement);
            throw RDFInternalError("Failed to add statement to model");
        }
        librdf_free_statement(statement);
        return true;
    }

    bool doRemove(Triple t) {
        librdf_statement *statement = tripleToStatement(t);
        if (!checkComplete(statement)) {
            librdf_free_statement(statement);
            throw RDFException("Failed to remove triple (statement is incomplete)");
        }
        // Looks like librdf_model_remove_statement returns the wrong
        // value in trees storage as of 1.0.9, so let's do this check
        // separately and ignore its return value
        if (!librdf_model_contains_statement(m_model, statement)) {
            librdf_free_statement(statement);
            return false;
        }
        librdf_model_remove_statement(m_model, statement);
        librdf_free_statement(statement);
        return true;
    }

    librdf_uri *uriToLrdfUri(Uri uri) const {
        librdf_uri *luri = librdf_new_uri
            (m_w.getWorld(),
             (const unsigned char *)uri.toString().toUtf8().data());
        if (!luri) throw RDFInternalError("Failed to convert URI to internal representation", uri);
        return luri;
    }

    Uri lrdfUriToUri(librdf_uri *u) const {
        const char *s = (const char *)librdf_uri_as_string(u);
        if (s) return Uri(QString::fromUtf8(s));
        else return Uri();
    }

    librdf_node *nodeToLrdfNode(Node v) const { // called with m_librdfLock held
        librdf_node *node = 0;
        switch (v.type) {
        case Node::Nothing:
            return 0;
        case Node::Blank: {
            QByteArray b = v.value.toUtf8();
            const unsigned char *bident = (const unsigned char *)b.data();
            node = librdf_new_node_from_blank_identifier(m_w.getWorld(), bident);
            if (!node) throw RDFException
                           ("Failed to construct node from blank identifier",
                            v.value);
        }
            break;
        case Node::URI: {
            librdf_uri *uri = uriToLrdfUri(Uri(v.value));
            if (!uri) throw RDFException("Failed to construct URI from value ", v.value);
            node = librdf_new_node_from_uri(m_w.getWorld(), uri);
            if (!node) throw RDFException("Failed to construct node from URI");
        }
            break;
        case Node::Literal: {
            QByteArray b = v.value.toUtf8();
            const unsigned char *literal = (const unsigned char *)b.data();
            if (v.datatype != Uri()) {
                Uri dtu = v.datatype;
                librdf_uri *type_uri = uriToLrdfUri(dtu);
                node = librdf_new_node_from_typed_literal
                    (m_w.getWorld(), literal, 0, type_uri);
                if (!node) throw RDFException
                               ("Failed to construct node from literal of type ",
                                v.datatype);
            } else {
                node = librdf_new_node_from_literal
                    (m_w.getWorld(), literal, 0, 0);
                if (!node) throw RDFException
                               ("Failed to construct node from literal");
            }
        }
            break;
        }
        return node;
    }

    Node lrdfNodeToNode(librdf_node *node) const {
        
        Node v;
        if (!node) return v;

        if (librdf_node_is_resource(node)) {

            v.type = Node::URI;
            librdf_uri *uri = librdf_node_get_uri(node);
            v.value = lrdfUriToUri(uri).toString();

        } else if (librdf_node_is_literal(node)) {

            v.type = Node::Literal;
            const char *s = (const char *)librdf_node_get_literal_value(node);
            if (s) v.value = QString::fromUtf8(s);
            librdf_uri *type_uri = librdf_node_get_literal_value_datatype_uri(node);
            if (type_uri) v.datatype = lrdfUriToUri(type_uri);
            
        } else if (librdf_node_is_blank(node)) {

            v.type = Node::Blank;
            const char *s = (const char *)librdf_node_get_blank_identifier(node);
            if (s) v.value = s;
        }

        return v;
    }

    librdf_statement *tripleToStatement(Triple t) const {
        librdf_node *na = nodeToLrdfNode(t.a);
        librdf_node *nb = nodeToLrdfNode(t.b);
        librdf_node *nc = nodeToLrdfNode(t.c);
        librdf_statement *statement = 
            librdf_new_statement_from_nodes(m_w.getWorld(), na, nb, nc);
        if (!statement) throw RDFException("Failed to construct statement");
        return statement;
    }

    Triple statementToTriple(librdf_statement *statement) const {
        librdf_node *subject = librdf_statement_get_subject(statement);
        librdf_node *predicate = librdf_statement_get_predicate(statement);
        librdf_node *object = librdf_statement_get_object(statement);
        Triple triple(lrdfNodeToNode(subject),
                      lrdfNodeToNode(predicate),
                      lrdfNodeToNode(object));
        return triple;
    }

    bool checkComplete(librdf_statement *statement) const {
        if (librdf_statement_is_complete(statement)) return true;
        else {
            unsigned char *text = librdf_statement_to_string(statement);
            QString str = QString::fromUtf8((char *)text);
            std::cerr << "BasicStore::checkComplete: WARNING: RDF statement is incomplete: " << str.toStdString() << std::endl;
            free(text);
            return false;
        }
    }

    Triples doMatch(Triple t, bool single = false) const {
        // Any of a, b, and c in t that have Nothing as their node type
        // will contribute all matching nodes to the returned triples
        Triples results;
        librdf_statement *templ = tripleToStatement(t);
        librdf_stream *stream = librdf_model_find_statements(m_model, templ);
        librdf_free_statement(templ);
        if (!stream) throw RDFInternalError("Failed to match RDF triples");
        while (!librdf_stream_end(stream)) {
            librdf_statement *current = librdf_stream_get_object(stream);
            if (current) results.push_back(statementToTriple(current));
            if (single) break;
            librdf_stream_next(stream);
        }
        librdf_free_stream(stream);
        return results;
    }

    ResultSet runQuery(QString rawQuery) const {

        QString sparql;
        m_prefixLock.lock();
        for (PrefixMap::const_iterator i = m_prefixes.begin();
             i != m_prefixes.end(); ++i) {
            sparql += QString(" PREFIX %1: <%2> ")
                .arg(i.key()).arg(i.value().toString());
        }
        m_prefixLock.unlock();
        sparql += rawQuery;

        ResultSet returned;
        librdf_query *query =
            librdf_new_query(m_w.getWorld(), "sparql", 0,
                             (const unsigned char *)sparql.toUtf8().data(), 0);
        if (!query) return returned;

        librdf_query_results *results = librdf_query_execute(query, m_model);
        if (!results) {
            librdf_free_query(query);
            return returned;
        }
        if (!librdf_query_results_is_bindings(results)) {
            librdf_free_query_results(results);
            librdf_free_query(query);
            return returned;
        }
    
        while (!librdf_query_results_finished(results)) {
            int count = librdf_query_results_get_bindings_count(results);
            Dictionary dict;
            for (int i = 0; i < count; ++i) {

                const char *name =
                    librdf_query_results_get_binding_name(results, i);
                if (!name) continue;
                QString key = (const char *)name;

                librdf_node *node =
                    librdf_query_results_get_binding_value(results, i);

                dict[key] = lrdfNodeToNode(node);
            }

            returned.push_back(dict);
            librdf_query_results_next(results);
        }

        librdf_free_query_results(results);
        librdf_free_query(query);

//        DEBUG << "runQuery: returning " << returned.size() << " result(s)" << endl;
        
        return returned;
    }
};

QMutex
BasicStore::D::m_librdfLock;

QMutex
BasicStore::D::World::m_mutex;

librdf_world *
BasicStore::D::World::m_world = 0;

int
BasicStore::D::World::m_refcount = 0;

BasicStore::BasicStore() :
    m_d(new D())
{
}

BasicStore::~BasicStore()
{
    delete m_d;
}

void
BasicStore::setBaseUri(Uri uri)
{
    m_d->setBaseUri(uri);
}

Uri
BasicStore::getBaseUri() const
{
    return m_d->getBaseUri();
}

void
BasicStore::clear()
{
    m_d->clear();
}

bool
BasicStore::add(Triple t)
{
    return m_d->add(t);
}

bool
BasicStore::remove(Triple t)
{
    return m_d->remove(t);
}

void
BasicStore::change(ChangeSet t)
{
    m_d->change(t);
}

void
BasicStore::revert(ChangeSet t)
{
    m_d->revert(t);
}

bool
BasicStore::contains(Triple t) const
{
    return m_d->contains(t);
}

Triples
BasicStore::match(Triple t) const
{
    return m_d->match(t);
}

void
BasicStore::addPrefix(QString prefix, Uri uri)
{
    m_d->addPrefix(prefix, uri);
}

ResultSet
BasicStore::query(QString sparql) const
{
    return m_d->query(sparql);
}

Node
BasicStore::complete(Triple t) const
{
    return m_d->complete(t);
}

Triple
BasicStore::matchOnce(Triple t) const
{
    return m_d->matchOnce(t);
}

Node
BasicStore::queryOnce(QString sparql, QString bindingName) const
{
    return m_d->queryOnce(sparql, bindingName);
}

Uri
BasicStore::getUniqueUri(QString prefix) const
{
    return m_d->getUniqueUri(prefix);
}

Uri
BasicStore::expand(QString uri) const
{
    return m_d->expand(uri);
}

Node
BasicStore::addBlankNode()
{
    return m_d->addBlankNode();
}

void
BasicStore::save(QString filename) const
{
    m_d->save(filename);
}

void
BasicStore::import(QUrl url, ImportDuplicatesMode idm, QString format)
{
    m_d->import(url, idm, format);
}

BasicStore *
BasicStore::load(QUrl url, QString format)
{
    BasicStore *s = new BasicStore();
    s->setBaseUri(Uri(url));
    // store is empty, ImportIgnoreDuplicates is faster
    s->import(url, ImportIgnoreDuplicates, format);
    return s;
}

BasicStore::Features
BasicStore::getSupportedFeatures() const
{
    Features fs;
    fs << ModifyFeature << QueryFeature << RemoteImportFeature;
    return fs;
}

}

#endif


		
