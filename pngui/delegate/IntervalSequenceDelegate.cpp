#include <QWidget>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QAbstractItemModel>
#include <QModelIndex>

#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;

#include "IntervalSequenceEditor.h"
#include "IntervalSequenceDelegate.h"

struct IntervalSequenceDelegateData {
    IntervalSequenceDelegateData() :
        editor(0)
    {}

    IntervalSequenceEditor *editor;
};


IntervalSequenceDelegate::IntervalSequenceDelegate(QWidget *parent)  :
    QStyledItemDelegate(parent), d(new IntervalSequenceDelegateData())
{
}

IntervalSequenceDelegate::~IntervalSequenceDelegate()
{
}

void IntervalSequenceDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (d->editor && (index.data().canConvert<QList<Interval *> >())) {
        QList<Interval *> intervals = qvariant_cast<QList<Interval *> >(index.data());
        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        // IntervalSequenceEditor::paint(intervals, painter, option, IntervalSequenceEditor::ReadOnly);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize IntervalSequenceDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().canConvert<QList<Interval *> >()) {
        QList<Interval *> intervals = qvariant_cast<QList<Interval *> >(index.data());
        return IntervalSequenceEditor::sizeHint(intervals);
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

QWidget *IntervalSequenceDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().canConvert<QList<Interval *> >()) {
        d->editor = new IntervalSequenceEditor(parent);
        connect(d->editor, &IntervalSequenceEditor::editingFinished, this, &IntervalSequenceDelegate::commitAndCloseEditor);
        connect(d->editor, &IntervalSequenceEditor::editingSplitSequence, this, &IntervalSequenceDelegate::splitSequence);
        connect(d->editor, &IntervalSequenceEditor::editingMergeSequenceWithPrevious, this, &IntervalSequenceDelegate::mergeSequenceWithPrevious);
        connect(d->editor, &IntervalSequenceEditor::editingMergeSequenceWithNext, this, &IntervalSequenceDelegate::mergeSequenceWithNext);
        return d->editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void IntervalSequenceDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.data().canConvert<QList<Interval *> >()) {
        QList<Interval *> intervals = qvariant_cast<QList<Interval *> >(index.data());
        IntervalSequenceEditor *intervalsEditor = qobject_cast<IntervalSequenceEditor *>(editor);
        intervalsEditor->setIntervals(intervals);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void IntervalSequenceDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.data().canConvert<QList<Interval *> >()) {
        IntervalSequenceEditor *intervalsEditor = qobject_cast<IntervalSequenceEditor *>(editor);
        model->setData(index, QVariant::fromValue(intervalsEditor->intervals()));
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void IntervalSequenceDelegate::commitAndCloseEditor()
{
    IntervalSequenceEditor *intervalsEditor = qobject_cast<IntervalSequenceEditor *>(sender());
    emit commitData(intervalsEditor);
    emit closeEditor(intervalsEditor);
}

void IntervalSequenceDelegate::editingSplitSequence(int itemIndex)
{
    emit splitSequence(itemIndex);
}

void IntervalSequenceDelegate::editingMergeSequenceWithPrevious()
{
    emit mergeSequenceWithPrevious();
}

void IntervalSequenceDelegate::editingMergeSequenceWithNext()
{
    emit mergeSequenceWithNext();
}

