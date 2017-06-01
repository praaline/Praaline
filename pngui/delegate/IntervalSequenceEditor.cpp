#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

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
    setMouseTracking(true);
    setAutoFillBackground(true);
}

IntervalSequenceEditor::~IntervalSequenceEditor()
{
    delete d;
}

QSize IntervalSequenceEditor::sizeHint() const
{
    return sizeHint(d->intervals);
}

void IntervalSequenceEditor::setIntervals(const QList<Praaline::Core::Interval *> intervals)
{
    d->intervals = intervals;
}

QList<Praaline::Core::Interval *> IntervalSequenceEditor::intervals()
{
    return d->intervals;
}

void IntervalSequenceEditor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    paint(d->intervals, &painter, rect(), this->palette(), IntervalSequenceEditor::Editable);
}

void IntervalSequenceEditor::mouseReleaseEvent(QMouseEvent *event)
{

}

int IntervalSequenceEditor::boundaryAtPosition(int x)
{

}

void IntervalSequenceEditor::paint(QList<Interval *> intervals, QPainter *painter, const QRect &rect, const QPalette &palette,
                                   IntervalSequenceEditor::EditMode mode)
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    if (mode == Editable) {
        painter->setBrush(palette.highlight());
    } else {
        painter->setBrush(palette.foreground());
    }

    foreach (Interval *intv, intervals) {

    }

    painter->restore();
}

QSize IntervalSequenceEditor::sizeHint(QList<Interval *> intervals)
{

}
