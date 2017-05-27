#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;

#include "IntervalSequenceEditor.h"

struct IntervalSequenceEditorData
{
    QList<Interval *> intervals;
};

IntervalSequenceEditor::IntervalSequenceEditor(QWidget *parent) :
    QWidget(parent), d(new IntervalSequenceEditorData)
{
}

IntervalSequenceEditor::~IntervalSequenceEditor()
{
}

QSize IntervalSequenceEditor::sizeHint() const
{

}

void IntervalSequenceEditor::setIntervals(const QList<Praaline::Core::Interval *> intervals)
{
    d->intervals = intervals;
}

QList<Praaline::Core::Interval *> IntervalSequenceEditor::intervals()
{
    return d->intervals;
}

void IntervalSequenceEditor::paintEvent(QPaintEvent *event)
{

}

void IntervalSequenceEditor::mouseReleaseEvent(QMouseEvent *event)
{

}

int IntervalSequenceEditor::boundaryAtPosition(int x)
{

}
