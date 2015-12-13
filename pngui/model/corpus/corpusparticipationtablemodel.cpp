#include "corpusparticipationtablemodel.h"

CorpusParticipationTableModel::CorpusParticipationTableModel(QList<QPointer<CorpusParticipation> > items,
                                                             QList<QPointer<MetadataStructureAttribute> > attributes,
                                                             QPointer<Corpus> corpus, bool multiCorpus, QObject *parent)
    : QAbstractTableModel(parent), m_multiCorpus(multiCorpus), m_items(items), m_attributes(attributes), m_corpus(corpus)
{
}

CorpusParticipationTableModel::~CorpusParticipationTableModel()
{
}

QModelIndex CorpusParticipationTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int CorpusParticipationTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.count();
}

int CorpusParticipationTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int basicPropertiesCount = (m_multiCorpus) ? 4 : 3;
    return basicPropertiesCount + m_attributes.count();
}

QVariant CorpusParticipationTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (m_multiCorpus) {
            if (section == 0) return tr("Corpus ID");
            else if (section == 1) return tr("Communication ID");
            else if (section == 2) return tr("Speaker ID");
            else if (section == 3) return tr("Role");
            else if ((section - 4) >= 0 && (section - 4) < m_attributes.count())
                return m_attributes.at(section - 4)->name();
            else
                return QVariant();
        } else {
            if (section == 0) return tr("Communication ID");
            else if (section == 1) return tr("Speaker ID");
            else if (section == 2) return tr("Role");
            else if ((section - 3) >= 0 && (section - 3) < m_attributes.count())
                return m_attributes.at(section - 3)->name();
            else
                return QVariant();
        }
    }
    return QVariant();
}

QVariant CorpusParticipationTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_items.count() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QPointer<CorpusParticipation> part = m_items.at(index.row());
        if (!part) return QVariant();
        if (m_multiCorpus) {
            if (index.column() == 0) return part->property("corpusID");
            else if (index.column() == 1) return part->property("communicationID");
            else if (index.column() == 2) return part->property("speakerID");
            else if (index.column() == 3) return part->property("role");
            else if ((index.column() - 4) >= 0 && (index.column() - 4) < m_attributes.count())
                return part->property(m_attributes.at(index.column() - 4)->ID());
            else
                return QVariant();
        } else {
            if (index.column() == 0) return part->property("communicationID");
            else if (index.column() == 1) return part->property("speakerID");
            else if (index.column() == 2) return part->property("role");
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < m_attributes.count())
                return part->property(m_attributes.at(index.column() - 3)->ID());
            else
                return QVariant();
        }
    }
    return QVariant();
}


Qt::ItemFlags CorpusParticipationTableModel::flags(const QModelIndex &index) const
{
    if (index.column() < columnCount())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index);
}

bool CorpusParticipationTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (index.row() >= m_items.count() || index.row() < 0)
        return false;

    if (role == Qt::EditRole) {
        QPointer<CorpusParticipation> part = m_items.at(index.row());
        if (!part) return false;
        if (m_multiCorpus) {
            if      (index.column() == 0) return part->setProperty("corpusID", value);
            else if (index.column() == 1) return part->setProperty("communicationID", value);
            else if (index.column() == 2) return part->setProperty("speakerID", value);
            else if (index.column() == 3) return part->setProperty("role", value);
            else if ((index.column() - 4) >= 0 && (index.column() - 4) < m_attributes.count())
                return part->setProperty(m_attributes.at(index.column() - 4)->ID(), value);
            else
                return false;
        } else {
            if      (index.column() == 0) return part->setProperty("communicationID", value);
            else if (index.column() == 1) return part->setProperty("speakerID", value);
            else if (index.column() == 2) return part->setProperty("role", value);
            else if ((index.column() - 3) >= 0 && (index.column() - 3) < m_attributes.count())
                return part->setProperty(m_attributes.at(index.column() - 3)->ID(), value);
            else
                return false;
        }
    }
    return QAbstractTableModel::setData(index, value, role);
}

bool CorpusParticipationTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

bool CorpusParticipationTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    return false;
}

