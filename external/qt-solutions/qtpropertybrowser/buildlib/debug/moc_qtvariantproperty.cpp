/****************************************************************************
** Meta object code from reading C++ file 'qtvariantproperty.h'
**
** Created: Sun 7. Oct 20:28:12 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qtvariantproperty.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtvariantproperty.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtVariantPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      41,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      39,   26,   25,   25, 0x05,
      97,   74,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
     144,   26,   25,   25, 0x0a,
     200,  175,   25,   25, 0x0a,
     245,  243,   25,   25, 0x08,
     282,  279,   25,   25, 0x08,
     320,  243,   25,   25, 0x08,
     359,  243,   25,   25, 0x08,
     396,  279,   25,   25, 0x08,
     440,  243,   25,   25, 0x08,
     482,  243,   25,   25, 0x08,
     519,  243,   25,   25, 0x08,
     554,  243,   25,   25, 0x08,
     592,  243,   25,   25, 0x08,
     631,  243,   25,   25, 0x08,
     668,  243,   25,   25, 0x08,
     704,  279,   25,   25, 0x08,
     746,  243,   25,   25, 0x08,
     782,  243,   25,   25, 0x08,
     822,  243,   25,   25, 0x08,
     865,  243,   25,   25, 0x08,
     901,  243,   25,   25, 0x08,
     939,  243,   25,   25, 0x08,
     976,  243,   25,   25, 0x08,
    1014,  243,   25,   25, 0x08,
    1050,  279,   25,   25, 0x08,
    1092,  243,   25,   25, 0x08,
    1129,  279,   25,   25, 0x08,
    1173,  243,   25,   25, 0x08,
    1209,  243,   25,   25, 0x08,
    1250,  243,   25,   25, 0x08,
    1287,  243,   25,   25, 0x08,
    1329,  243,   25,   25, 0x08,
    1366,  243,   25,   25, 0x08,
    1412,  243,   25,   25, 0x08,
    1462,  243,   25,   25, 0x08,
    1504,  243,   25,   25, 0x08,
    1540,  243,   25,   25, 0x08,
    1578,  243,   25,   25, 0x08,
    1624,  279,   25,   25, 0x08,
    1682,  243,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtVariantPropertyManager[] = {
    "QtVariantPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QVariant)\0"
    "property,attribute,val\0"
    "attributeChanged(QtProperty*,QString,QVariant)\0"
    "setValue(QtProperty*,QVariant)\0"
    "property,attribute,value\0"
    "setAttribute(QtProperty*,QString,QVariant)\0"
    ",\0slotValueChanged(QtProperty*,int)\0"
    ",,\0slotRangeChanged(QtProperty*,int,int)\0"
    "slotSingleStepChanged(QtProperty*,int)\0"
    "slotValueChanged(QtProperty*,double)\0"
    "slotRangeChanged(QtProperty*,double,double)\0"
    "slotSingleStepChanged(QtProperty*,double)\0"
    "slotDecimalsChanged(QtProperty*,int)\0"
    "slotValueChanged(QtProperty*,bool)\0"
    "slotValueChanged(QtProperty*,QString)\0"
    "slotRegExpChanged(QtProperty*,QRegExp)\0"
    "slotEchoModeChanged(QtProperty*,int)\0"
    "slotValueChanged(QtProperty*,QDate)\0"
    "slotRangeChanged(QtProperty*,QDate,QDate)\0"
    "slotValueChanged(QtProperty*,QTime)\0"
    "slotValueChanged(QtProperty*,QDateTime)\0"
    "slotValueChanged(QtProperty*,QKeySequence)\0"
    "slotValueChanged(QtProperty*,QChar)\0"
    "slotValueChanged(QtProperty*,QLocale)\0"
    "slotValueChanged(QtProperty*,QPoint)\0"
    "slotValueChanged(QtProperty*,QPointF)\0"
    "slotValueChanged(QtProperty*,QSize)\0"
    "slotRangeChanged(QtProperty*,QSize,QSize)\0"
    "slotValueChanged(QtProperty*,QSizeF)\0"
    "slotRangeChanged(QtProperty*,QSizeF,QSizeF)\0"
    "slotValueChanged(QtProperty*,QRect)\0"
    "slotConstraintChanged(QtProperty*,QRect)\0"
    "slotValueChanged(QtProperty*,QRectF)\0"
    "slotConstraintChanged(QtProperty*,QRectF)\0"
    "slotValueChanged(QtProperty*,QColor)\0"
    "slotEnumNamesChanged(QtProperty*,QStringList)\0"
    "slotEnumIconsChanged(QtProperty*,QMap<int,QIcon>)\0"
    "slotValueChanged(QtProperty*,QSizePolicy)\0"
    "slotValueChanged(QtProperty*,QFont)\0"
    "slotValueChanged(QtProperty*,QCursor)\0"
    "slotFlagNamesChanged(QtProperty*,QStringList)\0"
    "slotPropertyInserted(QtProperty*,QtProperty*,QtProperty*)\0"
    "slotPropertyRemoved(QtProperty*,QtProperty*)\0"
};

void QtVariantPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtVariantPropertyManager *_t = static_cast<QtVariantPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2]))); break;
        case 1: _t->attributeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QVariant(*)>(_a[3]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2]))); break;
        case 3: _t->setAttribute((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QVariant(*)>(_a[3]))); break;
        case 4: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 6: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 8: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 9: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 10: _t->d_func()->slotDecimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 11: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 12: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 13: _t->d_func()->slotRegExpChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 14: _t->d_func()->slotEchoModeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 16: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        case 17: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        case 18: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        case 19: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        case 20: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        case 21: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QLocale(*)>(_a[2]))); break;
        case 22: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 23: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPointF(*)>(_a[2]))); break;
        case 24: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 25: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2])),(*reinterpret_cast< const QSize(*)>(_a[3]))); break;
        case 26: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 27: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2])),(*reinterpret_cast< const QSizeF(*)>(_a[3]))); break;
        case 28: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 29: _t->d_func()->slotConstraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 30: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 31: _t->d_func()->slotConstraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 32: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 33: _t->d_func()->slotEnumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 34: _t->d_func()->slotEnumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 35: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizePolicy(*)>(_a[2]))); break;
        case 36: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 37: _t->d_func()->slotValueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        case 38: _t->d_func()->slotFlagNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 39: _t->d_func()->slotPropertyInserted((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< QtProperty*(*)>(_a[3]))); break;
        case 40: _t->d_func()->slotPropertyRemoved((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtVariantPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtVariantPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtVariantPropertyManager,
      qt_meta_data_QtVariantPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtVariantPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtVariantPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtVariantPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtVariantPropertyManager))
        return static_cast<void*>(const_cast< QtVariantPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtVariantPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 41)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 41;
    }
    return _id;
}

// SIGNAL 0
void QtVariantPropertyManager::valueChanged(QtProperty * _t1, const QVariant & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtVariantPropertyManager::attributeChanged(QtProperty * _t1, const QString & _t2, const QVariant & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtVariantEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_QtVariantEditorFactory[] = {
    "QtVariantEditorFactory\0"
};

void QtVariantEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QtVariantEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtVariantEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtVariantPropertyManager>::staticMetaObject, qt_meta_stringdata_QtVariantEditorFactory,
      qt_meta_data_QtVariantEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtVariantEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtVariantEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtVariantEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtVariantEditorFactory))
        return static_cast<void*>(const_cast< QtVariantEditorFactory*>(this));
    return QtAbstractEditorFactory<QtVariantPropertyManager>::qt_metacast(_clname);
}

int QtVariantEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtVariantPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
