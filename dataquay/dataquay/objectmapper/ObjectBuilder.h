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

#ifndef _DATAQUAY_OBJECT_BUILDER_H_
#define _DATAQUAY_OBJECT_BUILDER_H_

#include <QHash>
#include <QString>
#include <QVariant>

namespace Dataquay {

/**
 * \class ObjectBuilder ObjectBuilder.h <dataquay/objectmapper/ObjectBuilder.h>
 *
 * ObjectBuilder is a singleton object factory capable of constructing
 * new objects of classes that are subclassed from QObject.  Given the
 * class name as a string, and optionally a parent object, it will
 * return a new instance of the class.  To be capable of construction
 * using ObjectBuilder, a class must be declared using Q_OBJECT as
 * well as subclassed from QObject.
 *
 * All candidate object classes need to be registered with the builder
 * before they can be constructed.  The only class that ObjectBuilder
 * is able to construct without registration is QObject itself.
 *
 * This class permits code to construct new objects dynamically,
 * without needing to know anything about them except for their class
 * names, and without needing their definitions to be visible.  (The
 * definitions must be visible when the object classes are registered,
 * but not when the objects are constructed.)
 */
class ObjectBuilder
{
public:
    /**
     * Retrieve the single global instance of ObjectBuilder.
     */
    static ObjectBuilder *getInstance();

    /**
     * Register type T, a subclass of QObject, as a class that can be
     * constructed by calling a zero-argument constructor.
     *
     * For example, registerClass<QAction>() declares that QAction is
     * a subclass of QObject that may be built by calling
     * QAction::QAction().
     * 
     * A subsequent call to ObjectBuilder::build("QAction") would
     * return a new QAction built with that constructor (since
     * "QAction" is the class name of QAction returned by its meta
     * object).
     */
    template <typename T>
    void registerClass() {
        m_builders[T::staticMetaObject.className()] = new Builder0<T>();
    }

    /**
     * Register type T, a subclass of QObject, as a class that can be
     * constructed by calling a single-argument constructor whose
     * argument is of pointer-to-Parent type, where Parent is also a
     * subclass of QObject.
     *
     * For example, registerClass<QWidget, QWidget>() declares that
     * QWidget is a subclass of QObject that may be built by calling
     * QWidget::QWidget(QWidget *parent).
     * 
     * A subsequent call to ObjectBuilder::build("QWidget", parent)
     * would return a new QWidget built with that constructor (since
     * "QWidget" is the class name of QWidget returned by its meta
     * object).
     */
    template <typename T, typename Parent>
    void registerClass() {
        m_builders[T::staticMetaObject.className()] = new Builder1<T, Parent>();
    }

    /**
     * Register type T, a subclass of QObject, as a class that can be
     * constructed by calling a zero-argument constructor.  Also
     * declare pointerName to be the meta type name for pointers to
     * type T, such that QVariant can be used to store such pointers.
     *
     * For example, registerClass<QAction>("QAction*") declares that
     * QAction is a subclass of QObject that may be built by calling
     * QAction::QAction(), and that "QAction*" has been registered
     * (using qRegisterMetaType) as the meta type name for
     * pointer-to-QAction.
     * 
     * A subsequent call to ObjectBuilder::build("QAction") would
     * return a new QAction built with that constructor (since
     * "QAction" is the class name of QAction returned by its meta
     * object).
     */
    template <typename T>
    void registerClass(QString pointerName) {
        QString className = T::staticMetaObject.className();
        m_cpmap[className] = pointerName;
        m_pcmap[pointerName] = className;
        m_builders[className] = new Builder0<T>();
        registerExtractor<T>(pointerName);
    }

    /**
     * Register type T, a subclass of QObject, as a class that can be
     * constructed by calling a single-argument constructor whose
     * argument is of pointer-to-Parent type, where Parent is also a
     * subclass of QObject.  Also declare pointerName to be the meta
     * type name for pointers to type T, such that QVariant can be
     * used to store such pointers.
     *
     * For example, registerClass<QWidget, QWidget>("QWidget*")
     * declares that QWidget is a subclass of QObject that may be
     * built by calling QWidget::QWidget(QWidget *parent), and that
     * "QWidget*" has been registered (using qRegisterMetaType) as the
     * meta type name for pointer-to-QWidget.
     * 
     * A subsequent call to ObjectBuilder::build("QWidget", parent)
     * would return a new QWidget built with that constructor (since
     * "QWidget" is the class name of QWidget returned by its meta
     * object).
     */
    template <typename T, typename Parent>
    void registerClass(QString pointerName) {
        QString className = T::staticMetaObject.className();
        m_cpmap[className] = pointerName;
        m_pcmap[pointerName] = className;
        m_builders[className] = new Builder1<T, Parent>();
        registerExtractor<T>(pointerName);
    }

    /**
     * Register type T, a subclass of QObject, as an interface (a pure
     * virtual class) and pointerName to be the meta type name for
     * pointers to type T, such that QVariant can be used to store
     * such pointers.
     *
     * For example, registerClass<Command>("Command*") declares that
     * Command is a subclass of QObject that may not be built directly
     * but that "Command*" has been registered (using
     * qRegisterMetaType) as the meta type name for
     * pointer-to-QAction.
     *
     * A subsequent call to ObjectBuilder::extract("Command*", v)
     * would extract a pointer of type Command* from the QVariant v.
     */
    template <typename T>
    void registerInterface(QString pointerName) {
        QString className = T::staticMetaObject.className();
        m_cpmap[className] = pointerName;
        m_pcmap[pointerName] = className;
        registerExtractor<T>(pointerName);
    }

    /**
     * Return true if the class whose class name (according to its
     * meta object) is className has been registered for building.
     */
    bool knows(QString className) {
        return m_builders.contains(className);
    }

    /**
     * Return a new object whose class name (according to its meta
     * object) is className, with the given parent (cast
     * appropriately) passed to its single argument constructor.
     */
    QObject *build(QString className, QObject *parent) {
        if (!knows(className)) return 0;
        return m_builders[className]->build(parent);
    }

    /**
     * Return a new object whose class name (according to its meta
     * object) is className, constructed with no parent.
     */
    QObject *build(QString className) {
        if (!knows(className)) return 0;
        return m_builders[className]->build(0);
    }

    /**
     * Return true if the class whose pointer has meta-type name
     * pointerName has been registered with that pointer name
     * (i.e. using one of the registerClass(pointerName) methods or
     * registerInterface).
     */
    bool canExtract(QString pointerName) {
        return m_extractors.contains(pointerName);
    }

    /**
     * Return true if the class whose pointer has meta-type name
     * pointerName has been registered with that pointer name
     * (i.e. using one of the registerClass(pointerName) methods or
     * registerInterface).
     */
    bool canInject(QString pointerName) {
        return m_extractors.contains(pointerName);
    }

    /**
     * Provided the given pointerName has been registered using one of
     * the registerClass(pointerName) methods or registerInterface,
     * take the given variant containing that pointer type and extract
     * and return the pointer.
     */
    QObject *extract(QString pointerName, QVariant &v) {
        if (!canExtract(pointerName)) return 0;
        return m_extractors[pointerName]->extract(v);
    }

    /**
     * Provided the given pointerName has been registered using one of
     * the registerClass(pointerName) methods or registerInterface,
     * take the given pointer and stuff it into a variant, returning
     * the result.
     */
    QVariant inject(QString pointerName, QObject *p) {
        if (!canInject(pointerName)) return QVariant();
        return m_extractors[pointerName]->inject(p);
    }

    /**
     * Provided the given pointerName has been registered using one of
     * the registerClass(pointerName) methods, return the name of the
     * class that was used as the template argument for that method.
     */
    QString getClassNameForPointerName(QString pointerName) const {
        if (m_pcmap.contains(pointerName)) return m_pcmap[pointerName];
        return "";
    }

    /**
     * If the class whose class name (according to its meta object) is
     * className has been registered using one of the
     * registerClass(pointerName) methods, return the pointerName that
     * was passed to that method.
     */
    QString getPointerNameForClassName(QString className) const {
        if (m_cpmap.contains(className)) return m_cpmap[className];
        return "";
    }

private:
    ObjectBuilder() {
        registerClass<QObject, QObject>("QObject*");
    }
    ~ObjectBuilder() {
        for (BuilderMap::iterator i = m_builders.begin();
             i != m_builders.end(); ++i) {
            delete *i;
        }
        for (ExtractorMap::iterator i = m_extractors.begin();
             i != m_extractors.end(); ++i) {
            delete *i;
        }
    }

    template <typename T>
    void
    registerExtractor(QString pointerName) {
        m_extractors[pointerName] = new Extractor<T *>();
    }

    template <typename T>
    void
    registerExtractor(QString pointerName, QString listName) {
        m_extractors[pointerName] = new Extractor<T *>();
    }

    struct BuilderBase {
        virtual ~BuilderBase() { }
        virtual QObject *build(QObject *) = 0;
    };

    template <typename T> struct Builder0 : public BuilderBase {
        virtual QObject *build(QObject *) {
            return new T();
        }
    };

    template <typename T, typename Parent> struct Builder1 : public BuilderBase {
        virtual QObject *build(QObject *p) {
            return new T(qobject_cast<Parent *>(p));
        }
    };

    typedef QHash<QString, BuilderBase *> BuilderMap;
    BuilderMap m_builders;

    struct ExtractorBase {
        virtual ~ExtractorBase() { }
        virtual QObject *extract(const QVariant &v) = 0;
        virtual QVariant inject(QObject *) = 0;
    };

    template <typename Pointer> struct Extractor : public ExtractorBase {
        virtual QObject *extract(const QVariant &v) {
            return v.value<Pointer>();
        }
        virtual QVariant inject(QObject *o) {
            Pointer p = qobject_cast<Pointer>(o);
            if (p) return QVariant::fromValue<Pointer>(p);
            else return QVariant();
        }
    };

    typedef QHash<QString, ExtractorBase *> ExtractorMap;
    ExtractorMap m_extractors;

    QHash<QString, QString> m_cpmap;
    QHash<QString, QString> m_pcmap;
};

}

#endif
