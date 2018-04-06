#include <QDebug>
#include <QMultiHash>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QComboBox>
#include <QClipboard>
#include "ImportCorpusItemsWizardAnalysePage.h"
#include "ui_ImportCorpusItemsWizardAnalysePage.h"

#include "pncore/interfaces/praat/PraatTextGrid.h"

QString speakerPolicyToString(int policy) {
    if      (policy == ImportAnnotations::SpeakerPolicySingle)              return QObject::tr("Single Speaker");
    else if (policy == ImportAnnotations::SpeakerPolicyTierNames)           return QObject::tr("Each tier is a Speaker");
    else if (policy == ImportAnnotations::SpeakerPolicyIntervals)           return QObject::tr("Get Speaker ID from tier");
    else if (policy == ImportAnnotations::SpeakerPolicyPrimaryAndSecondary) return QObject::tr("Primary and Secondary");
    return QObject::tr("Unknown");
}

int speakerPolicyToInt(QString policy) {
    if      (policy == QObject::tr("Single Speaker"))           return ImportAnnotations::SpeakerPolicySingle;
    else if (policy == QObject::tr("Each tier is a Speaker"))   return ImportAnnotations::SpeakerPolicyTierNames;
    else if (policy == QObject::tr("Get Speaker ID from tier")) return ImportAnnotations::SpeakerPolicyIntervals;
    else if (policy == QObject::tr("Primary and Secondary"))    return ImportAnnotations::SpeakerPolicyPrimaryAndSecondary;
    return 0;
}

QWidget *ImportCorpusItemsDelegateSpeakerPolicy::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.column() != 3)
        return QStyledItemDelegate::createEditor(parent, option, index);
    QComboBox *cb = new QComboBox(parent);
    cb->addItem(QString(tr("Single Speaker")));
    cb->addItem(QString(tr("Each tier is a Speaker")));
    cb->addItem(QString(tr("Get Speaker ID from tier")));
    cb->addItem(QString(tr("Primary and Secondary")));
    return cb;
}
void ImportCorpusItemsDelegateSpeakerPolicy::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        int policy = index.data(Qt::EditRole).toInt();
        if      (policy == ImportAnnotations::SpeakerPolicySingle)              cb->setCurrentIndex(0);
        else if (policy == ImportAnnotations::SpeakerPolicyTierNames)           cb->setCurrentIndex(1);
        else if (policy == ImportAnnotations::SpeakerPolicyIntervals)           cb->setCurrentIndex(2);
        else if (policy == ImportAnnotations::SpeakerPolicyPrimaryAndSecondary) cb->setCurrentIndex(3);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
void ImportCorpusItemsDelegateSpeakerPolicy::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->currentText(), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

// --------------------------------------------------------------------------------------------------------------------

ImportCorpusItemsWizardAnalysePage::ImportCorpusItemsWizardAnalysePage(
        QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
        QSet<QString> &tierNamesCommon,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardAnalysePage),
    m_candidateAnnotations(candidateAnnotations), m_tierCorrespondances(tierCorrespondances), m_tierNamesCommon(tierNamesCommon),
    m_modelAnnotations(0), m_stop(false)
{
    ui->setupUi(this);
    connect(ui->commandStopFiles, SIGNAL(clicked()), this, SLOT(stopProcess()));
    setTitle(tr("Analysing annotation files"));
    setSubTitle(tr("Please wait while your annotation files are being analysed."));
    ui->progressBarFiles->setValue(0);
    ui->tableviewAnnotations->verticalHeader()->setDefaultSectionSize(20);

    connect(ui->commandUpdateSpeakerID, SIGNAL(clicked(bool)), this, SLOT(updateSpeakerIDTier()));
    connect(ui->commandCopyTable, SIGNAL(clicked()), this, SLOT(copyAnalysisTable()));
}

ImportCorpusItemsWizardAnalysePage::~ImportCorpusItemsWizardAnalysePage()
{
    delete ui;
}

void ImportCorpusItemsWizardAnalysePage::initializePage()
{
    if (m_modelAnnotations) delete m_modelAnnotations;
    m_modelAnnotations = new QStandardItemModel(this);
    m_modelAnnotations->setColumnCount(6);
    int i = 0;
    QPair<QString, QString> key;
    foreach (key, m_candidateAnnotations.keys()) {
        QString communicationID = key.first;
        QPointer<CorpusAnnotation> annot = m_candidateAnnotations.value(key);
        m_modelAnnotations->setItem(i, 0, new QStandardItem(communicationID));
        m_modelAnnotations->setItem(i, 1, new QStandardItem(annot->filename()));
        m_modelAnnotations->setItem(i, 2, new QStandardItem(annot->ID()));
        m_modelAnnotations->setItem(i, 3, new QStandardItem(speakerPolicyToString(annot->property("speakerPolicy").toInt())));
        m_modelAnnotations->setItem(i, 4, new QStandardItem(annot->property("speakerPolicyData").toString()));
        m_modelAnnotations->setItem(i, 5, new QStandardItem(""));
        i++;
    }
    m_modelAnnotations->setRowCount(i);
    QStringList headers;
    headers << tr("Communication ID") << tr("Filename") << tr("Annotation ID") << tr("Speaker Policy")
            << tr("Speaker ID or Tier") << tr("Contents");
    m_modelAnnotations->setHorizontalHeaderLabels(headers);
    ui->tableviewAnnotations->setModel(m_modelAnnotations);
    ImportCorpusItemsDelegateSpeakerPolicy *delegate = new ImportCorpusItemsDelegateSpeakerPolicy(ui->tableviewAnnotations);
    ui->tableviewAnnotations->setItemDelegate(delegate);
}

bool ImportCorpusItemsWizardAnalysePage::validatePage()
{
    m_stop = false;
    // update annotations
    qDeleteAll(m_candidateAnnotations);
    m_candidateAnnotations.clear();
    for (int i = 0; i < m_modelAnnotations->rowCount(); ++i) {
        QString communicationID = m_modelAnnotations->data(m_modelAnnotations->index(i, 0)).toString();
        QString filename = m_modelAnnotations->data(m_modelAnnotations->index(i, 1)).toString();
        QString annotationID = m_modelAnnotations->data(m_modelAnnotations->index(i, 2)).toString();
        int speakerPolicy =  speakerPolicyToInt(m_modelAnnotations->data(m_modelAnnotations->index(i, 3)).toString());
        QString speakerPolicyData = m_modelAnnotations->data(m_modelAnnotations->index(i, 4)).toString();
        CorpusAnnotation *annot = new CorpusAnnotation(annotationID);
        annot->setName(annotationID);
        annot->setProperty("speakerPolicy", speakerPolicy);
        annot->setProperty("speakerPolicyData", speakerPolicyData);
        annot->setFilename(filename);
        m_candidateAnnotations.insert(QPair<QString, QString>(communicationID, filename), annot);
    }
    return loadTextgrids();
}

void ImportCorpusItemsWizardAnalysePage::updateSpeakerIDTier()
{
    QString newSpeakerIDTier = ui->editSpeakerIDTier->text();
    int i = 0;
    QPair<QString, QString> key;
    foreach (key, m_candidateAnnotations.keys()) {
        QPointer<CorpusAnnotation> annot = m_candidateAnnotations.value(key);
        annot->setProperty("speakerPolicyData", newSpeakerIDTier);
        m_modelAnnotations->setItem(i, 4, new QStandardItem(annot->property("speakerPolicyData").toString()));
        i++;
    }
}

void ImportCorpusItemsWizardAnalysePage::stopProcess()
{
    m_stop = true;
}

bool ImportCorpusItemsWizardAnalysePage::loadTextgrids()
{
    if (m_stop) return false;

    ui->progressBarFiles->setMaximum(m_candidateAnnotations.count());
    m_tierCorrespondances.clear();
    int i = 1;
    foreach (QPointer<CorpusAnnotation> annot, m_candidateAnnotations.values()) {
        if (!annot) continue;
        QString filename = annot->filename();
        if (m_stop) return false;
        QList<PraatTierData> tiers = PraatTextGrid::getTierData(filename);
        if (tiers.isEmpty()) {
            m_modelAnnotations->setItem(i - 1, 5, new QStandardItem(tr("Cannot read file")));
            continue;
        }
        QSet<QString> tierNames;
        foreach (PraatTierData tier, tiers) {
            tierNames << tier.name;
            m_tierCorrespondances.insert(filename, TierCorrespondance(filename, tier.name, tier.type, tier.itemsCount));
        }
        if (i == 1)
            m_tierNamesCommon = tierNames;
        else
            m_tierNamesCommon = m_tierNamesCommon.intersect(tierNames);
        // update UI
        QStringList tierNamesList = tierNames.toList();
        qSort(tierNamesList);
        m_modelAnnotations->setItem(i - 1, 5, new QStandardItem(QString(tr("%1 tiers: %2")).arg(tiers.count()).arg(tierNamesList.join(", "))));
        ui->progressBarFiles->setValue(i);
        QApplication::processEvents();
        i++;
    }

    return true;
}

void ImportCorpusItemsWizardAnalysePage::copyAnalysisTable()
{
    QAbstractItemModel *abmodel = ui->tableviewAnnotations->model();
    if (!abmodel) return;
    QItemSelectionModel * model = ui->tableviewAnnotations->selectionModel();
    if (!model) return;
    QModelIndexList list = model->selectedIndexes();
    if(list.size() < 1) return;

    qSort(list);

    QString copy_table;
    QModelIndex previous = list.first();

    list.removeFirst();

    for(int i = 0; i < list.size(); i++) {
        QVariant data = abmodel->data(previous);
        QString text = data.toString();

        QModelIndex index = list.at(i);
        copy_table.append(text);

        if(index.row() != previous.row())
            copy_table.append('\n');
        else
            copy_table.append('\t');
        previous = index;
    }

    if (!list.isEmpty()) {
        copy_table.append(abmodel->data(list.last()).toString());
        copy_table.append('\n');
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copy_table);
}
