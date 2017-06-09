#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTextDocument>
#include <QColor>
#include <QStyle>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>

#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;

#include "IntervalSequenceEditor.h"

struct IntervalSequenceEditorData
{
    QList<Interval *> intervals;
    QStringList attributeIDs;
    QList<QRect> boundingRects;
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
    recalculateRectangles();
    update();
}

void IntervalSequenceEditor::setIntervalAttributeIDs(const QStringList &attributeIDs)
{
    d->attributeIDs = attributeIDs;
    recalculateRectangles();
    update();
}

QStringList IntervalSequenceEditor::intervalAttributeIDs() const
{
    return d->attributeIDs;
}


QList<Praaline::Core::Interval *> IntervalSequenceEditor::intervals()
{
    return d->intervals;
}

void IntervalSequenceEditor::recalculateRectangles()
{
    // Recalculate bounding rectangles
    d->boundingRects.clear();

    QFont intervalsFont = QApplication::font();
    QFontMetrics intervalsFm(intervalsFont);

    foreach (Interval *intv, d->intervals) {
        // Text
        QRect rect = intervalsFm.boundingRect(0, 0, 0, 0, Qt::AlignCenter|Qt::AlignVCenter,
                                              intv->text());
        foreach (QString attributeID, d->attributeIDs) {
            QRect rectAttr = intervalsFm.boundingRect(0, 0, 0, 0, Qt::AlignCenter|Qt::AlignVCenter,
                                                  intv->attribute(attributeID).toString());
            if (rectAttr.width() > rect.width()) rect = rectAttr;
        }
        d->boundingRects << rect;
    }
}

void IntervalSequenceEditor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    // paint(d->intervals, d->boundingRects, &painter, rect(), this->palette(), IntervalSequenceEditor::Editable);
}

void IntervalSequenceEditor::mouseReleaseEvent(QMouseEvent *event)
{

}

int IntervalSequenceEditor::boundaryAtPosition(int x)
{

}

void IntervalSequenceEditor::paint(QList<Interval *> intervals, QPainter *painter, QStyleOptionViewItem &option,
                                   IntervalSequenceEditor::EditMode mode)
{
    painter->save();

//    if (option.state & QStyle::State_Selected)
//        painter->fillRect(option.rect, option.palette.highlight());

//    QString headerText = index.data(HeaderRole).toString();
//    QString subheaderText = index.data(SubheaderRole).toString();

//    QFont headerFont = QApplication::font();
//    headerFont.setBold(true);
//    QFont subheaderFont = QApplication::font();
//    QFontMetrics headerFm(headerFont);
//    QFontMetrics subheaderFm(subheaderFont);

//    /*
//     * The x,y coords are not (0,0) but values given by 'option'. So, calculate the
//     * rects again given the x,y,w.
//     * Note that the given height is 0. That is because boundingRect() will return
//     * the suitable height if the given geometry does not fit. And this is exactly
//     * what we want.
//     */
//    QRect headerRect =
//            headerFm.boundingRect(option.rect.left() + iconSize.width(), option.rect.top() + padding,
//                                  option.rect.width() - iconSize.width(), 0,
//                                  Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,
//                                  headerText);
//    QRect subheaderRect =
//            subheaderFm.boundingRect(headerRect.left(), headerRect.bottom()+padding,
//                                     option.rect.width() - iconSize.width(), 0,
//                                     Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,
//                                     subheaderText);

//    painter->setPen(Qt::black);

//    painter->setFont(headerFont);
//    painter->drawText(headerRect, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, headerText);

//    painter->setFont(subheaderFont);
//    painter->drawText(subheaderRect, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, subheaderText);

    painter->restore();
}

QSize IntervalSequenceEditor::sizeHint(QList<Interval *> intervals)
{
//    QString intervalsText = index.data(HeaderRole).toString();



//    /* No need for x,y here. we only need to calculate the height given the width.
//     * Note that the given height is 0. That is because boundingRect() will return
//     * the suitable height if the given geometry does not fit. And this is exactly
//     * what we want.
//     */
//    QRect headerRect = headerFm.boundingRect(0, 0,
//                                             option.rect.width() - iconSize.width(), 0,
//                                             Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,
//                                             headerText);
//    QRect subheaderRect = subheaderFm.boundingRect(0, 0,
//                                                   option.rect.width() - iconSize.width(), 0,
//                                                   Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,
//                                                   subheaderText);

//    QSize size(option.rect.width(), headerRect.height() + subheaderRect.height() +  3*padding);

//    /* Keep the minimum height needed in mind. */
//    if(size.height()<iconSize.height())
//        size.setHeight(iconSize.height());

    QSize size;

    return size;


}
