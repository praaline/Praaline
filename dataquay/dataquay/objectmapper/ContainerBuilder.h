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

#ifndef _DATAQUAY_CONTAINER_BUILDER_H_
#define _DATAQUAY_CONTAINER_BUILDER_H_

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace Dataquay {

/**
 * \class ContainerBuilder ContainerBuilder.h <dataquay/objectmapper/ContainerBuilder.h>
 *
 * ContainerBuilder is a utility class which assists with storage of
 * arbitrary container objects into variant objects and subsequently
 * retrieving them into lists.
 *
 * Given types T and Container (where Container is typically something
 * like QList<T>), ContainerBuilder can convert a QVariantList holding
 * type T into a QVariant holding Container, and back again.
 */
class ContainerBuilder
{
public:
    /**
     * Retrieve the single global instance of ContainerBuilder.
     */
    static ContainerBuilder *getInstance();

    /**
     * ContainerKind describes the sort of behaviour a container
     * displays with regard to ordering and structure.  A container of
     * kind SequenceKind is externally ordered (order must be
     * preserved when serialising, for example); kind SetKind is
     * unordered or internally ordered (its ordering does not need to
     * be described).
     */
    enum ContainerKind {
        UnknownKind = 0,
        SequenceKind,   // e.g. list
        SetKind         // e.g. set
        // perhaps also something for maps
    };

    /**
     * Register Container as a container of kind ContainerKind holding
     * type T.  T must be something capable of being stored in a
     * QVariant -- so typically a primitive type, a Qt utility type
     * such as QString, or a pointer type.  Both T and Container must
     * be registered as Qt metatypes using qRegisterMetaType.
     * 
     * The arguments typeName and containerName give the textual names
     * for the types T and Container; these must be the same as were
     * used when calling qRegisterMetaType.
     *
     * For example, to register QStringList:
     *
     * registerContainer<QString, QStringList>
     *     ("QString", "QStringList", SequenceKind);
     */
    template <typename T, typename Container>
    void registerContainer(QString typeName, QString containerName,
                           ContainerKind kind) {
        registerContainerExtractor<T, Container>
            (typeName, containerName, kind);
    }

    /**
     * Return true if the container named containerName can be
     * extracted from a variant.  This is the case if containerName
     * was the name of a container provided to an earlier call to
     * registerContainer.
     */
    bool canExtractContainer(QString containerName) {
        return m_containerExtractors.contains(containerName);
    }

    /**
     * Return true if the container named containerName can be
     * injected into a variant.  This is the case if containerName was
     * the name of a container provided to an earlier call to
     * registerContainer.
     */
    bool canInjectContainer(QString containerName) {
        return m_containerExtractors.contains(containerName);
    }

    /**
     * Return the typeName that is associated with the given
     * containerName.  That is, when registerContainer was called for
     * a container with name containerName, typeName is the name that
     * was given as the first argument of that call.
     */
    QString getTypeNameForContainer(QString containerName) {
        if (!canExtractContainer(containerName)) return QString();
        return m_containerExtractors[containerName]->getTypeName();
    }

    /**
     * Return the kind of the container with the given containerName.
     * That is, when registerContainer was called for a container with
     * name containerName, return the kind that was given as the final
     * argument of that call.
     */
    ContainerKind getContainerKind(QString containerName) {
        if (!canExtractContainer(containerName)) return UnknownKind;
        return m_containerExtractors[containerName]->getKind();
    }

    /**
     * Extract the named container type from the given variant object
     * (which must hold that container type) and return a list of
     * variants containing the individual elements in the container.
     */
    QVariantList extractContainer(QString containerName, const QVariant &v) {
        if (!canExtractContainer(containerName)) return QVariantList();
        return m_containerExtractors[containerName]->extract(v);
    }

    /**
     * Inject the named container type into a new variant object.
     * Taking a list of variants holding the individual elements in
     * the container, return a single variant holding the container
     * itself.
     */
    QVariant injectContainer(QString containerName, const QVariantList &vl) {
        if (!canInjectContainer(containerName)) return QVariant();
        return m_containerExtractors[containerName]->inject(vl);
    }

private:
    ContainerBuilder() {
        registerContainer<QString, QStringList>
            ("QString", "QStringList", SequenceKind);
    }
    ~ContainerBuilder() {
        for (ContainerExtractorMap::iterator i = m_containerExtractors.begin();
             i != m_containerExtractors.end(); ++i) {
            delete *i;
        }
    }

    template <typename T, typename Container>
    void
    registerContainerExtractor(QString typeName, QString containerName,
                               ContainerKind kind) {
        m_containerExtractors[containerName] =
            new ContainerExtractor<T, Container>(typeName, kind);
    }

    struct ContainerExtractorBase {
        virtual ~ContainerExtractorBase() { }
        virtual QVariantList extract(const QVariant &v) = 0;
        virtual QVariant inject(const QVariantList &) = 0;
        virtual QString getTypeName() const = 0;
        virtual ContainerKind getKind() const = 0;
    };

    template <typename T, typename Container>
    struct ContainerExtractor : public ContainerExtractorBase
    {
        ContainerExtractor(QString typeName, ContainerKind kind) :
            m_typeName(typeName), m_kind(kind) { }

        virtual QVariantList extract(const QVariant &v) {
            Container tl = v.value<Container>();
            QVariantList vl;
            foreach (const T &t, tl) vl << QVariant::fromValue<T>(t);
            return vl;
        }
        virtual QVariant inject(const QVariantList &vl) {
            Container tl;
            foreach (const QVariant &v, vl) tl << v.value<T>();
            return QVariant::fromValue<Container>(tl);
        }
        virtual QString getTypeName() const {
            return m_typeName;
        }
        virtual ContainerKind getKind() const {
            return m_kind;
        }

        QString m_typeName;
        ContainerKind m_kind;
    };

    typedef QHash<QString, ContainerExtractorBase *> ContainerExtractorMap;
    ContainerExtractorMap m_containerExtractors;
};

}

#endif
