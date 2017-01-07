#include "ConnectionBlurEffect.h"

#include "ConnectionGraphicsObject.h"
#include "ConnectionPainter.h"

ConnectionBlurEffect::
ConnectionBlurEffect(ConnectionGraphicsObject* object)
    : _object(object)
{
    //
}


void
ConnectionBlurEffect::
draw(QPainter* painter)
{
    QGraphicsBlurEffect::draw(painter);

    //ConnectionPainter::paint(painter,
    //_object->connectionGeometry(),
    //_object->connectionState());

    //_item->paint(painter, nullptr, nullptr);
}
