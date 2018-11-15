#ifndef UDTOKENBASE_H
#define UDTOKENBASE_H

#include <QString>
#include <QMap>

class UDTokenBase
{
public:
    UDTokenBase(const QString &form);
    UDTokenBase(const QString &form, const QString &misc);
    UDTokenBase(const QString &form, const QMap<QString, QString> &misc);
    UDTokenBase(const UDTokenBase &token);
    virtual ~UDTokenBase();

    // Fields
    QString form() const;
    void setForm(const QString &form);

    QString misc(const QString &key) const;
    void setMisc(const QString &key, const QString &value);
    QString miscToString() const;
    void setMiscFromString(const QString &misc);

    // CoNLL-U defined SpaceAfter=No feature
    bool getSpaceAfter() const;
    void setSpaceAfter(bool spaceAfter);

    // UDPipe-compatible all-spaces-preserving SpacesBefore and SpacesAfter features
    QString getSpacesBefore() const;
    void setSpacesBefore(const QString &spacesBefore);
    QString getSpacesAfter() const;
    void setSpacesAfter(const QString &spacesAfter);
    QString getSpacesInToken() const;
    void setSpacesInToken(const QString &spacesInToken);

    // UDPipe-compatible TokenRange feature
    bool getTokenRange(size_t& start, size_t& end) const;
    void setTokenRange(size_t start, size_t end);

protected:
    QString attributesToString(const QMap<QString, QString> &attributes) const;
    QMap<QString, QString> stringToAttributes(const QString &string) const;
    QString whitespaceEscape(const QString &original) const;
    QString whitespaceUnescape(const QString &escaped) const;

    QString m_form;
    QMap<QString, QString> m_misc;
};

#endif // UDTOKENBASE_H
