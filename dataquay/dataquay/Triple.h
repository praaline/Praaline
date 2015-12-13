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

#ifndef _DATAQUAY_TRIPLE_H_
#define _DATAQUAY_TRIPLE_H_

#include "Node.h"

namespace Dataquay
{

/**
 * \class Triple Triple.h <dataquay/Triple.h>
 *
 * Triple represents an RDF statement made up of three Node objects.
 */
class Triple
{
public:
    /**
     * Construct a triple of three Nothing nodes.
     */
    Triple() { }

    /**
     * Construct a triple of the three given nodes.
     *
     * Our triples may contain anything, including the Nothing node
     * type for undefined elements (used in wildcard matching, etc).
     *
     * However, in order to be inserted in the RDF store, a triple
     * must have either URI or Blank type for its first (or subject)
     * node, URI for its second (or predicate) node, and either URI,
     * Blank, or Literal type for its third (or object) node.
     */
    Triple(Node _a, Node _b, Node _c) :
        a(_a), b(_b), c(_c) { }

    ~Triple() { }

    /**
     * Return the subject node. This is synonymous with accessing the
     * data member a.
     */
    Node subject() { return a; }

    /**
     * Return the predicate node. This is synonymous with accessing
     * the data member b.
     */
    Node predicate() { return b; }

    /**
     * Return the object node. This is synonymous with accessing the
     * data member c.
     */
    Node object() { return c; }
    
    bool operator<(const Triple &t) const {
        if (a != t.a) return a < t.a;
        if (b != t.b) return b < t.b;
        if (c != t.c) return c < t.c;
        return false;
    }

    Node a;
    Node b;
    Node c;
};

bool operator==(const Triple &a, const Triple &b);
bool operator!=(const Triple &a, const Triple &b);
    
QDataStream &operator<<(QDataStream &out, const Triple &);
QDataStream &operator>>(QDataStream &in, Triple &);

std::ostream &operator<<(std::ostream &out, const Triple &);
QTextStream &operator<<(QTextStream &out, const Triple &);

/// A list of RDF triples.
class Triples : public QList<Triple> {
public:
    /**
     * Return true if the two Triples lists contain the same elements.
     * Triples is an ordered list, so operator== returns true only if
     * the two lists have the same elements in the same order; this
     * test is independent of order (although not independent of
     * number, in the case of duplicate triples) and so may be more
     * meaningful in some cases.
     */
    bool matches(const Triples &other) const {
        if (this == &other) return true;
        if (size() != other.size()) return false;
        if (size() < 2) return QList<Triple>::operator==(other);
        // Triple has operator< but not qHash, hence use QMap rather than QSet
        QMap<Triple, int> a, b;
        foreach (Triple t, *this) ++a[t];
        foreach (Triple t, other) ++b[t];
        return a == b;
    }

    Nodes subjects() {
        Nodes result;
        foreach (Triple t, *this) result.push_back(t.a);
        return result;
    }
    Nodes predicates() {
        Nodes result;
        foreach (Triple t, *this) result.push_back(t.b);
        return result;
    }
    Nodes objects() {
        Nodes result;
        foreach (Triple t, *this) result.push_back(t.c);
        return result;
    }
};

}
 
#endif
