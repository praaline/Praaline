#ifndef ANVILMETADATATRANSCRIPT_H
#define ANVILMETADATATRANSCRIPT_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QString>
#include "base/RealTime.h"
#include "corpus/CorpusSpeaker.h"
#include "interfaces/InterfaceTextFile.h"

class QXmlStreamReader;

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT AnvilMetadataTranscript : InterfaceTextFile
{
public:
    // Public static methods to read and write Transcriber Annotation Graphs in XML format
    static bool load(const QString &filename, Corpus *corpus);
    static bool save(const QString &filename, Corpus *corpus);

private:
    struct LocationData {
        QString continent;
        QString country;
        QString region;
        QString address;
    };

    struct ContactData {
        QString name;
        QString address;
        QString email;
        QString organisation;
    };

    struct ProjectData {
        QString name;
        QString title;
        QString id;
        QString description;
        QList<ContactData *> contacts;
    };

    struct ContentData {
        QString genre;
        QString subGenres;  // list
        QString task;
        QString modalities; // list
        QString subjects;   // list
        QString interactivity;
        QString planning;
        QString involvement;
        QString socialContext;
        QString eventStructure;
        QString channel;
        QString languages; // list
        QStringList keys;
        QString description;
    };

    struct ActorData {
        QString role;
        QString name;
        QString fullName;
        QString code;
        QString familySocialRole; // list
        QString languages;
        QString ethnicGroup;
        int age;
        QVariant birthDate;
        QString sex;
        QString education;
        QString anonymized;
        QString contact; // contact data
        QStringList keys;
        QString description;
    };

    struct MediaFileResourceData {
        QString type;
    };

    struct WrittenResourceData {
        QString type;
    };

    struct SessionData {
        QString name;
        QString title;
        QVariant date;
        QString description;
        // Metadata group
        QList<LocationData *> locations;
        QList<ProjectData *> projects;
        QStringList keys;
        QList<ContentData *> contents;
        QList<ActorData *> actors;
        // Resources
        QList<MediaFileResourceData *> mediaFiles;
        QList<WrittenResourceData *> writtenResources;
    };

    AnvilMetadataTranscript();

    static LocationData             *readLocation(QXmlStreamReader &xml);
    static ContactData              *readContact(QXmlStreamReader &xml);
    static ProjectData              *readProject(QXmlStreamReader &xml);
    static ContentData              *readContent(QXmlStreamReader &xml);
    static ActorData                *readActor(QXmlStreamReader &xml);
    static MediaFileResourceData    *readMediaFileResource(QXmlStreamReader &xml);
    static WrittenResourceData      *readWrittenResource(QXmlStreamReader &xml);
    static SessionData              *readSession(QXmlStreamReader &xml);

    // XML element names
    static QString xmlElementName_Location;
    static QString xmlElementName_Contact;
    static QString xmlElementName_Project;
    static QString xmlElementName_Content;
    static QString xmlElementName_Actor;
    static QString xmlElementName_MediaFileResource;
    static QString xmlElementName_WrittenResource;
    static QString xmlElementName_Resources;
    static QString xmlElementName_Actors;
    static QString xmlElementName_MDGroup;
    static QString xmlElementName_Session;
};

} // namespace Core
} // namespace Praaline

#endif // ANVILMETADATATRANSCRIPT_H
