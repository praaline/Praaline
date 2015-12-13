/****************************************************************************
** Meta object code from reading C++ file 'qtgroupboxpropertybrowser.h'
**
** Created: Sun 7. Oct 20:28:20 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qtgroupboxpropertybrowser.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtgroupboxpropertybrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtGroupBoxPropertyBrowser[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x08,
      40,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtGroupBoxPropertyBrowser[] = {
    "QtGroupBoxPropertyBrowser\0\0slotUpdate()\0"
    "slotEditorDestroyed()\0"
};

void QtGroupBoxPropertyBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtGroupBoxPropertyBrowser *_t = static_cast<QtGroupBoxPropertyBrowser *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotUpdate(); break;
        case 1: _t->d_func()->slotEditorDestroyed(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QtGroupBoxPropertyBrowser::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtGroupBoxPropertyBrowser::staticMetaObject = {
    { &QtAbstractPropertyBrowser::staticMetaObject, qt_meta_stringdata_QtGroupBoxPropertyBrowser,
      qt_meta_data_QtGroupBoxPropertyBrowser, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtGroupBoxPropertyBrowser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtGroupBoxPropertyBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtGroupBoxPropertyBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtGroupBoxPropertyBrowser))
        return static_cast<void*>(const_cast< QtGroupBoxPropertyBrowser*>(this));
    return QtAbstractPropertyBrowser::qt_metacast(_clname);
}

int QtGroupBoxPropertyBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyBrowser::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
