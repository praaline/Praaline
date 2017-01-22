#ifndef CORPUSOBJECT_H
#define CORPUSOBJECT_H

/*
    Praaline - Core module - Corpus metadata
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>
#include "base/ISaveable.h"

namespace Praaline {
namespace Core {

class CorpusRepository;

class PRAALINE_CORE_SHARED_EXPORT CorpusObject : public QObject, public ISaveable
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString originalID READ originalID)
    Q_PROPERTY(QString corpusID READ corpusID WRITE setCorpusID)
    Q_PROPERTY(Type type READ type)

public:
    enum Type {
        Type_Corpus,
        Type_Communication,
        Type_Speaker,
        Type_Recording,
        Type_Annotation,
        Type_Participation,
        Type_Bookmark,
        Type_Undefined
    };
    Q_ENUM(Type)

    // User-friendly and translated names
    static QString typeToString(CorpusObject::Type type);

    explicit CorpusObject(CorpusRepository *repository = 0, QObject *parent = 0);
    explicit CorpusObject(const QString &ID, CorpusRepository *repository = 0, QObject *parent = 0);
    virtual ~CorpusObject() {}

    virtual QString ID() const { return m_ID; }
    virtual void setID(const QString &ID);
    virtual QString originalID() const { return m_originalID; }

    virtual QString corpusID() const { return m_corpusID; }
    virtual void setCorpusID(const QString &corpusID);

    // Subclasses (different types of corpus objects) must override this
    virtual CorpusObject::Type type() const = 0;
    virtual bool save() = 0;

    // Get/set properties using QString names
    QVariant property(const QString &name) const;
    bool setProperty(const QString &name, const QVariant &value);

    QPointer<CorpusRepository> repository() const { return m_repository; }

signals:
    void changedID(const QString &oldID, const QString &newID);

protected:
    QString m_ID;
    QString m_originalID;
    QString m_corpusID;
    QPointer<CorpusRepository> m_repository;

    void copyPropertiesFrom(CorpusObject *other);

    // Monitor dynamic property changes
    bool eventFilter(QObject *obj, QEvent *event);

private:
    friend class CorpusDatastore;
    Q_DISABLE_COPY(CorpusObject)
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSOBJECT_H
