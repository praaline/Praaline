#include <QString>
#include <QMap>
#include "UDTokenBase.h"

// protected
QString UDTokenBase::attributesToString(const QMap<QString, QString> &attributes) const
{
    QString ret;
    foreach (QString key, attributes.keys()) {
        ret.append(QString("%1=%2|").arg(key, m_misc.value(key)));
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

// protected
QMap<QString, QString> UDTokenBase::stringToAttributes(const QString &string) const
{
    QMap<QString, QString> attributes;
    foreach (QString pair, string.split("|")) {
        if (!pair.contains("=")) continue;
        QString key = pair.section("=", 0, 0);
        QString value = pair.section("=", 1, 1);
        attributes.insert(key, value);
    }
    return attributes;
}

// ====================================================================================================================
// Constructors / Destructor
// ====================================================================================================================

UDTokenBase::UDTokenBase(const QString &form) :
    m_form(form)
{
}

UDTokenBase::UDTokenBase(const QString &form, const QString &misc) :
    m_form(form)
{
    m_misc = stringToAttributes(misc);
}

UDTokenBase::UDTokenBase(const QString &form, const QMap<QString, QString> &misc) :
    m_form(form), m_misc(misc)
{
}

UDTokenBase::UDTokenBase(const UDTokenBase &token) :
    m_form(token.m_form), m_misc(token.m_misc)
{
}

UDTokenBase::~UDTokenBase()
{
}

// ====================================================================================================================
// Fields
// ====================================================================================================================

QString UDTokenBase::form() const
{
    return m_form;
}

void UDTokenBase::setForm(const QString &form)
{
    m_form = form;
}

QString UDTokenBase::misc(const QString &key) const
{
    return m_misc.value(key);
}

void UDTokenBase::setMisc(const QString &key, const QString &value)
{
    m_misc.insert(key, value);
}

QString UDTokenBase::miscToString() const
{
    return attributesToString(m_misc);
}

void UDTokenBase::setMiscFromString(const QString &misc)
{
    m_misc = stringToAttributes(misc);
}

// ====================================================================================================================
// CoNLL-U defined SpaceAfter=No feature
// ====================================================================================================================

bool UDTokenBase::getSpaceAfter() const
{
    if (m_misc.value("SpaceAfter") == "No") return false;
    return true;
}

void UDTokenBase::setSpaceAfter(bool spaceAfter)
{
    if (spaceAfter)
        // The default is to have one space after the token, when the SpaceAfter attribute is not present
        m_misc.remove("SpaceAfter");
    else
        m_misc.insert("SpaceAfter", "No");
}

// Whitespace espcaping
QString UDTokenBase::whitespaceEscape(const QString &original) const
{
    return QString(original)
            .replace(" ", "\\s").replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n")
            .replace("|", "\\p").replace("\\", "\\\\");
}

QString UDTokenBase::whitespaceUnescape(const QString &escaped) const
{
    return QString(escaped)
            .replace("\\s", " ").replace("\\t", "\t").replace("\\r", "\r").replace("\\n", "\n")
            .replace("\\p", "|").replace("\\\\", "\\");
}

// ====================================================================================================================
// UDPipe-compatible all-spaces-preserving SpacesBefore and SpacesAfter features
// ====================================================================================================================

QString UDTokenBase::getSpacesBefore() const
{
    if (m_misc.contains("SpacesBefore"))
        return whitespaceUnescape(m_misc.value("SpacesBefore"));
    return "";
}

void UDTokenBase::setSpacesBefore(const QString &spacesBefore)
{
    m_misc.insert("SpacesBefore", whitespaceEscape(spacesBefore));
}

QString UDTokenBase::getSpacesAfter() const
{
    if (m_misc.contains("SpacesAfter"))
        return whitespaceUnescape(m_misc.value("SpacesAfter"));
    if (misc("SpaceAfter") == "No")
        return "";
    return " ";
}

void UDTokenBase::setSpacesAfter(const QString &spacesAfter)
{
    if (spacesAfter.isEmpty()) {
        m_misc.remove("SpacesAfter");
        m_misc.insert("SpaceAfter", "No");
    } else {
        m_misc.remove("SpaceAfter");
        m_misc.insert("SpacesAfter", whitespaceEscape(spacesAfter));
    }
}

QString UDTokenBase::getSpacesInToken() const
{
    if (m_misc.contains("SpacesInToken"))
        return whitespaceUnescape(m_misc.value("SpacesInToken"));
    return "";
}

void UDTokenBase::setSpacesInToken(const QString &spacesInToken)
{
    m_misc.insert("SpacesInToken", whitespaceEscape(spacesInToken));
}

// ====================================================================================================================
// UDPipe-compatible TokenRange feature
// ====================================================================================================================

bool UDTokenBase::getTokenRange(size_t& start, size_t& end) const
{
    if (m_misc.contains("TokenRange")) {
        QString range = m_misc.value("TokenRange");
        if (range.contains(":")) {
            bool ok;
            start = range.section(":", 0, 0).toULongLong(&ok);
            if (!ok) return false;
            end = range.section(":", 1, 1).toULongLong(&ok);
            if (!ok) return false;
            return true;
        }
        else return false;
    }
    return false;
}

void UDTokenBase::setTokenRange(size_t start, size_t end)
{
    QString range = QString("%1:%2").arg(start).arg(end);
    m_misc.insert("TokenRange", range);
}


