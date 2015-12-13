
/* Bits pulled out from Node.cpp during tidying -- not a useful example yet */

/*!!! move this (and converse) to examples/ as example of encoder registration
static
QString
qTimeToXsdDuration(QTime t)
{
    if (t.hour() != 0) {
        return
            QString("PT%1H%2M%3S")
            .arg(t.hour())
            .arg(t.minute())
            .arg(t.second() + double(t.msec())/1000.0);
    } else if (t.minute() != 0) {
        return
            QString("PT%1M%2S")
            .arg(t.minute())
            .arg(t.second() + double(t.msec())/1000.0);
    } else {
        return
            QString("PT%1S")
            .arg(t.second() + double(t.msec())/1000.0);
    }
}
*/



