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

#ifndef _DATAQUAY_OBJECT_MAPPER_H_
#define _DATAQUAY_OBJECT_MAPPER_H_

#include "../Node.h"
#include "../Store.h"

#include "ObjectMapperDefs.h"

#include <QObject>

namespace Dataquay
{

class TransactionalStore;
class TypeMapping;

/**
 * \class ObjectMapper ObjectMapper.h <dataquay/objectmapper/ObjectMapper.h>
 *
 * ObjectMapper manages a set of objects, maintaining a consistent
 * record of their state in a TransactionalStore by mapping changes in
 * both directions between objects and store.  It uses ObjectStorer to
 * map objects (derived from QObject) to the store, and then watches
 * both the objects and the store for changes, applying to the store
 * any changes in the objects and using ObjectLoader to bring the
 * objects up to date with any changes in the store.
 *
 * See ObjectStorer for details of how objects are mapped to the RDF
 * store, and ObjectLoader for details of how changes in the RDF store
 * are mapped back to the objects.
 *
 * ObjectMapper watches QObject properties' notify signals to
 * determine when a property has changed, and uses QObject::destroyed
 * to determine when an object has been deleted.  You can also advise
 * it of changes using the objectModified slot directly (for example
 * where a property has no notify signal).
 *
 * ObjectMapper requires a TransactionalStore as its backing RDF
 * store, and uses the TransactionalStore's transactionCommitted
 * signal to tell it when a change has been made to the store which
 * should be mapped back to the object structure.
 *  
 * You must call the commit() method to cause any changes to be
 * written to the store.  This also commits the underlying
 * transaction.
 *
 * Call add() to add a new object to the store (managing it, and also
 * marking it to be stored on the next commit).  ObjectMapper does not
 * have any other way to find out about new objects, even if they are
 * properties or children of existing managed objects.
 *
 * Alternatively, call manage() to manage an object without marking it
 * as needing to be written -- implying that the object is known to be
 * up-to-date with the store already.  ObjectMapper will refuse to
 * manage any object that lacks a uri property, as any objects that
 * have not previously been mapped will normally need to be add()ed
 * (which gives them URIs) rather than manage()d.
 *
 * It is safe to call add() or manage() with an object that is already
 * managed; if add() is called with such an object it will be
 * rescheduled to be stored, but there will be no other effect.
 *
 * Call unmanage() to tell ObjectMapper to stop watching an object.
 * You should not call this when the object is destroyed, unless you
 * want to ensure that destroying it does not remove it from the
 * store.  Managed objects are automatically monitored for destruction
 * and removed from the store and unmanaged appropriately.
 *
 * ObjectMapper is thread-safe.
 */
class ObjectMapper : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct an object mapper backed by the given store.  The
     * mapper is initially managing no objects.
     *
     * The store must be a TransactionalStore (rather than for example
     * a BasicStore) because the object mapper commits each update as
     * a single transaction and relies on the
     * TransactionalStore::transactionCommitted signal to learn about
     * changes in the store.
     */
    ObjectMapper(TransactionalStore *ts);
    ~ObjectMapper();

    /**
     * Obtain the TransactionalStore that was passed to the
     * constructor.
     */
    TransactionalStore *getStore();

    /**
     * Supply a TypeMapping object describing the RDF URIs that should
     * be used to encode each object's property and class names.
     * Without this, ObjectMapper (or rather its ObjectStorer and
     * ObjectLoader classes) will generate suitable-looking URIs for
     * each class and property names.
     */
    void setTypeMapping(const TypeMapping &);

    /**
     * Obtain the TypeMapping previously set using setTypeMapping, or
     * the default (empty) TypeMapping if none has been set.
     */
    const TypeMapping &getTypeMapping() const;

    /**
     * Set the policy used to determine whether to give an object a
     * URI or use a blank node for it.  The default is
     * BlankNodesAsNeeded.
     */
    void setBlankNodePolicy(BlankNodePolicy policy);

    /**
     * Retrieve the current policy used to determine whether to give
     * an object a URI or use a blank node for it.
     */
    BlankNodePolicy getBlankNodePolicy() const;

    /**
     * Obtain the RDF node to which the given object has been mapped,
     * or a null Node if the object has not yet been stored by this
     * ObjectMapper.
     */
    Node getNodeForObject(QObject *o) const;

    /**
     * Obtain the QObject which has been mapped to the given node, or
     * NULL if the node is not one that has been stored by this
     * ObjectMapper.
     */
    QObject *getObjectByNode(Node n) const;

    //!!!doc
    QObject *load(Node node);

    //!!!doc
    QObjectList loadType(Uri type);
    

signals:
    void committed();

public slots:
    /**
     * Add a new object to the store.  This tells ObjectMapper to
     * manage the object, and also marks it to be stored the next time
     * \ref commit is called.
     *
     * You must call \ref add or \ref manage for every new object that
     * needs to be managed; ObjectMapper does not have any other way
     * to find out about new objects, even if they are properties or
     * children of existing managed objects.
     *!!! above is no longer correct (anything stored or loaded is managed automatically), document
     */
    void add(QObject *);

    /**
     * Add a list of new objects to the store.  This tells
     * ObjectMapper to manage the objects, and also marks them to be
     * stored the next time \ref commit is called.
     *
     * You must call \ref add or \ref manage for every new object that
     * needs to be managed; ObjectMapper does not have any other way
     * to find out about new objects, even if they are properties or
     * children of existing managed objects.
     *!!! above is no longer correct (anything stored or loaded is managed automatically), document
     */
    void add(QObjectList);

    /**
     * Tell ObjectMapper to start managing an object.  This tells
     * ObjectMapper to watch the object and commit to the store any
     * changes that it detects in the object's properties, or when the
     * object is destroyed.  If the object was managed already,
     * nothing happens.
     *
     * This does not mark the object as needing to be written; it
     * implies that the object is known to be up-to-date with the
     * store already.  ObjectMapper will refuse to manage any object
     * that lacks a uri property; if your object is a "new" one, you
     * should use \ref add instead of \ref manage.
     *!!! above is no longer enough (anything stored or loaded is managed automatically), document
     */
    void manage(QObject *);

    /**
     * Tell ObjectMapper to start managing a list of objects.  This
     * tells ObjectMapper to watch the objects and commit to the store
     * any changes that it detects in the objects' properties, or when
     * the objects are destroyed.
     *
     * Any objects that were managed already are ignored: there is no
     * harm in passing them more than once, except a small time
     * penalty.
     *
     * This does not mark the objects as needing to be written; it
     * implies that the objects are known to be up-to-date with the
     * store already.  ObjectMapper will refuse to manage any object
     * that lacks a uri property; if your object is a "new" one, you
     * should use \ref add instead of \ref manage.
     *!!! above is no longer enough (anything stored or loaded is managed automatically), document
     */
    void manage(QObjectList);

    /**
     * Tell ObjectMapper to stop managing the given object.
     */
    void unmanage(QObject *);

    /**
     * Tell ObjectMapper to stop managing the given objects.
     */
    void unmanage(QObjectList);

    /**
     * Commit to the store any changes that have happened to the
     * currently managed objects since the last commit.
     *
     * You need to call this (or commitAndObtain) if you want any
     * changes to appear in the store.
     */
    void commit();

    /**
     * Commit to the store any changes that have happened to the
     * currently managed objects since the last commit, and return the
     * changes committed.
     *
     * You need to call this (or commit) if you want any changes to
     * appear in the store.
     */
    ChangeSet commitAndObtain();

    /**
     * Notify ObjectMapper that the given object has changed.
     * ObjectMapper automatically watches the notify signals for an
     * object's properties, but it will not spot any changes that do
     * not have an associated notify signal.
     */
    void objectModified(QObject *);

    /**
     * Notify ObjectMapper that the given object is being destroyed.
     * This should not normally be necessary, as ObjectMapper
     * automatically watches the destroyed signals for objects it
     * manages.
     */
    void objectDestroyed(QObject *);

private slots:
    void transactionCommitted(const ChangeSet &cs);

private:
    ObjectMapper(const ObjectMapper &);
    ObjectMapper &operator=(const ObjectMapper &);

    class D;
    D *m_d;
};

}

#endif
