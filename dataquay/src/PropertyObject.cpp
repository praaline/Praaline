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

#include "Uri.h"

#include "PropertyObject.h"

#include "Transaction.h"
#include "Node.h"

namespace Dataquay
{

QString
PropertyObject::m_defaultPrefix = "property:";

PropertyObject::PropertyObject(Store *s, Uri uri) :
    m_store(s), m_pfx(m_defaultPrefix), m_node(uri)
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::PropertyObject(Store *s, QString uri) :
    m_store(s), m_pfx(m_defaultPrefix), m_node(s->expand(uri))
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::PropertyObject(Store *s, Node node) :
    m_store(s), m_pfx(m_defaultPrefix), m_node(node)
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::PropertyObject(Store *s, QString pfx, Uri uri) :
    m_store(s), m_pfx(pfx), m_node(uri)
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::PropertyObject(Store *s, QString pfx, QString uri) :
    m_store(s), m_pfx(pfx), m_node(s->expand(uri))
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::PropertyObject(Store *s, QString pfx, Node node) :
    m_store(s), m_pfx(pfx), m_node(node)
{
    m_upfx = m_store->expand(m_pfx);
}

PropertyObject::~PropertyObject()
{
}

Node
PropertyObject::getNode() const
{
    return m_node;
}

Uri
PropertyObject::getObjectType() const
{
    Triple t = m_store->matchOnce(Triple(m_node, Uri("a"), Node()));
    if (t != Triple()) {
        return Uri(t.c.value);
    } else {
        return Uri();
    }
}

Uri
PropertyObject::getObjectType(Transaction *tx) const
{
    Store *s = getStore(tx);
    Triple t = s->matchOnce(Triple(m_node, Uri("a"), Node()));
    if (t != Triple()) {
        return Uri(t.c.value);
    } else {
        return Uri();
    }
}

bool
PropertyObject::hasProperty(QString name) const
{
    Uri property = getPropertyUri(name);
    Triple r = m_store->matchOnce(Triple(m_node, property, Node()));
    return (r != Triple());
}

bool
PropertyObject::hasProperty(Transaction *tx, QString name) const
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple r = s->matchOnce(Triple(m_node, property, Node()));
    return (r != Triple());
}

QVariant
PropertyObject::getProperty(QString name) const
{
    Uri property = getPropertyUri(name);
    Triple r = m_store->matchOnce(Triple(m_node, property, Node()));
    if (r == Triple()) return QVariant();
    return r.c.toVariant();
}

QVariant
PropertyObject::getProperty(Transaction *tx, QString name) const
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple r = s->matchOnce(Triple(m_node, property, Node()));
    if (r == Triple()) return QVariant();
    return r.c.toVariant();
}

QVariantList
PropertyObject::getPropertyList(QString name) const
{
    Uri property = getPropertyUri(name);
    Triples r = m_store->match(Triple(m_node, property, Node()));
    QVariantList vl;
    for (int i = 0; i < r.size(); ++i) {
        vl.push_back(r[i].c.toVariant());
    }
    return vl;
}

QVariantList
PropertyObject::getPropertyList(Transaction *tx, QString name) const
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triples r = s->match(Triple(m_node, property, Node()));
    QVariantList vl;
    for (int i = 0; i < r.size(); ++i) {
        vl.push_back(r[i].c.toVariant());
    }
    return vl;
}

Node
PropertyObject::getPropertyNode(QString name) const
{
    Uri property = getPropertyUri(name);
    Triple r = m_store->matchOnce(Triple(m_node, property, Node()));
    return r.c;
}

Node
PropertyObject::getPropertyNode(Transaction *tx, QString name) const
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple r = s->matchOnce(Triple(m_node, property, Node()));
    return r.c;
}

Nodes
PropertyObject::getPropertyNodeList(QString name) const
{
    Uri property = getPropertyUri(name);
    Triples r = m_store->match(Triple(m_node, property, Node()));
    Nodes n;
    for (int i = 0; i < r.size(); ++i) n.push_back(r[i].c);
    return n;
}

Nodes
PropertyObject::getPropertyNodeList(Transaction *tx, QString name) const
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triples r = s->match(Triple(m_node, property, Node()));
    Nodes n;
    for (int i = 0; i < r.size(); ++i) n.push_back(r[i].c);
    return n;
}

QStringList
PropertyObject::getPropertyNames() const
{
    QStringList properties;
    Triples ts = m_store->match(Triple(m_node, Node(), Node()));
    for (int i = 0; i < ts.size(); ++i) {
        QString propertyUri = ts[i].b.value;
        if (propertyUri.startsWith(m_pfx)) {
            properties.push_back(propertyUri.remove(0, m_pfx.length()));
        }
    }
    return properties;
}

QStringList
PropertyObject::getPropertyNames(Transaction *tx) const
{
    Store *s = getStore(tx);
    QStringList properties;
    Triples ts = s->match(Triple(m_node, Node(), Node()));
    for (int i = 0; i < ts.size(); ++i) {
        QString propertyUri = ts[i].b.value;
        if (propertyUri.startsWith(m_pfx)) {
            properties.push_back(propertyUri.remove(0, m_pfx.length()));
        }
    }
    return properties;
}

void
PropertyObject::setProperty(QString name, QVariant value)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
    t.c = Node::fromVariant(value);
    m_store->add(t);
}

void
PropertyObject::setProperty(QString name, Uri uri)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
    t.c = Node(uri);
    m_store->add(t);
}

void
PropertyObject::setProperty(QString name, Node node)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
    t.c = node;
    m_store->add(t);
}

void
PropertyObject::setProperty(Transaction *tx, QString name, QVariant value)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
    t.c = Node::fromVariant(value);
    s->add(t);
}

void
PropertyObject::setProperty(Transaction *tx, QString name, Uri uri)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
    t.c = Node(uri);
    s->add(t);
}

void
PropertyObject::setProperty(Transaction *tx, QString name, Node node)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
    t.c = node;
    s->add(t);
}

void
PropertyObject::setPropertyList(QString name, QVariantList values)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
    for (int i = 0; i < values.size(); ++i) {
        t.c = Node::fromVariant(values[i]);
        m_store->add(t);
    }
}

void
PropertyObject::setPropertyList(Transaction *tx,
                                QString name, QVariantList values)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
    for (int i = 0; i < values.size(); ++i) {
        t.c = Node::fromVariant(values[i]);
        s->add(t);
    }
}
  
void
PropertyObject::setPropertyList(QString name, Nodes nodes)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
    for (int i = 0; i < nodes.size(); ++i) {
        t.c = nodes[i];
        m_store->add(t);
    }
}

void
PropertyObject::setPropertyList(Transaction *tx, QString name, Nodes nodes)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
    for (int i = 0; i < nodes.size(); ++i) {
        t.c = nodes[i];
        s->add(t);
    }
}
    
void
PropertyObject::removeProperty(QString name)
{
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    m_store->remove(t); // remove all matching triples
}
    
void
PropertyObject::removeProperty(Transaction *tx, QString name)
{
    Store *s = getStore(tx);
    Uri property = getPropertyUri(name);
    Triple t(m_node, property, Node());
    s->remove(t); // remove all matching triples
}

Store *
PropertyObject::getStore(Transaction *tx) const
{
    if (tx == NoTransaction) return m_store;
    else return tx;
}

Uri
PropertyObject::getPropertyUri(QString name) const
{
    if (name == "a") return m_store->expand(name);
    if (name.contains(':')) return m_store->expand(name);
    return Uri(m_upfx.toString() + name); // m_upfx is already expanded
}

void
PropertyObject::setDefaultPropertyPrefix(QString prefix)
{
    m_defaultPrefix = prefix;
}

CacheingPropertyObject::CacheingPropertyObject(Store *s, Uri uri) :
    m_po(s, uri),
    m_cached(false)
{
}

CacheingPropertyObject::CacheingPropertyObject(Store *s, QString uri) :
    m_po(s, uri),
    m_cached(false)
{
}

CacheingPropertyObject::CacheingPropertyObject(Store *s, QString pfx, Uri uri) :
    m_po(s, pfx, uri),
    m_cached(false)
{
}

CacheingPropertyObject::CacheingPropertyObject(Store *s, QString pfx, QString uri) :
    m_po(s, pfx, uri),
    m_cached(false)
{
}

CacheingPropertyObject::CacheingPropertyObject(Store *s, QString pfx, Node node) :
    m_po(s, pfx, node),
    m_cached(false)
{
}

Uri
CacheingPropertyObject::getObjectType() const
{
    encache();
    Uri key = Uri::rdfTypeUri();
    if (!m_cache.contains(key) || m_cache[key][0].type != Node::URI) {
        return Uri();
    }
    return Uri(m_cache[key][0].value);
}

bool
CacheingPropertyObject::hasProperty(QString name) const
{
    encache();
    Uri key = m_po.getPropertyUri(name);
    bool has = m_cache.contains(key);
    return has;
}

QVariant
CacheingPropertyObject::getProperty(QString name) const
{
    encache();
    Uri key = m_po.getPropertyUri(name);
    if (!m_cache.contains(key)) return QVariant();
    return m_cache[key][0].toVariant();
}

QVariantList
CacheingPropertyObject::getPropertyList(QString name) const
{
    encache();
    Uri key = m_po.getPropertyUri(name);
    if (!m_cache.contains(key)) return QVariantList();
    QVariantList vl;
    foreach (const Node &n, m_cache[key]) {
        vl.push_back(n.toVariant());
    }
    return vl;
}

Node
CacheingPropertyObject::getPropertyNode(QString name) const
{
    encache();
    Uri key = m_po.getPropertyUri(name);
    if (!m_cache.contains(key)) return Node();
    return m_cache[key][0];
}

Nodes
CacheingPropertyObject::getPropertyNodeList(QString name) const
{
    encache();
    Uri key = m_po.getPropertyUri(name);
    Nodes result;
    if (!m_cache.contains(key)) return result;
    result = m_cache[key];
    return result;
}

QStringList
CacheingPropertyObject::getPropertyNames() const
{
    return m_po.getPropertyNames();
}

void
CacheingPropertyObject::setProperty(QString name, QVariant value)
{
    m_po.setProperty(name, value);
    m_cached = false;
}

void
CacheingPropertyObject::setProperty(QString name, Uri value)
{
    m_po.setProperty(name, value);
    m_cached = false;
}

void
CacheingPropertyObject::setProperty(QString name, Node node)
{
    m_po.setProperty(name, node);
    m_cached = false;
}

void
CacheingPropertyObject::setPropertyList(QString name, QVariantList values)
{
    m_po.setPropertyList(name, values);
    m_cached = false;
}

void
CacheingPropertyObject::setPropertyList(QString name, Nodes nodes)
{
    m_po.setPropertyList(name, nodes);
    m_cached = false;
}

void
CacheingPropertyObject::removeProperty(QString name)
{
    m_po.removeProperty(name);
    m_cached = false;
}

Uri
CacheingPropertyObject::getPropertyUri(QString name) const
{
    return m_po.getPropertyUri(name);
}

void
CacheingPropertyObject::encache() const
{
    if (m_cached) return;

    m_cache.clear();

    Triples ts = m_po.getStore(NoTransaction)
        ->match(Triple(m_po.getNode(), Node(), Node()));

    for (int i = 0; i < ts.size(); ++i) {
        if (ts[i].b.type != Node::URI) continue; // shouldn't happen, but
        m_cache[Uri(ts[i].b.value)].push_back(ts[i].c);
    }

    m_cached = true;
}

}

