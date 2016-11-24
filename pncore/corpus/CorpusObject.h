#ifndef CORPUSOBJECT_H
#define CORPUSOBJECT_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QVariant>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString corpusID READ corpusID WRITE setCorpusID)

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

    // User-friendly and translated names
    static QString typeToString(CorpusObject::Type type);

    explicit CorpusObject(QObject *parent = 0);
    CorpusObject(const QString &ID, QObject *parent = 0);
    virtual ~CorpusObject() {}

    virtual QString ID() const { return m_ID; }
    virtual void setID(const QString &ID);
    virtual QString originalID() const { return m_originalID; }

    virtual QString corpusID() const { return m_corpusID; }
    virtual void setCorpusID(const QString &corpusID);

    // Subclasses (different types of corpus objects) should override this
    virtual CorpusObject::Type type() const { return CorpusObject::Type_Undefined; }

    virtual QString basePath() const { return QString(); }
    virtual QString baseMediaPath() const { return QString(); }

    bool isDirty() { return m_dirty; }
    void setDirty(bool value);
    bool isNew() { return m_new; }
    void setNew(bool value) { m_new = value; }

    // get/set properties using QString names
    QVariant property(const QString &name) const;
    bool setProperty(const QString &name, const QVariant &value);

protected:
    QString m_ID;
    QString m_originalID;
    QString m_corpusID;
    bool m_dirty;
    bool m_new;

    void copyPropertiesFrom(CorpusObject *other);

    // Monitor dynamic property changes
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void changedID(const QString &oldID, const QString &newID);

public slots:
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSOBJECT_H
