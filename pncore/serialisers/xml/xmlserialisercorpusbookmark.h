#ifndef XMLSERIALISERCORPUSBOOKMARK_H
#define XMLSERIALISERCORPUSBOOKMARK_H

#include <QObject>
#include <QPointer>
#include <QString>
#include "xmlserialiserbase.h"
#include "corpus/corpusbookmark.h"

class PRAALINE_CORE_SHARED_EXPORT XMLSerialiserCorpusBookmark : XMLSerialiserBase
{
public:
    static bool saveCorpusBookmarks(const QList<QPointer<CorpusBookmark> > &list, const QString &filename);
    static bool saveCorpusBookmarks(const QList<QPointer<CorpusBookmark> > &list, QXmlStreamWriter &xml);
    static bool loadCorpusBookmarks(QList<QPointer<CorpusBookmark> > &list, const QString &filename);
    static bool loadCorpusBookmarks(QList<QPointer<CorpusBookmark> > &list, QXmlStreamReader &xml);

private:
    XMLSerialiserCorpusBookmark();
    ~XMLSerialiserCorpusBookmark();

    static CorpusBookmark *readBookmark(QXmlStreamReader &xml);
    static bool writeBookmark(CorpusBookmark *bookmark, QXmlStreamWriter &xml);

    static QString xmlElementName_CorpusBookmark;
};

#endif // XMLSERIALISERCORPUSBOOKMARK_H
