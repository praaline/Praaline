#ifndef INTERVALSEQUENCEDELEGATE_H
#define INTERVALSEQUENCEDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

struct IntervalSequenceDelegateData;

class IntervalSequenceDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit IntervalSequenceDelegate(QWidget *parent = nullptr);
    virtual ~IntervalSequenceDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

signals:
    void splitSequence(int itemIndex);
    void mergeSequenceWithPrevious();
    void mergeSequenceWithNext();

private slots:
    void commitAndCloseEditor();
    void editingSplitSequence(int itemIndex);
    void editingMergeSequenceWithPrevious();
    void editingMergeSequenceWithNext();

private:
    IntervalSequenceDelegateData *d;
};

#endif // INTERVALSEQUENCEDELEGATE_H
