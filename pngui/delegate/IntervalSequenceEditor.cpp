#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTextDocument>
#include <QColor>
#include <QStyle>

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
    setAutoFillBackground(false);
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

    QTextDocument doc;

    // Since the QTextDocument will do all the rendering, the color,
    // and the font have to be put back inside the doc
//    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
//                              ? QPalette::Normal : QPalette::Disabled;
//    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
//        cg = QPalette::Inactive;
    QColor textColor = palette.color(QPalette::Normal, QPalette::Text);
    doc.setDefaultStyleSheet(QString("body { color: %1}")
                             .arg(textColor.name()));

    QString html;
    foreach (Interval *intv, intervals) {
        html.append(intv->text()).append(" ");
    }
    doc.setHtml(html);
    doc.setDocumentMargin(1); // the default is 4 which is too much

    //painter->save();
    painter->translate(rect.topLeft());
    doc.drawContents(painter);
    // painter->restore();





    painter->restore();
}

QSize IntervalSequenceEditor::sizeHint(QList<Interval *> intervals)
{

}
