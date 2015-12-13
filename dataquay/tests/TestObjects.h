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

#ifndef _TEST_OBJECTS_H_
#define _TEST_OBJECTS_H_

#include <QObject>
#include <QMetaType>
#include <QStringList>
#include <QSet>

class A : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject *ref READ getRef WRITE setRef STORED true)

public:
    A(QObject *parent = 0) : QObject(parent), m_ref(0) { }

    QObject *getRef() const { return m_ref; }
    void setRef(QObject *r) { m_ref = r; }

private:
    QObject *m_ref;
};

Q_DECLARE_METATYPE(A*)

class B : public QObject
{
    Q_OBJECT

    Q_PROPERTY(A *aref READ getA WRITE setA STORED true)

public:
    B(QObject *parent = 0) : QObject(parent), m_a(0) { }

    A *getA() const { return m_a; }
    void setA(A *r) { m_a = qobject_cast<A *>(r); }

private:
    A *m_a;
};

Q_DECLARE_METATYPE(B*)

class C : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString string READ getString WRITE setString NOTIFY stringChanged STORED true)
    Q_PROPERTY(QStringList strings READ getStrings WRITE setStrings NOTIFY stringsChanged STORED true)
    Q_PROPERTY(QList<float> floats READ getFloats WRITE setFloats STORED true)
    Q_PROPERTY(QList<B *> bees READ getBees WRITE setBees STORED true)
    Q_PROPERTY(QSet<C *> cees READ getCees WRITE setCees STORED true)
    Q_PROPERTY(QObjectList objects READ getObjects WRITE setObjects STORED true)

public:
    C(QObject *parent = 0) : QObject(parent) { }

    QString getString() const { return m_string; }
    void setString(QString s) { m_string = s; emit stringChanged(s); }

    QStringList getStrings() const { return m_strings; }
    void setStrings(QStringList sl) { m_strings = sl; emit stringsChanged(sl); }

    QList<float> getFloats() const { return m_floats; }
    void setFloats(QList<float> fl) { m_floats = fl; }

    QList<B *> getBees() const { return m_bees; }
    void setBees(QList<B *> bl) { m_bees = bl; }

    QSet<C *> getCees() const { return m_cees; }
    void setCees(QSet<C *> cl) { m_cees = cl; }

    QObjectList getObjects() const { return m_objects; }
    void setObjects(QObjectList ol) { m_objects = ol; }

signals:
    void stringChanged(QString s);
    void stringsChanged(QStringList sl);

private:
    QString m_string;
    QStringList m_strings;
    QList<float> m_floats;
    QList<B *> m_bees;
    QSet<C *> m_cees;
    QObjectList m_objects;
};

Q_DECLARE_METATYPE(C*)
Q_DECLARE_METATYPE(QList<float>)
Q_DECLARE_METATYPE(QObjectList)
Q_DECLARE_METATYPE(QList<A*>)
Q_DECLARE_METATYPE(QList<B*>)
Q_DECLARE_METATYPE(QList<C*>)
Q_DECLARE_METATYPE(QSet<C*>)

/* StreamableValueType is a type that can be streamed to QDataStream
 * and thus converted automatically to QVariant, but that will not be
 * registered with a Node encoder -- so Node must use an "unknown
 * type" datatype
 */
enum StreamableValueType { ValueA = 0, ValueB, ValueC, ValueD, ValueE };

/* NonStreamableValueType is a type that cannot be streamed to
 * QDataStream, but that we will register with a Node encoder -- so
 * Node can give it the proper datatype
 */
enum NonStreamableValueType { ValueF = 0, ValueG, ValueH, ValueI, ValueJ };

extern QDataStream &operator<<(QDataStream &out, StreamableValueType v);
extern QDataStream &operator>>(QDataStream &in, StreamableValueType &v);
    
Q_DECLARE_METATYPE(StreamableValueType)
Q_DECLARE_METATYPE(NonStreamableValueType)

#endif
