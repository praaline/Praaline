#include <QDebug>
#include <QXmlStreamReader>
#include "AnvilMetadataTranscript.h"

#include "pncore/corpus/Corpus.h"

namespace Praaline {
namespace Core {

QString AnvilMetadataTranscript::xmlElementName_Location("Location");
QString AnvilMetadataTranscript::xmlElementName_Contact("Contact");
QString AnvilMetadataTranscript::xmlElementName_Project("Project");
QString AnvilMetadataTranscript::xmlElementName_Content("Content");
QString AnvilMetadataTranscript::xmlElementName_Actor("Actor");
QString AnvilMetadataTranscript::xmlElementName_MediaFileResource("MediaFile");
QString AnvilMetadataTranscript::xmlElementName_WrittenResource("WrittenResource");
QString AnvilMetadataTranscript::xmlElementName_Resources("Resources");
QString AnvilMetadataTranscript::xmlElementName_Actors("Actors");
QString AnvilMetadataTranscript::xmlElementName_MDGroup("MDGroup");
QString AnvilMetadataTranscript::xmlElementName_Session("Session");

AnvilMetadataTranscript::AnvilMetadataTranscript()
{
}

// private static
AnvilMetadataTranscript::LocationData *
AnvilMetadataTranscript::readLocation(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Location)) return 0;
    LocationData *location = new LocationData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Location) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Continent")  { location->continent = xml.readElementText(); }
            else if (xml.name() == "Country")    { location->country = xml.readElementText(); }
            else if (xml.name() == "Region")     { location->region = xml.readElementText(); }
            else if (xml.name() == "Address")    { location->address = xml.readElementText(); }
        }
        xml.readNext();
    }
    return location;
}

// private static
AnvilMetadataTranscript::ContactData *
AnvilMetadataTranscript::readContact(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Contact)) return 0;
    ContactData *contact = new ContactData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Contact) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Name")          { contact->name = xml.readElementText(); }
            else if (xml.name() == "Address")       { contact->address = xml.readElementText(); }
            else if (xml.name() == "Email")         { contact->email = xml.readElementText(); }
            else if (xml.name() == "Organisation")  { contact->organisation = xml.readElementText(); }
        }
        xml.readNext();
    }
    return contact;
}

// private static
AnvilMetadataTranscript::ProjectData *
AnvilMetadataTranscript::readProject(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Project)) return 0;
    ProjectData *project = new ProjectData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Project) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Name")        { project->name = xml.readElementText(); }
            else if (xml.name() == "Title")       { project->title = xml.readElementText(); }
            else if (xml.name() == "Id")          { project->id = xml.readElementText(); }
            else if (xml.name() == "Description") { project->description = xml.readElementText(); }
            else if (xml.name() == "Contact") {
                ContactData *contact = readContact(xml);
                if (contact) project->contacts << contact;
            }
        }
        xml.readNext();
    }
    return project;
}

// private static
AnvilMetadataTranscript::ContentData *
AnvilMetadataTranscript::readContent(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Content)) return 0;
    ContentData *content = new ContentData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Content) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Genre")          { content->genre = xml.readElementText(); }
            else if (xml.name() == "SubGenre")       { content->subGenres = xml.readElementText(); }
            else if (xml.name() == "Task")           { content->task = xml.readElementText(); }
            else if (xml.name() == "Modalities")     { content->modalities = xml.readElementText(); }
            else if (xml.name() == "Subject")        { content->subjects = xml.readElementText(); }
            else if (xml.name() == "Interactivity")  { content->interactivity = xml.readElementText(); }
            else if (xml.name() == "PlanningType")   { content->planning = xml.readElementText(); }
            else if (xml.name() == "Involvement")    { content->involvement = xml.readElementText(); }
            else if (xml.name() == "SocialContext")  { content->socialContext = xml.readElementText(); }
            else if (xml.name() == "EventStructure") { content->eventStructure = xml.readElementText(); }
            else if (xml.name() == "Channel")        { content->channel = xml.readElementText(); }
        }
        xml.readNext();
    }
    return content;
}

// private static
AnvilMetadataTranscript::ActorData *
AnvilMetadataTranscript::readActor(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Actor)) return 0;
    ActorData *actor = new ActorData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Actor) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Role")              { actor->role = xml.readElementText(); }
            else if (xml.name() == "Name")              { actor->name = xml.readElementText(); }
            else if (xml.name() == "FullName")          { actor->fullName = xml.readElementText(); }
            else if (xml.name() == "Code")              { actor->code = xml.readElementText(); }
            else if (xml.name() == "FamilySocialRole")  { actor->familySocialRole = xml.readElementText(); }
            else if (xml.name() == "EthnicGroup")       { actor->ethnicGroup = xml.readElementText(); }
            else if (xml.name() == "Age")               { actor->age = xml.readElementText().toInt(); }
            else if (xml.name() == "BirthDate")         { actor->birthDate = xml.readElementText(); }
            else if (xml.name() == "Sex")               { actor->sex = xml.readElementText(); }
            else if (xml.name() == "Education")         { actor->education = xml.readElementText(); }
            else if (xml.name() == "Anonymized")        { actor->anonymized = xml.readElementText(); }
            else if (xml.name() == "Contact")           { ContactData *contact = readContact(xml);
                QStringList contactData; contactData << contact->name << contact->organisation << contact->address << contact->email;
                actor->contact = contactData.join(", ");
            }
            else if (xml.name() == "Description")       { actor->description = xml.readElementText(); }
        }
        xml.readNext();
    }
    return actor;
}

// private static
AnvilMetadataTranscript::MediaFileResourceData *
AnvilMetadataTranscript::readMediaFileResource(QXmlStreamReader &xml)
{
    xml.readNext();
    return 0;
}

// private static
AnvilMetadataTranscript::WrittenResourceData *
AnvilMetadataTranscript::readWrittenResource(QXmlStreamReader &xml)
{
    xml.readNext();
    return 0;
}

// private static
AnvilMetadataTranscript::SessionData *
AnvilMetadataTranscript::readSession(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Session)) return 0;
    SessionData *session = new SessionData();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Session) && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if      (xml.name() == "Name")           { session->name = xml.readElementText(); }
            else if (xml.name() == "Title")          { session->title = xml.readElementText(); }
            else if (xml.name() == "Date")           { session->date = xml.readElementText(); }
            else if (xml.name() == "Descripiton")    { session->description = xml.readElementText(); }
            else if (xml.name() == xmlElementName_MDGroup) {
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_MDGroup) && !xml.atEnd()) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == xmlElementName_Location) {
                            LocationData *location = readLocation(xml);
                            if (location) session->locations << location;
                        }
                        else if (xml.name() == xmlElementName_Project) {
                            ProjectData *project = readProject(xml);
                            if (project) session->projects << project;
                        }
                        else if (xml.name() == xmlElementName_Content) {
                             ContentData *content = readContent(xml);
                            if (content) session->contents << content;
                        }
                        else if (xml.name() == xmlElementName_Actors) {
                            xml.readNext();
                            while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Actors) && !xml.atEnd()) {
                                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                                    if (xml.name() == xmlElementName_Actor) {
                                        ActorData *actor = readActor(xml);
                                        if (actor) session->actors << actor;
                                    }
                                }
                                xml.readNext();
                            }
                        } // Actors
                    }
                    xml.readNext();
                }
            } // /MDGroup
            else if (xml.name() == xmlElementName_Resources) {
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Resources) && !xml.atEnd()) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {

                    }
                    xml.readNext();
                }
            } // /Resources
        }
        xml.readNext();
    }
    return session;
}

// static
bool AnvilMetadataTranscript::load(const QString &filename, Corpus *corpus)
{
    SessionData *session = 0;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Session") {
            session = readSession(xml);
            file.close();
        }
        xml.readNext(); // next element
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();

    // Update / create corpus items
    if (!session) return false;
    QString communicationID = QString(session->name).remove("-meta");
    QPointer<CorpusCommunication> com = corpus->communication(communicationID);
    if (!com) return false;
    com->setProperty("title", session->title);
    com->setProperty("dateRecorded", session->date);
    com->setProperty("description", session->description);
    foreach (ContentData *content, session->contents) {
        com->setProperty("genre", content->genre);
        com->setProperty("subgenres", content->subGenres);
        com->setProperty("task", content->task);
        com->setProperty("modalities", content->modalities);
        com->setProperty("subjects", content->subjects);
        com->setProperty("interactivity", content->interactivity);
        com->setProperty("planning", content->planning);
        com->setProperty("involvement", content->involvement);
        com->setProperty("socialContext", content->socialContext);
        com->setProperty("eventStructure", content->eventStructure);
        com->setProperty("channel", content->channel);
    }
    int i = 1;
    foreach (ActorData *actor, session->actors) {
        QString speakerID = QString("%1_$L%2").arg(communicationID).arg(i);
        i++;
        QPointer<CorpusSpeaker> spk = corpus->speaker(speakerID);
        if (!spk) continue;
        spk->setProperty("age", actor->age);
        spk->setProperty("anonymized", actor->anonymized);
        spk->setProperty("birthDate", actor->birthDate);
        spk->setProperty("code", actor->code);
        spk->setProperty("contact", actor->contact);
        spk->setProperty("description", actor->description);
        spk->setProperty("education", actor->education);
        spk->setProperty("ethnicGroup", actor->ethnicGroup);
        spk->setProperty("familySocialRole", actor->familySocialRole);
        spk->setProperty("fullName", actor->fullName);
        spk->setProperty("name", actor->name);
        spk->setProperty("sex", actor->sex);

        QPointer<CorpusParticipation> participation = corpus->participation(communicationID, speakerID);
        if (participation) {
            participation->setProperty("role", actor->role);
        }
    }

    return true;
}

// static
bool AnvilMetadataTranscript::save(const QString &filename, Corpus *corpus)
{
    Q_UNUSED(filename)
    Q_UNUSED(corpus)
    return false;
}

} // namespace Core
} // namespace Praaline
