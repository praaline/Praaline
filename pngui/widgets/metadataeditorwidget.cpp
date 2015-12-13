#include <QPointer>
#include <QString>
#include <QMap>
#include <QPair>
#include <QDate>
#include <QGridLayout>

#include "corpus/corpusparticipation.h"

// Qt property browser
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"

#include "metadataeditorwidget.h"


MetadataEditorWidget::MetadataEditorWidget(QWidget *parent) :
    QWidget(parent)
{
    // Managers
    m_stringManager = new QtStringPropertyManager(this);
    m_boolManager = new QtBoolPropertyManager(this);
    m_intManager = new QtIntPropertyManager(this);
    m_doubleManager = new QtDoublePropertyManager(this);
    m_enumManager = new QtEnumPropertyManager(this);
    m_dateManager = new QtDatePropertyManager(this);
    m_groupManager = new QtGroupPropertyManager(this);

    // Signals and slots
    connect(m_stringManager, SIGNAL(valueChanged(QtProperty*, const QString &)),
            this, SLOT(valueChanged(QtProperty *, const QString &)));
    connect(m_boolManager, SIGNAL(valueChanged(QtProperty*, bool)),
            this, SLOT(valueChanged(QtProperty *, bool)));
    connect(m_intManager, SIGNAL(valueChanged(QtProperty*, int)),
            this, SLOT(valueChanged(QtProperty *, int)));
    connect(m_doubleManager, SIGNAL(valueChanged(QtProperty *, double)),
            this, SLOT(valueChanged(QtProperty *, double)));
    connect(m_enumManager, SIGNAL(valueChanged(QtProperty*, int)),
            this, SLOT(valueChanged(QtProperty *, int)));
    connect(m_dateManager, SIGNAL(valueChanged(QtProperty*, const QDate &)),
            this, SLOT(valueChanged(QtProperty *, const QDate &)));

    QtDoubleSpinBoxFactory *doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
    QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);
    QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(this);
    QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);
    QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(this);
    QtDateEditFactory *dateEditFactory = new QtDateEditFactory(this);

    m_propertyEditor = new QtTreePropertyBrowser(this);
    m_propertyEditor->setFactoryForManager(m_stringManager, lineEditFactory);
    m_propertyEditor->setFactoryForManager(m_boolManager, checkBoxFactory);
    m_propertyEditor->setFactoryForManager(m_intManager, spinBoxFactory);
    m_propertyEditor->setFactoryForManager(m_doubleManager, doubleSpinBoxFactory);
    m_propertyEditor->setFactoryForManager(m_dateManager, dateEditFactory);
    m_propertyEditor->setFactoryForManager(m_enumManager, comboBoxFactory);

    m_propertyEditor->setResizeMode(QtTreePropertyBrowser::ResizeToContents);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(m_propertyEditor);
    this->setLayout(layout);
}

MetadataEditorWidget::~MetadataEditorWidget()
{
}

void MetadataEditorWidget::addProperty(QtProperty *property, CorpusObject::Type type, const QString &itemID, const QString &attributeID, QtProperty *group)
{
    m_propertyToId[property] = PropertyID(type, itemID, attributeID);
    m_idToProperty[PropertyID(type, itemID, attributeID)] = property;
    if (!group)
        m_propertyEditor->addProperty(property);
    else
        group->addSubProperty(property);
}

QtProperty *MetadataEditorWidget::addProperties(QPointer<MetadataStructure> mstructure, QPointer<CorpusObject> item,
                                                QtProperty *under)
{
    if (!mstructure) return 0;
    if (!item) return 0;

    QtProperty *property;
    QString itemID = item->ID();
    QString itemType;
    switch (item->type()) {
    case CorpusObject::Type_Corpus : itemType = "Corpus"; break;
    case CorpusObject::Type_Communication : itemType = "Communication"; break;
    case CorpusObject::Type_Speaker : itemType = "Speaker"; break;
    case CorpusObject::Type_Recording : itemType = "Recording"; break;
    case CorpusObject::Type_Annotation : itemType = "Annotation"; break;
    case CorpusObject::Type_Participation : itemType = "Participation"; break;
    case CorpusObject::Type_Undefined : itemType = "Corpus Item"; break;
    }

    QtProperty *groupTopLevel = m_groupManager->addProperty(itemType + " " + itemID);
    QtProperty *group = groupTopLevel;
    // Basic attributes
    if (itemType != "Participation") {
        // ID, Name (common for every type of object, except participations)
        property = m_stringManager->addProperty("ID");
        m_stringManager->setValue(property, item->ID());
        addProperty(property, item->type(), itemID, QLatin1String("ID"), groupTopLevel);
        property = m_stringManager->addProperty("Name");
        m_stringManager->setValue(property, item->property("name").toString());
        addProperty(property, item->type(), itemID, QLatin1String("name"), groupTopLevel);
    } else {
        property = m_stringManager->addProperty("Role");
        m_stringManager->setValue(property, item->property("role").toString());
        addProperty(property, item->type(), itemID, QLatin1String("role"), groupTopLevel);
    }
    // Used-defined attributes
    for (int i = 0; i < mstructure->sectionsCount(item->type()); ++i) {
        MetadataStructureSection *sec = mstructure->section(item->type(), i);
        if (!sec) continue;
        if (i != 0) group = m_groupManager->addProperty(sec->name());
        foreach (QPointer<MetadataStructureAttribute> attr, sec->attributes()) {
            if (!attr) continue;
            if (attr->datatype() == "datetime") {
                property = m_dateManager->addProperty(attr->name());
                m_dateManager->setValue(property, item->property(attr->ID()).toDate());
                addProperty(property, item->type(), itemID, QLatin1String(attr->ID().toLatin1()), group);
            }
            else {
                property = m_stringManager->addProperty(attr->name());
                m_stringManager->setValue(property, item->property(attr->ID()).toString());
                addProperty(property, item->type(), itemID, QLatin1String(attr->ID().toLatin1()), group);
            }
        }
    }
    // Basic attributes that appear after user-defined ones
    if (item->type() == CorpusObject::Type_Recording) {
        property = m_stringManager->addProperty("File name");
        m_stringManager->setValue(property, item->property("filename").toString());
        addProperty(property, item->type(), itemID, QLatin1String("filename"), groupTopLevel);
        property = m_doubleManager->addProperty("Duration");
        m_doubleManager->setValue(property, item->property("duration").toDouble());
        addProperty(property, item->type(), itemID, QLatin1String("duration"), groupTopLevel);
        //
        QtProperty *groupSoundInfo = m_groupManager->addProperty("Sound file information");
        property = m_stringManager->addProperty("Format");
        m_stringManager->setValue(property, item->property("format").toString());
        addProperty(property, item->type(), itemID, QLatin1String("format"), groupSoundInfo);
        property = m_intManager->addProperty("Channels");
        m_intManager->setValue(property, item->property("channels").toInt());
        addProperty(property, item->type(), itemID, QLatin1String("channels"), groupSoundInfo);
        property = m_intManager->addProperty("Sample rate");
        m_intManager->setValue(property, item->property("sampleRate").toInt());
        addProperty(property, item->type(), itemID, QLatin1String("sampleRate"), groupSoundInfo);
        addProperty(groupSoundInfo, item->type(), itemID, QLatin1String("sound_info"), groupTopLevel);
    }
    // Finished building, add to structure
    if (under)
        addProperty(groupTopLevel, item->type(), itemID, QLatin1String("metadata"), under);
    else
        addProperty(groupTopLevel, item->type(), itemID, QLatin1String("metadata"), 0);
    return groupTopLevel;
}

void MetadataEditorWidget::clear()
{
    // Clean up previous items
    m_items.clear();
    QMap<QtProperty *, PropertyID>::ConstIterator itProp = m_propertyToId.constBegin();
    while (itProp != m_propertyToId.constEnd()) {
        delete itProp.key();
        itProp++;
    }
    m_propertyToId.clear();
    m_idToProperty.clear();
}

void MetadataEditorWidget::rebind(QPointer<MetadataStructure> mstructure, QList<QPointer<CorpusObject> > &items,
                                  bool includeParticipationSpeaker, bool includeParticipationCommunication)
{
    if (!mstructure) return;
    // Clean up previous items
    clear();
    // Create property box of new items
    foreach (QPointer<CorpusObject> item, items) {
        if (!item) continue;
        m_items.insert(QPair<CorpusObject::Type, QString>(item->type(), item->ID()), item);
        QtProperty *group = addProperties(mstructure, item);
        if (!group) continue;
        if (item->type() == CorpusObject::Type_Participation && includeParticipationSpeaker) {
            CorpusParticipation *part = qobject_cast<CorpusParticipation *>(item);
            if (!part) continue;
            m_items.insert(QPair<CorpusObject::Type, QString>(CorpusObject::Type_Speaker, part->speakerID()),
                           static_cast<CorpusObject *>(part->speaker()));
            addProperties(mstructure, static_cast<CorpusObject *>(part->speaker()), group);
        }
        if (item->type() == CorpusObject::Type_Participation && includeParticipationCommunication) {
            CorpusParticipation *part = qobject_cast<CorpusParticipation *>(item);
            if (!part) continue;
            m_items.insert(QPair<CorpusObject::Type, QString>(CorpusObject::Type_Communication, part->communicationID()),
                           static_cast<CorpusObject *>(part->communication()));
            addProperties(mstructure, static_cast<CorpusObject *>(part->communication()), group);
        }
    }
}

// Update properties mechanism

void MetadataEditorWidget::valueChanged(QtProperty *property, const QString &value)
{
    if (!m_propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = m_items.value(QPair<CorpusObject::Type, QString>(m_propertyToId[property].type, m_propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(m_propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, bool value)
{
    if (!m_propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = m_items.value(QPair<CorpusObject::Type, QString>(m_propertyToId[property].type, m_propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(m_propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, int value)
{
    if (!m_propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = m_items.value(QPair<CorpusObject::Type, QString>(m_propertyToId[property].type, m_propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(m_propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, double value)
{
    if (!m_propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = m_items.value(QPair<CorpusObject::Type, QString>(m_propertyToId[property].type, m_propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(m_propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, const QDate &value)
{
    if (!m_propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = m_items.value(QPair<CorpusObject::Type, QString>(m_propertyToId[property].type, m_propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(m_propertyToId[property].attributeID, value);
}

