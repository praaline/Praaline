#ifndef INTERVALSEQUENCEEDITOR_H
#define INTERVALSEQUENCEEDITOR_H

#include <QWidget>
#include <QStyleOptionViewItem>

namespace Praaline {
namespace Core {
class Interval;
}
}

struct IntervalSequenceEditorData;

class IntervalSequenceEditor : public QWidget
{
    Q_OBJECT

public:
    enum EditMode { Editable, ReadOnly };

    IntervalSequenceEditor(QWidget *parent = nullptr);
    ~IntervalSequenceEditor();

    void setIntervals(const QList<Praaline::Core::Interval *> intervals);
    QList<Praaline::Core::Interval *> intervals();

    void setIntervalAttributeIDs(const QStringList &attributeIDs);
    QStringList intervalAttributeIDs() const;

    QSize sizeHint() const override;

    static void paint(QList<Praaline::Core::Interval *> intervals, QPainter *painter, QStyleOptionViewItem &option, EditMode mode);
    static QSize sizeHint(QList<Praaline::Core::Interval *> intervals);

signals:
    void editingSplitSequence(int itemIndex);
    void editingMergeSequenceWithPrevious();
    void editingMergeSequenceWithNext();
    void editingFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int boundaryAtPosition(int x);
    void recalculateRectangles();

    IntervalSequenceEditorData *d;
};

#endif // INTERVALSEQUENCEEDITOR_H
