/****************************************************************************
** Meta object code from reading C++ file 'qtpropertymanager.h'
**
** Created: Sun 7. Oct 20:28:30 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qtpropertymanager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtpropertymanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtGroupPropertyManager[] = {

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

static const char qt_meta_stringdata_QtGroupPropertyManager[] = {
    "QtGroupPropertyManager\0"
};

void QtGroupPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QtGroupPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtGroupPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtGroupPropertyManager,
      qt_meta_data_QtGroupPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtGroupPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtGroupPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtGroupPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtGroupPropertyManager))
        return static_cast<void*>(const_cast< QtGroupPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtGroupPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_QtIntPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      35,   22,   21,   21, 0x05,
      88,   65,   21,   21, 0x05,
     136,  122,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
     171,   22,   21,   21, 0x0a,
     213,  197,   21,   21, 0x0a,
     257,  241,   21,   21, 0x0a,
     285,   65,   21,   21, 0x0a,
     315,  122,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtIntPropertyManager[] = {
    "QtIntPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,int)\0"
    "property,minVal,maxVal\0"
    "rangeChanged(QtProperty*,int,int)\0"
    "property,step\0singleStepChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,int)\0property,minVal\0"
    "setMinimum(QtProperty*,int)\0property,maxVal\0"
    "setMaximum(QtProperty*,int)\0"
    "setRange(QtProperty*,int,int)\0"
    "setSingleStep(QtProperty*,int)\0"
};

void QtIntPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtIntPropertyManager *_t = static_cast<QtIntPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->singleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 7: _t->setSingleStep((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtIntPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtIntPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtIntPropertyManager,
      qt_meta_data_QtIntPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtIntPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtIntPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtIntPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtIntPropertyManager))
        return static_cast<void*>(const_cast< QtIntPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtIntPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtIntPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtIntPropertyManager::rangeChanged(QtProperty * _t1, int _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtIntPropertyManager::singleStepChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_QtBoolPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      67,   23,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtBoolPropertyManager[] = {
    "QtBoolPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,bool)\0"
    "setValue(QtProperty*,bool)\0"
};

void QtBoolPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtBoolPropertyManager *_t = static_cast<QtBoolPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtBoolPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtBoolPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtBoolPropertyManager,
      qt_meta_data_QtBoolPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtBoolPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtBoolPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtBoolPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtBoolPropertyManager))
        return static_cast<void*>(const_cast< QtBoolPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtBoolPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtBoolPropertyManager::valueChanged(QtProperty * _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtDoublePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,
      94,   71,   24,   24, 0x05,
     148,  134,   24,   24, 0x05,
     200,  186,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     233,   25,   24,   24, 0x0a,
     278,  262,   24,   24, 0x0a,
     325,  309,   24,   24, 0x0a,
     356,   71,   24,   24, 0x0a,
     392,  134,   24,   24, 0x0a,
     426,  186,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtDoublePropertyManager[] = {
    "QtDoublePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,double)\0"
    "property,minVal,maxVal\0"
    "rangeChanged(QtProperty*,double,double)\0"
    "property,step\0singleStepChanged(QtProperty*,double)\0"
    "property,prec\0decimalsChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,double)\0"
    "property,minVal\0setMinimum(QtProperty*,double)\0"
    "property,maxVal\0setMaximum(QtProperty*,double)\0"
    "setRange(QtProperty*,double,double)\0"
    "setSingleStep(QtProperty*,double)\0"
    "setDecimals(QtProperty*,int)\0"
};

void QtDoublePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDoublePropertyManager *_t = static_cast<QtDoublePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->singleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 5: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 6: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 7: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 8: _t->setSingleStep((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 9: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDoublePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDoublePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDoublePropertyManager,
      qt_meta_data_QtDoublePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDoublePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDoublePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDoublePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDoublePropertyManager))
        return static_cast<void*>(const_cast< QtDoublePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDoublePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QtDoublePropertyManager::valueChanged(QtProperty * _t1, double _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDoublePropertyManager::rangeChanged(QtProperty * _t1, double _t2, double _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDoublePropertyManager::singleStepChanged(QtProperty * _t1, double _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtDoublePropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
static const uint qt_meta_data_QtStringPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,
      88,   72,   24,   24, 0x05,
     133,  123,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     166,   25,   24,   24, 0x0a,
     196,   72,   24,   24, 0x0a,
     245,  227,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtStringPropertyManager[] = {
    "QtStringPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QString)\0"
    "property,regExp\0regExpChanged(QtProperty*,QRegExp)\0"
    "property,\0echoModeChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,QString)\0"
    "setRegExp(QtProperty*,QRegExp)\0"
    "property,echoMode\0setEchoMode(QtProperty*,EchoMode)\0"
};

void QtStringPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtStringPropertyManager *_t = static_cast<QtStringPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->regExpChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 2: _t->echoModeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 4: _t->setRegExp((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 5: _t->setEchoMode((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< EchoMode(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtStringPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtStringPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtStringPropertyManager,
      qt_meta_data_QtStringPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtStringPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtStringPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtStringPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtStringPropertyManager))
        return static_cast<void*>(const_cast< QtStringPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtStringPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtStringPropertyManager::valueChanged(QtProperty * _t1, const QString & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtStringPropertyManager::regExpChanged(QtProperty * _t1, const QRegExp & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtStringPropertyManager::echoModeChanged(QtProperty * _t1, const int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_QtDatePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      91,   68,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     129,   23,   22,   22, 0x0a,
     173,  157,   22,   22, 0x0a,
     219,  203,   22,   22, 0x0a,
     249,   68,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtDatePropertyManager[] = {
    "QtDatePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QDate)\0"
    "property,minVal,maxVal\0"
    "rangeChanged(QtProperty*,QDate,QDate)\0"
    "setValue(QtProperty*,QDate)\0property,minVal\0"
    "setMinimum(QtProperty*,QDate)\0"
    "property,maxVal\0setMaximum(QtProperty*,QDate)\0"
    "setRange(QtProperty*,QDate,QDate)\0"
};

void QtDatePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDatePropertyManager *_t = static_cast<QtDatePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 3: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 4: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 5: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDatePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDatePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDatePropertyManager,
      qt_meta_data_QtDatePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDatePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDatePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDatePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDatePropertyManager))
        return static_cast<void*>(const_cast< QtDatePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDatePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtDatePropertyManager::valueChanged(QtProperty * _t1, const QDate & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDatePropertyManager::rangeChanged(QtProperty * _t1, const QDate & _t2, const QDate & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtTimePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      68,   23,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtTimePropertyManager[] = {
    "QtTimePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QTime)\0"
    "setValue(QtProperty*,QTime)\0"
};

void QtTimePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtTimePropertyManager *_t = static_cast<QtTimePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtTimePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtTimePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtTimePropertyManager,
      qt_meta_data_QtTimePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtTimePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtTimePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtTimePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtTimePropertyManager))
        return static_cast<void*>(const_cast< QtTimePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtTimePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtTimePropertyManager::valueChanged(QtProperty * _t1, const QTime & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtDateTimePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      40,   27,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
      76,   27,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtDateTimePropertyManager[] = {
    "QtDateTimePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QDateTime)\0"
    "setValue(QtProperty*,QDateTime)\0"
};

void QtDateTimePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDateTimePropertyManager *_t = static_cast<QtDateTimePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDateTimePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDateTimePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtDateTimePropertyManager,
      qt_meta_data_QtDateTimePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDateTimePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDateTimePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDateTimePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateTimePropertyManager))
        return static_cast<void*>(const_cast< QtDateTimePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtDateTimePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtDateTimePropertyManager::valueChanged(QtProperty * _t1, const QDateTime & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtKeySequencePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      43,   30,   29,   29, 0x05,

 // slots: signature, parameters, type, tag, flags
      82,   30,   29,   29, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtKeySequencePropertyManager[] = {
    "QtKeySequencePropertyManager\0\0"
    "property,val\0valueChanged(QtProperty*,QKeySequence)\0"
    "setValue(QtProperty*,QKeySequence)\0"
};

void QtKeySequencePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtKeySequencePropertyManager *_t = static_cast<QtKeySequencePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtKeySequencePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtKeySequencePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtKeySequencePropertyManager,
      qt_meta_data_QtKeySequencePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtKeySequencePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtKeySequencePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtKeySequencePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtKeySequencePropertyManager))
        return static_cast<void*>(const_cast< QtKeySequencePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtKeySequencePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtKeySequencePropertyManager::valueChanged(QtProperty * _t1, const QKeySequence & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtCharPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      68,   23,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtCharPropertyManager[] = {
    "QtCharPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QChar)\0"
    "setValue(QtProperty*,QChar)\0"
};

void QtCharPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtCharPropertyManager *_t = static_cast<QtCharPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtCharPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtCharPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtCharPropertyManager,
      qt_meta_data_QtCharPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtCharPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtCharPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtCharPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCharPropertyManager))
        return static_cast<void*>(const_cast< QtCharPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtCharPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtCharPropertyManager::valueChanged(QtProperty * _t1, const QChar & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtLocalePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   25,   24,   24, 0x0a,
     104,  102,   24,   24, 0x08,
     137,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtLocalePropertyManager[] = {
    "QtLocalePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QLocale)\0"
    "setValue(QtProperty*,QLocale)\0,\0"
    "slotEnumChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtLocalePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtLocalePropertyManager *_t = static_cast<QtLocalePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QLocale(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QLocale(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtLocalePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtLocalePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtLocalePropertyManager,
      qt_meta_data_QtLocalePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtLocalePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtLocalePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtLocalePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtLocalePropertyManager))
        return static_cast<void*>(const_cast< QtLocalePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtLocalePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QtLocalePropertyManager::valueChanged(QtProperty * _t1, const QLocale & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtPointPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   24,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      70,   24,   23,   23, 0x0a,
     101,   99,   23,   23, 0x08,
     133,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtPointPropertyManager[] = {
    "QtPointPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QPoint)\0"
    "setValue(QtProperty*,QPoint)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtPointPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtPointPropertyManager *_t = static_cast<QtPointPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtPointPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtPointPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtPointPropertyManager,
      qt_meta_data_QtPointPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtPointPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtPointPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtPointPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtPointPropertyManager))
        return static_cast<void*>(const_cast< QtPointPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtPointPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QtPointPropertyManager::valueChanged(QtProperty * _t1, const QPoint & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtPointFPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,
      86,   72,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
     119,   25,   24,   24, 0x0a,
     149,   72,   24,   24, 0x0a,
     180,  178,   24,   24, 0x08,
     218,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtPointFPropertyManager[] = {
    "QtPointFPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QPointF)\0"
    "property,prec\0decimalsChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,QPointF)\0"
    "setDecimals(QtProperty*,int)\0,\0"
    "slotDoubleChanged(QtProperty*,double)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtPointFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtPointFPropertyManager *_t = static_cast<QtPointFPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPointF(*)>(_a[2]))); break;
        case 1: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QPointF(*)>(_a[2]))); break;
        case 3: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtPointFPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtPointFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtPointFPropertyManager,
      qt_meta_data_QtPointFPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtPointFPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtPointFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtPointFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtPointFPropertyManager))
        return static_cast<void*>(const_cast< QtPointFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtPointFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtPointFPropertyManager::valueChanged(QtProperty * _t1, const QPointF & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtPointFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtSizePropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      91,   68,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     129,   23,   22,   22, 0x0a,
     173,  157,   22,   22, 0x0a,
     219,  203,   22,   22, 0x0a,
     249,   68,   22,   22, 0x0a,
     285,  283,   22,   22, 0x08,
     317,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSizePropertyManager[] = {
    "QtSizePropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QSize)\0"
    "property,minVal,maxVal\0"
    "rangeChanged(QtProperty*,QSize,QSize)\0"
    "setValue(QtProperty*,QSize)\0property,minVal\0"
    "setMinimum(QtProperty*,QSize)\0"
    "property,maxVal\0setMaximum(QtProperty*,QSize)\0"
    "setRange(QtProperty*,QSize,QSize)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtSizePropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSizePropertyManager *_t = static_cast<QtSizePropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2])),(*reinterpret_cast< const QSize(*)>(_a[3]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 3: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 4: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2]))); break;
        case 5: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSize(*)>(_a[2])),(*reinterpret_cast< const QSize(*)>(_a[3]))); break;
        case 6: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSizePropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSizePropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizePropertyManager,
      qt_meta_data_QtSizePropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSizePropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSizePropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSizePropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizePropertyManager))
        return static_cast<void*>(const_cast< QtSizePropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizePropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtSizePropertyManager::valueChanged(QtProperty * _t1, const QSize & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtSizePropertyManager::rangeChanged(QtProperty * _t1, const QSize & _t2, const QSize & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtSizeFPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   24,   23,   23, 0x05,
      93,   70,   23,   23, 0x05,
     147,  133,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     180,   24,   23,   23, 0x0a,
     225,  209,   23,   23, 0x0a,
     272,  256,   23,   23, 0x0a,
     303,   70,   23,   23, 0x0a,
     339,  133,   23,   23, 0x0a,
     370,  368,   23,   23, 0x08,
     408,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSizeFPropertyManager[] = {
    "QtSizeFPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QSizeF)\0"
    "property,minVal,maxVal\0"
    "rangeChanged(QtProperty*,QSizeF,QSizeF)\0"
    "property,prec\0decimalsChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,QSizeF)\0"
    "property,minVal\0setMinimum(QtProperty*,QSizeF)\0"
    "property,maxVal\0setMaximum(QtProperty*,QSizeF)\0"
    "setRange(QtProperty*,QSizeF,QSizeF)\0"
    "setDecimals(QtProperty*,int)\0,\0"
    "slotDoubleChanged(QtProperty*,double)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtSizeFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSizeFPropertyManager *_t = static_cast<QtSizeFPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 1: _t->rangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2])),(*reinterpret_cast< const QSizeF(*)>(_a[3]))); break;
        case 2: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 4: _t->setMinimum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 5: _t->setMaximum((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2]))); break;
        case 6: _t->setRange((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizeF(*)>(_a[2])),(*reinterpret_cast< const QSizeF(*)>(_a[3]))); break;
        case 7: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 9: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSizeFPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSizeFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizeFPropertyManager,
      qt_meta_data_QtSizeFPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSizeFPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSizeFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSizeFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizeFPropertyManager))
        return static_cast<void*>(const_cast< QtSizeFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizeFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QtSizeFPropertyManager::valueChanged(QtProperty * _t1, const QSizeF & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtSizeFPropertyManager::rangeChanged(QtProperty * _t1, const QSizeF & _t2, const QSizeF & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtSizeFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_QtRectPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      88,   68,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     125,   23,   22,   22, 0x0a,
     153,   68,   22,   22, 0x0a,
     188,  186,   22,   22, 0x08,
     220,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtRectPropertyManager[] = {
    "QtRectPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QRect)\0"
    "property,constraint\0"
    "constraintChanged(QtProperty*,QRect)\0"
    "setValue(QtProperty*,QRect)\0"
    "setConstraint(QtProperty*,QRect)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtRectPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtRectPropertyManager *_t = static_cast<QtRectPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 1: _t->constraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 3: _t->setConstraint((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRect(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtRectPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtRectPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtRectPropertyManager,
      qt_meta_data_QtRectPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtRectPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtRectPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtRectPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtRectPropertyManager))
        return static_cast<void*>(const_cast< QtRectPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtRectPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtRectPropertyManager::valueChanged(QtProperty * _t1, const QRect & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtRectPropertyManager::constraintChanged(QtProperty * _t1, const QRect & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtRectFPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   24,   23,   23, 0x05,
      90,   70,   23,   23, 0x05,
     142,  128,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
     175,   24,   23,   23, 0x0a,
     204,   70,   23,   23, 0x0a,
     238,  128,   23,   23, 0x0a,
     269,  267,   23,   23, 0x08,
     307,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtRectFPropertyManager[] = {
    "QtRectFPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QRectF)\0"
    "property,constraint\0"
    "constraintChanged(QtProperty*,QRectF)\0"
    "property,prec\0decimalsChanged(QtProperty*,int)\0"
    "setValue(QtProperty*,QRectF)\0"
    "setConstraint(QtProperty*,QRectF)\0"
    "setDecimals(QtProperty*,int)\0,\0"
    "slotDoubleChanged(QtProperty*,double)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtRectFPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtRectFPropertyManager *_t = static_cast<QtRectFPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 1: _t->constraintChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 2: _t->decimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 4: _t->setConstraint((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRectF(*)>(_a[2]))); break;
        case 5: _t->setDecimals((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->d_func()->slotDoubleChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 7: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtRectFPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtRectFPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtRectFPropertyManager,
      qt_meta_data_QtRectFPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtRectFPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtRectFPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtRectFPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtRectFPropertyManager))
        return static_cast<void*>(const_cast< QtRectFPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtRectFPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtRectFPropertyManager::valueChanged(QtProperty * _t1, const QRectF & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtRectFPropertyManager::constraintChanged(QtProperty * _t1, const QRectF & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtRectFPropertyManager::decimalsChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_QtEnumPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      81,   66,   22,   22, 0x05,
     138,  123,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     184,   23,   22,   22, 0x0a,
     210,   66,   22,   22, 0x0a,
     248,  123,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtEnumPropertyManager[] = {
    "QtEnumPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,int)\0"
    "property,names\0enumNamesChanged(QtProperty*,QStringList)\0"
    "property,icons\0"
    "enumIconsChanged(QtProperty*,QMap<int,QIcon>)\0"
    "setValue(QtProperty*,int)\0"
    "setEnumNames(QtProperty*,QStringList)\0"
    "setEnumIcons(QtProperty*,QMap<int,QIcon>)\0"
};

void QtEnumPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtEnumPropertyManager *_t = static_cast<QtEnumPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->enumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->enumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 3: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->setEnumNames((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 5: _t->setEnumIcons((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtEnumPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtEnumPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtEnumPropertyManager,
      qt_meta_data_QtEnumPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtEnumPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtEnumPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtEnumPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtEnumPropertyManager))
        return static_cast<void*>(const_cast< QtEnumPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtEnumPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtEnumPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtEnumPropertyManager::enumNamesChanged(QtProperty * _t1, const QStringList & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtEnumPropertyManager::enumIconsChanged(QtProperty * _t1, const QMap<int,QIcon> & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_QtFlagPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,
      81,   66,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
     123,   23,   22,   22, 0x0a,
     149,   66,   22,   22, 0x0a,
     189,  187,   22,   22, 0x08,
     223,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtFlagPropertyManager[] = {
    "QtFlagPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,int)\0"
    "property,names\0flagNamesChanged(QtProperty*,QStringList)\0"
    "setValue(QtProperty*,int)\0"
    "setFlagNames(QtProperty*,QStringList)\0"
    ",\0slotBoolChanged(QtProperty*,bool)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtFlagPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtFlagPropertyManager *_t = static_cast<QtFlagPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->flagNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->setFlagNames((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotBoolChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtFlagPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtFlagPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtFlagPropertyManager,
      qt_meta_data_QtFlagPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtFlagPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtFlagPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtFlagPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtFlagPropertyManager))
        return static_cast<void*>(const_cast< QtFlagPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtFlagPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QtFlagPropertyManager::valueChanged(QtProperty * _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtFlagPropertyManager::flagNamesChanged(QtProperty * _t1, const QStringList & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_QtSizePolicyPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      42,   29,   28,   28, 0x05,

 // slots: signature, parameters, type, tag, flags
      80,   29,   28,   28, 0x0a,
     116,  114,   28,   28, 0x08,
     148,  114,   28,   28, 0x08,
     181,   28,   28,   28, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSizePolicyPropertyManager[] = {
    "QtSizePolicyPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QSizePolicy)\0"
    "setValue(QtProperty*,QSizePolicy)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotEnumChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtSizePolicyPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSizePolicyPropertyManager *_t = static_cast<QtSizePolicyPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizePolicy(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QSizePolicy(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSizePolicyPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSizePolicyPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtSizePolicyPropertyManager,
      qt_meta_data_QtSizePolicyPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSizePolicyPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSizePolicyPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSizePolicyPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSizePolicyPropertyManager))
        return static_cast<void*>(const_cast< QtSizePolicyPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtSizePolicyPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QtSizePolicyPropertyManager::valueChanged(QtProperty * _t1, const QSizePolicy & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtFontPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   23,   22,   22, 0x05,

 // slots: signature, parameters, type, tag, flags
      68,   23,   22,   22, 0x0a,
      98,   96,   22,   22, 0x08,
     130,   96,   22,   22, 0x08,
     163,   96,   22,   22, 0x08,
     197,   22,   22,   22, 0x08,
     232,   22,   22,   22, 0x08,
     258,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtFontPropertyManager[] = {
    "QtFontPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QFont)\0"
    "setValue(QtProperty*,QFont)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotEnumChanged(QtProperty*,int)\0"
    "slotBoolChanged(QtProperty*,bool)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
    "slotFontDatabaseChanged()\0"
    "slotFontDatabaseDelayedChange()\0"
};

void QtFontPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtFontPropertyManager *_t = static_cast<QtFontPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotBoolChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 6: _t->d_func()->slotFontDatabaseChanged(); break;
        case 7: _t->d_func()->slotFontDatabaseDelayedChange(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtFontPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtFontPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtFontPropertyManager,
      qt_meta_data_QtFontPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtFontPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtFontPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtFontPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtFontPropertyManager))
        return static_cast<void*>(const_cast< QtFontPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtFontPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QtFontPropertyManager::valueChanged(QtProperty * _t1, const QFont & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtColorPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   24,   23,   23, 0x05,

 // slots: signature, parameters, type, tag, flags
      70,   24,   23,   23, 0x0a,
     101,   99,   23,   23, 0x08,
     133,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtColorPropertyManager[] = {
    "QtColorPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QColor)\0"
    "setValue(QtProperty*,QColor)\0,\0"
    "slotIntChanged(QtProperty*,int)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
};

void QtColorPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtColorPropertyManager *_t = static_cast<QtColorPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotIntChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtColorPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtColorPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtColorPropertyManager,
      qt_meta_data_QtColorPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtColorPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtColorPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtColorPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtColorPropertyManager))
        return static_cast<void*>(const_cast< QtColorPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtColorPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void QtColorPropertyManager::valueChanged(QtProperty * _t1, const QColor & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_QtCursorPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      38,   25,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      72,   25,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtCursorPropertyManager[] = {
    "QtCursorPropertyManager\0\0property,val\0"
    "valueChanged(QtProperty*,QCursor)\0"
    "setValue(QtProperty*,QCursor)\0"
};

void QtCursorPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtCursorPropertyManager *_t = static_cast<QtCursorPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        case 1: _t->setValue((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtCursorPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtCursorPropertyManager::staticMetaObject = {
    { &QtAbstractPropertyManager::staticMetaObject, qt_meta_stringdata_QtCursorPropertyManager,
      qt_meta_data_QtCursorPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtCursorPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtCursorPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtCursorPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCursorPropertyManager))
        return static_cast<void*>(const_cast< QtCursorPropertyManager*>(this));
    return QtAbstractPropertyManager::qt_metacast(_clname);
}

int QtCursorPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractPropertyManager::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void QtCursorPropertyManager::valueChanged(QtProperty * _t1, const QCursor & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
