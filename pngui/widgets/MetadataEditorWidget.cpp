#include <QPointer>
#include <QString>
#include <QMap>
#include <QPair>
#include <QDate>
#include <QGridLayout>
#include <QScrollArea>

#include "pncore/corpus/CorpusParticipation.h"
#include "pncore/corpus/CorpusObject.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/NameValueList.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/NameValueListDatastore.h"
using namespace Praaline::Core;

// Qt property browser
#include "qtpropertymanager.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"
#include "qtgroupboxpropertybrowser.h"
#include "qtbuttonpropertybrowser.h"

#include "MetadataEditorWidget.h"

struct MetadataEditorWidgetData {
    MetadataEditorWidgetData () :
        stringManager(0), boolManager(0), intManager(0), doubleManager(0), enumManager(0), dateManager(0), groupManager(0),
        doubleSpinBoxFactory(0), checkBoxFactory(0), spinBoxFactory(0), lineEditFactory(0), comboBoxFactory(0), dateEditFactory(0),
        mainLayout(0), propertyEditor(0), style(MetadataEditorWidget::TreeStyle), metadataStructure(0)
    {}

    QtStringPropertyManager *stringManager;
    QtBoolPropertyManager *boolManager;
    QtIntPropertyManager *intManager;
    QtDoublePropertyManager *doubleManager;
    QtEnumPropertyManager *enumManager;
    QtDatePropertyManager *dateManager;
    QtTimePropertyManager *timeManager;
    QtDateTimePropertyManager *dateTimeManager;
    QtGroupPropertyManager *groupManager;

    QtDoubleSpinBoxFactory *doubleSpinBoxFactory;
    QtCheckBoxFactory *checkBoxFactory;
    QtSpinBoxFactory *spinBoxFactory;
    QtLineEditFactory *lineEditFactory;
    QtEnumEditorFactory *comboBoxFactory;
    QtDateEditFactory *dateEditFactory;
    QtTimeEditFactory *timeEditFactory;
    QtDateTimeEditFactory *dateTimeEditFactory;

    QGridLayout *mainLayout;
    QScrollArea *scroll;
    QtAbstractPropertyBrowser *propertyEditor;
    MetadataEditorWidget::MetadataEditorWidgetStyle style;

    QPointer<MetadataStructure> metadataStructure;
    QMap<QPair<Praaline::Core::CorpusObject::Type, QString>, QPointer<Praaline::Core::CorpusObject> > items;
    QList<QPointer<Praaline::Core::CorpusObject> > itemsSorted;

    QMap<QtProperty *, MetadataEditorWidget::PropertyID> propertyToId;
    QMap<MetadataEditorWidget::PropertyID, QtProperty *> idToProperty;

    QMap<QtProperty *, NameValueList *> nameValueLists;
};


MetadataEditorWidget::MetadataEditorWidget(MetadataEditorWidgetStyle style, QWidget *parent) :
    QWidget(parent), d(new MetadataEditorWidgetData)
{
    // Managers
    d->groupManager = new QtGroupPropertyManager(this);
    d->stringManager = new QtStringPropertyManager(this);
    d->boolManager = new QtBoolPropertyManager(this);
    d->intManager = new QtIntPropertyManager(this);
    d->doubleManager = new QtDoublePropertyManager(this);
    d->enumManager = new QtEnumPropertyManager(this);
    d->dateManager = new QtDatePropertyManager(this);
    d->timeManager = new QtTimePropertyManager(this);
    d->dateTimeManager = new QtDateTimePropertyManager(this);

    // Factories
    d->lineEditFactory = new QtLineEditFactory(this); // string
    d->checkBoxFactory = new QtCheckBoxFactory(this); // bool
    d->spinBoxFactory = new QtSpinBoxFactory(this); // int
    d->doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this); // double
    d->comboBoxFactory = new QtEnumEditorFactory(this); // enum
    d->dateEditFactory = new QtDateEditFactory(this); // date
    d->timeEditFactory = new QtTimeEditFactory(this); // time
    d->dateTimeEditFactory = new QtDateTimeEditFactory(this); // datetime

    // Signals and slots
    connect(d->stringManager, SIGNAL(valueChanged(QtProperty*, const QString &)),
            this, SLOT(valueChanged(QtProperty *, const QString &)));
    connect(d->boolManager, SIGNAL(valueChanged(QtProperty*, bool)),
            this, SLOT(valueChanged(QtProperty *, bool)));
    connect(d->intManager, SIGNAL(valueChanged(QtProperty*, int)),
            this, SLOT(valueChanged(QtProperty *, int)));
    connect(d->doubleManager, SIGNAL(valueChanged(QtProperty *, double)),
            this, SLOT(valueChanged(QtProperty *, double)));
    connect(d->enumManager, SIGNAL(valueChanged(QtProperty*, int)),
            this, SLOT(enumValueChanged(QtProperty*,int)));
    connect(d->dateManager, SIGNAL(valueChanged(QtProperty*, const QDate &)),
            this, SLOT(valueChanged(QtProperty*, const QDate &)));
    connect(d->timeManager, SIGNAL(valueChanged(QtProperty*,QTime)),
            this, SLOT(valueChanged(QtProperty*, const QTime &)));
    connect(d->dateTimeManager, SIGNAL(valueChanged(QtProperty*,QDateTime)),
            this, SLOT(valueChanged(QtProperty*, const QDateTime &)));

    d->mainLayout = new QGridLayout(this);
    this->setLayout(d->mainLayout);
    d->scroll = new QScrollArea();
    d->scroll->setWidgetResizable(true);

    createPropertyBrowser(style);
}

MetadataEditorWidget::~MetadataEditorWidget()
{
    delete d;
}

void MetadataEditorWidget::createPropertyBrowser(MetadataEditorWidgetStyle style)
{
    // Delete previous property editor (if one exists)
    if (d->propertyEditor) {
        if (d->style == MetadataEditorWidget::TreeStyle)
            d->mainLayout->removeWidget(d->propertyEditor);
        else
            d->mainLayout->removeWidget(d->scroll);
        delete d->propertyEditor;
        d->propertyEditor = 0;
    }

    // Create the property browser, based on the selected style
    switch (style) {
    case MetadataEditorWidget::TreeStyle :       d->propertyEditor = new QtTreePropertyBrowser(this);     break;
    case MetadataEditorWidget::GroupBoxStyle :   d->propertyEditor = new QtGroupBoxPropertyBrowser(this); break;
    case MetadataEditorWidget::ButtonStyle :     d->propertyEditor = new QtButtonPropertyBrowser(this);   break;
    }
    if (!d->propertyEditor) return;

    // Set factories and managers for different data types
    d->propertyEditor->setFactoryForManager(d->stringManager, d->lineEditFactory);
    d->propertyEditor->setFactoryForManager(d->boolManager, d->checkBoxFactory);
    d->propertyEditor->setFactoryForManager(d->intManager, d->spinBoxFactory);
    d->propertyEditor->setFactoryForManager(d->doubleManager, d->doubleSpinBoxFactory);
    d->propertyEditor->setFactoryForManager(d->enumManager, d->comboBoxFactory);
    d->propertyEditor->setFactoryForManager(d->dateManager, d->dateEditFactory);
    d->propertyEditor->setFactoryForManager(d->timeManager, d->timeEditFactory);
    d->propertyEditor->setFactoryForManager(d->dateTimeManager, d->dateTimeEditFactory);

    // Presentation details depending on property browser type
    if (style == MetadataEditorWidget::TreeStyle) {
        QtTreePropertyBrowser *editor = qobject_cast<QtTreePropertyBrowser *>(d->propertyEditor);
        if (editor) editor->setResizeMode(QtTreePropertyBrowser::ResizeToContents);
        d->mainLayout->addWidget(d->propertyEditor);
    } else {
        d->scroll->setWidget(d->propertyEditor);
        d->mainLayout->addWidget(d->scroll);
    }

    // Save the new style
    d->style = style;
}

void MetadataEditorWidget::setEditorStyle(MetadataEditorWidgetStyle style)
{
    // Create new property browser if necessary
    if ((d->style == style) && (d->propertyEditor)) return; // already OK
    createPropertyBrowser(style);
    // Create properties based on saved state
    QtProperty *group = Q_NULLPTR;
    foreach (QPointer<CorpusObject> item, d->itemsSorted) {
        if (!group) group = addProperties(d->metadataStructure, item);
        else addProperties(d->metadataStructure, item, group);
    }
}

void MetadataEditorWidget::addProperty(QtProperty *property, CorpusObject::Type type,
                                       const QString &itemID, const QString &attributeID, QtProperty *group)
{
    d->propertyToId[property] = PropertyID(type, itemID, attributeID);
    d->idToProperty[PropertyID(type, itemID, attributeID)] = property;
    if (!group)
        d->propertyEditor->addProperty(property);
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
    case CorpusObject::Type_Corpus :        itemType = "Corpus";        break;
    case CorpusObject::Type_Communication : itemType = "Communication"; break;
    case CorpusObject::Type_Speaker :       itemType = "Speaker";       break;
    case CorpusObject::Type_Recording :     itemType = "Recording";     break;
    case CorpusObject::Type_Annotation :    itemType = "Annotation";    break;
    case CorpusObject::Type_Participation : itemType = "Participation"; break;
    case CorpusObject::Type_Undefined :     itemType = "Corpus Item";   break;
    default:                                itemType = "Corpus Item";   break;
    }

    QtProperty *groupTopLevel = d->groupManager->addProperty(itemType + " " + itemID);
    QtProperty *group = groupTopLevel;
    // Basic attributes
    if (itemType != "Participation") {
        // ID, Name (common for every type of object, except participations)
        property = d->stringManager->addProperty("ID");
        d->stringManager->setValue(property, item->ID());
        addProperty(property, item->type(), itemID, QLatin1String("ID"), groupTopLevel);
        property = d->stringManager->addProperty("Name");
        d->stringManager->setValue(property, item->property("name").toString());
        addProperty(property, item->type(), itemID, QLatin1String("name"), groupTopLevel);
    } else {
        property = d->stringManager->addProperty("Role");
        d->stringManager->setValue(property, item->property("role").toString());
        addProperty(property, item->type(), itemID, QLatin1String("role"), groupTopLevel);
    }
    // Used-defined attributes
    for (int i = 0; i < mstructure->sectionsCount(item->type()); ++i) {
        MetadataStructureSection *sec = mstructure->section(item->type(), i);
        if (!sec) continue;
        if (i != 0) group = d->groupManager->addProperty(sec->name());
        foreach (MetadataStructureAttribute *attr, sec->attributes()) {
            if (!attr) continue;
            // Select correct manager based on attribute type (string, bool, int, double, enum)
            if (!attr->nameValueList().isEmpty()) {
                property = d->enumManager->addProperty(attr->name());
                QStringList enumItems;
                if (item->repository() && item->repository()->metadata()) {
                    NameValueList *nvl = item->repository()->metadata()->getNameValueList(attr->nameValueList());
                    if (nvl) {
                        d->nameValueLists.insert(property, nvl);
                        enumItems << nvl->displayStrings();
                        d->enumManager->setEnumNames(property, enumItems);
                        d->enumManager->setValue(property, nvl->indexOfValue(item->property(attr->ID())));
                    }
                }
            }
            else if (attr->datatype().base() == DataType::Boolean) {
                property = d->boolManager->addProperty(attr->name());
                d->boolManager->setValue(property, item->property(attr->ID()).toBool());
            }
            else if (attr->datatype().base() == DataType::BigInt || attr->datatype().base() == DataType::Integer ||
                attr->datatype().base() == DataType::SmallInt) {
                property = d->intManager->addProperty(attr->name());
                d->intManager->setValue(property, item->property(attr->ID()).toInt());
            }
            else if (attr->datatype().base() == DataType::Decimal || attr->datatype().base() == DataType::Double ||
                     attr->datatype().base() == DataType::Float) {
                property = d->doubleManager->addProperty(attr->name());
                d->doubleManager->setValue(property, item->property(attr->ID()).toDouble());
            }
            else if (attr->datatype().base() == DataType::Date) {
                property = d->dateManager->addProperty(attr->name());
                d->dateManager->setValue(property, item->property(attr->ID()).toDate());
            }
            else if (attr->datatype().base() == DataType::Time) {
                property = d->timeManager->addProperty(attr->name());
                d->timeManager->setValue(property, item->property(attr->ID()).toTime());
            }
            else if (attr->datatype().base() == DataType::DateTime) {
                property = d->dateTimeManager->addProperty(attr->name());
                d->dateTimeManager->setValue(property, item->property(attr->ID()).toDateTime());
            }
            else {
                property = d->stringManager->addProperty(attr->name());
                d->stringManager->setValue(property, item->property(attr->ID()).toString());
            }
            addProperty(property, item->type(), itemID, QLatin1String(attr->ID().toLatin1()), group);
        }
    }
    // Basic attributes that appear after user-defined ones
    if (item->type() == CorpusObject::Type_Recording) {
        property = d->stringManager->addProperty("File name");
        d->stringManager->setValue(property, item->property("filename").toString());
        addProperty(property, item->type(), itemID, QLatin1String("filename"), groupTopLevel);
        property = d->doubleManager->addProperty("Duration");
        d->doubleManager->setValue(property, item->property("durationSec").toDouble());
        addProperty(property, item->type(), itemID, QLatin1String("durationSec"), groupTopLevel);
        // Sound information
        QtProperty *groupSoundInfo = d->groupManager->addProperty("Sound file information");
        property = d->stringManager->addProperty("Format");
        d->stringManager->setValue(property, item->property("format").toString());
        addProperty(property, item->type(), itemID, QLatin1String("format"), groupSoundInfo);
        property = d->intManager->addProperty("Channels");
        d->intManager->setValue(property, item->property("channels").toInt());
        addProperty(property, item->type(), itemID, QLatin1String("channels"), groupSoundInfo);
        property = d->intManager->addProperty("Sample rate");
        d->intManager->setValue(property, item->property("sampleRate").toInt());
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
    d->items.clear();
    d->itemsSorted.clear();
    QMap<QtProperty *, PropertyID>::ConstIterator itProp = d->propertyToId.constBegin();
    while (itProp != d->propertyToId.constEnd()) {
        delete itProp.key();
        itProp++;
    }
    d->propertyToId.clear();
    d->idToProperty.clear();
    d->nameValueLists.clear();
}

void MetadataEditorWidget::rebind(QPointer<MetadataStructure> mstructure, QList<QPointer<CorpusObject> > &items,
                                  bool includeParticipationSpeaker, bool includeParticipationCommunication)
{
    if (!mstructure) return;
    // Clean up previous items
    clear();
    // Save the items that should be shown
    foreach (QPointer<CorpusObject> item, items) {
        if (!item) continue;
        d->items.insert(QPair<CorpusObject::Type, QString>(item->type(), item->ID()), item);
        d->itemsSorted.append(item);
        if (item->type() == CorpusObject::Type_Participation && includeParticipationSpeaker) {
            CorpusParticipation *part = qobject_cast<CorpusParticipation *>(item);
            if (!part) continue;
            d->items.insert(QPair<CorpusObject::Type, QString>(CorpusObject::Type_Speaker, part->speakerID()),
                           static_cast<CorpusObject *>(part->speaker()));
            d->itemsSorted.append(static_cast<CorpusObject *>(part->speaker()));
        }
        if (item->type() == CorpusObject::Type_Participation && includeParticipationCommunication) {
            CorpusParticipation *part = qobject_cast<CorpusParticipation *>(item);
            if (!part) continue;
            d->items.insert(QPair<CorpusObject::Type, QString>(CorpusObject::Type_Communication, part->communicationID()),
                           static_cast<CorpusObject *>(part->communication()));
            d->itemsSorted.append(static_cast<CorpusObject *>(part->communication()));
        }
    }
    // Create properties
    QtProperty *group = Q_NULLPTR;
    foreach (QPointer<CorpusObject> item, d->itemsSorted) {
        if (!group) group = addProperties(mstructure, item);
        else addProperties(mstructure, item, group);
    }
    // Save state
    d->metadataStructure = mstructure;
}

// ==============================================================================================================================
// Update properties mechanism
// ==============================================================================================================================

void MetadataEditorWidget::valueChanged(QtProperty *property, const QString &value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, bool value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, int value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, double value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, const QDate &value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, const QTime &value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::valueChanged(QtProperty *property, const QDateTime &value)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    item->setProperty(d->propertyToId[property].attributeID, value);
}

void MetadataEditorWidget::enumValueChanged(QtProperty *property, const int index)
{
    if (!d->propertyToId.contains(property)) return;
    QPointer<CorpusObject> item = d->items.value(QPair<CorpusObject::Type, QString>(d->propertyToId[property].type, d->propertyToId[property].itemID));
    if (!item) return;
    NameValueList *nvl = d->nameValueLists.value(property, 0);
    if (!nvl) return;
    QVariant value = nvl->value(index);
    item->setProperty(d->propertyToId[property].attributeID, value);
}
