/****************************************************************************
** Meta object code from reading C++ file 'qteditorfactory.h'
**
** Created: Sun 7. Oct 20:28:05 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qteditorfactory.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qteditorfactory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtSpinBoxFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   18,   17,   17, 0x08,
      60,   57,   17,   17, 0x08,
      98,   18,   17,   17, 0x08,
     137,   17,   17,   17, 0x08,
     155,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSpinBoxFactory[] = {
    "QtSpinBoxFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,int)\0"
    ",,\0slotRangeChanged(QtProperty*,int,int)\0"
    "slotSingleStepChanged(QtProperty*,int)\0"
    "slotSetValue(int)\0slotEditorDestroyed(QObject*)\0"
};

void QtSpinBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSpinBoxFactory *_t = static_cast<QtSpinBoxFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSpinBoxFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSpinBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtSpinBoxFactory,
      qt_meta_data_QtSpinBoxFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSpinBoxFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSpinBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSpinBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSpinBoxFactory))
        return static_cast<void*>(const_cast< QtSpinBoxFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtSpinBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_QtSliderFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   17,   16,   16, 0x08,
      59,   56,   16,   16, 0x08,
      97,   17,   16,   16, 0x08,
     136,   16,   16,   16, 0x08,
     154,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtSliderFactory[] = {
    "QtSliderFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,int)\0"
    ",,\0slotRangeChanged(QtProperty*,int,int)\0"
    "slotSingleStepChanged(QtProperty*,int)\0"
    "slotSetValue(int)\0slotEditorDestroyed(QObject*)\0"
};

void QtSliderFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtSliderFactory *_t = static_cast<QtSliderFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtSliderFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtSliderFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtSliderFactory,
      qt_meta_data_QtSliderFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtSliderFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtSliderFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtSliderFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtSliderFactory))
        return static_cast<void*>(const_cast< QtSliderFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtSliderFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_QtScrollBarFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   20,   19,   19, 0x08,
      62,   59,   19,   19, 0x08,
     100,   20,   19,   19, 0x08,
     139,   19,   19,   19, 0x08,
     157,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtScrollBarFactory[] = {
    "QtScrollBarFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,int)\0"
    ",,\0slotRangeChanged(QtProperty*,int,int)\0"
    "slotSingleStepChanged(QtProperty*,int)\0"
    "slotSetValue(int)\0slotEditorDestroyed(QObject*)\0"
};

void QtScrollBarFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtScrollBarFactory *_t = static_cast<QtScrollBarFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtScrollBarFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtScrollBarFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtIntPropertyManager>::staticMetaObject, qt_meta_stringdata_QtScrollBarFactory,
      qt_meta_data_QtScrollBarFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtScrollBarFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtScrollBarFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtScrollBarFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtScrollBarFactory))
        return static_cast<void*>(const_cast< QtScrollBarFactory*>(this));
    return QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacast(_clname);
}

int QtScrollBarFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtIntPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_QtCheckBoxFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   19,   18,   18, 0x08,
      59,   18,   18,   18, 0x08,
      78,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtCheckBoxFactory[] = {
    "QtCheckBoxFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,bool)\0"
    "slotSetValue(bool)\0slotEditorDestroyed(QObject*)\0"
};

void QtCheckBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtCheckBoxFactory *_t = static_cast<QtCheckBoxFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtCheckBoxFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtCheckBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtBoolPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCheckBoxFactory,
      qt_meta_data_QtCheckBoxFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtCheckBoxFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtCheckBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtCheckBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCheckBoxFactory))
        return static_cast<void*>(const_cast< QtCheckBoxFactory*>(this));
    return QtAbstractEditorFactory<QtBoolPropertyManager>::qt_metacast(_clname);
}

int QtCheckBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtBoolPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtDoubleSpinBoxFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   24,   23,   23, 0x08,
      69,   66,   23,   23, 0x08,
     113,   24,   23,   23, 0x08,
     155,   24,   23,   23, 0x08,
     192,   23,   23,   23, 0x08,
     213,   23,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtDoubleSpinBoxFactory[] = {
    "QtDoubleSpinBoxFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,double)\0"
    ",,\0slotRangeChanged(QtProperty*,double,double)\0"
    "slotSingleStepChanged(QtProperty*,double)\0"
    "slotDecimalsChanged(QtProperty*,int)\0"
    "slotSetValue(double)\0slotEditorDestroyed(QObject*)\0"
};

void QtDoubleSpinBoxFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDoubleSpinBoxFactory *_t = static_cast<QtDoubleSpinBoxFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSingleStepChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotDecimalsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->d_func()->slotSetValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDoubleSpinBoxFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDoubleSpinBoxFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDoublePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDoubleSpinBoxFactory,
      qt_meta_data_QtDoubleSpinBoxFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDoubleSpinBoxFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDoubleSpinBoxFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDoubleSpinBoxFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDoubleSpinBoxFactory))
        return static_cast<void*>(const_cast< QtDoubleSpinBoxFactory*>(this));
    return QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacast(_clname);
}

int QtDoubleSpinBoxFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDoublePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
static const uint qt_meta_data_QtLineEditFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   19,   18,   18, 0x08,
      62,   19,   18,   18, 0x08,
     101,   19,   18,   18, 0x08,
     138,   18,   18,   18, 0x08,
     160,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtLineEditFactory[] = {
    "QtLineEditFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QString)\0"
    "slotRegExpChanged(QtProperty*,QRegExp)\0"
    "slotEchoModeChanged(QtProperty*,int)\0"
    "slotSetValue(QString)\0"
    "slotEditorDestroyed(QObject*)\0"
};

void QtLineEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtLineEditFactory *_t = static_cast<QtLineEditFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRegExpChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QRegExp(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEchoModeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtLineEditFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtLineEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtStringPropertyManager>::staticMetaObject, qt_meta_stringdata_QtLineEditFactory,
      qt_meta_data_QtLineEditFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtLineEditFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtLineEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtLineEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtLineEditFactory))
        return static_cast<void*>(const_cast< QtLineEditFactory*>(this));
    return QtAbstractEditorFactory<QtStringPropertyManager>::qt_metacast(_clname);
}

int QtLineEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtStringPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_QtDateEditFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   19,   18,   18, 0x08,
      63,   60,   18,   18, 0x08,
     105,   18,   18,   18, 0x08,
     125,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtDateEditFactory[] = {
    "QtDateEditFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QDate)\0"
    ",,\0slotRangeChanged(QtProperty*,QDate,QDate)\0"
    "slotSetValue(QDate)\0slotEditorDestroyed(QObject*)\0"
};

void QtDateEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDateEditFactory *_t = static_cast<QtDateEditFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotRangeChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDate(*)>(_a[2])),(*reinterpret_cast< const QDate(*)>(_a[3]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QDate(*)>(_a[1]))); break;
        case 3: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDateEditFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDateEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDatePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDateEditFactory,
      qt_meta_data_QtDateEditFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDateEditFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDateEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDateEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateEditFactory))
        return static_cast<void*>(const_cast< QtDateEditFactory*>(this));
    return QtAbstractEditorFactory<QtDatePropertyManager>::qt_metacast(_clname);
}

int QtDateEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDatePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
static const uint qt_meta_data_QtTimeEditFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   19,   18,   18, 0x08,
      60,   18,   18,   18, 0x08,
      80,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtTimeEditFactory[] = {
    "QtTimeEditFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QTime)\0"
    "slotSetValue(QTime)\0slotEditorDestroyed(QObject*)\0"
};

void QtTimeEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtTimeEditFactory *_t = static_cast<QtTimeEditFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QTime(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QTime(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtTimeEditFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtTimeEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtTimePropertyManager>::staticMetaObject, qt_meta_stringdata_QtTimeEditFactory,
      qt_meta_data_QtTimeEditFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtTimeEditFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtTimeEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtTimeEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtTimeEditFactory))
        return static_cast<void*>(const_cast< QtTimeEditFactory*>(this));
    return QtAbstractEditorFactory<QtTimePropertyManager>::qt_metacast(_clname);
}

int QtTimeEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtTimePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtDateTimeEditFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   23,   22,   22, 0x08,
      68,   22,   22,   22, 0x08,
      92,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtDateTimeEditFactory[] = {
    "QtDateTimeEditFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QDateTime)\0"
    "slotSetValue(QDateTime)\0"
    "slotEditorDestroyed(QObject*)\0"
};

void QtDateTimeEditFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtDateTimeEditFactory *_t = static_cast<QtDateTimeEditFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QDateTime(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtDateTimeEditFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtDateTimeEditFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtDateTimePropertyManager>::staticMetaObject, qt_meta_stringdata_QtDateTimeEditFactory,
      qt_meta_data_QtDateTimeEditFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtDateTimeEditFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtDateTimeEditFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtDateTimeEditFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtDateTimeEditFactory))
        return static_cast<void*>(const_cast< QtDateTimeEditFactory*>(this));
    return QtAbstractEditorFactory<QtDateTimePropertyManager>::qt_metacast(_clname);
}

int QtDateTimeEditFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtDateTimePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtKeySequenceEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   28,   27,   27, 0x08,
      76,   27,   27,   27, 0x08,
     103,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtKeySequenceEditorFactory[] = {
    "QtKeySequenceEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QKeySequence)\0"
    "slotSetValue(QKeySequence)\0"
    "slotEditorDestroyed(QObject*)\0"
};

void QtKeySequenceEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtKeySequenceEditorFactory *_t = static_cast<QtKeySequenceEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QKeySequence(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QKeySequence(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtKeySequenceEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtKeySequenceEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtKeySequencePropertyManager>::staticMetaObject, qt_meta_stringdata_QtKeySequenceEditorFactory,
      qt_meta_data_QtKeySequenceEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtKeySequenceEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtKeySequenceEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtKeySequenceEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtKeySequenceEditorFactory))
        return static_cast<void*>(const_cast< QtKeySequenceEditorFactory*>(this));
    return QtAbstractEditorFactory<QtKeySequencePropertyManager>::qt_metacast(_clname);
}

int QtKeySequenceEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtKeySequencePropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtCharEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   21,   20,   20, 0x08,
      62,   20,   20,   20, 0x08,
      82,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtCharEditorFactory[] = {
    "QtCharEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QChar)\0"
    "slotSetValue(QChar)\0slotEditorDestroyed(QObject*)\0"
};

void QtCharEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtCharEditorFactory *_t = static_cast<QtCharEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QChar(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotSetValue((*reinterpret_cast< const QChar(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtCharEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtCharEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtCharPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCharEditorFactory,
      qt_meta_data_QtCharEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtCharEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtCharEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtCharEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCharEditorFactory))
        return static_cast<void*>(const_cast< QtCharEditorFactory*>(this));
    return QtAbstractEditorFactory<QtCharPropertyManager>::qt_metacast(_clname);
}

int QtCharEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtCharPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtEnumEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   21,   20,   20, 0x08,
      60,   21,   20,   20, 0x08,
     106,   21,   20,   20, 0x08,
     156,   20,   20,   20, 0x08,
     174,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtEnumEditorFactory[] = {
    "QtEnumEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,int)\0"
    "slotEnumNamesChanged(QtProperty*,QStringList)\0"
    "slotEnumIconsChanged(QtProperty*,QMap<int,QIcon>)\0"
    "slotSetValue(int)\0slotEditorDestroyed(QObject*)\0"
};

void QtEnumEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtEnumEditorFactory *_t = static_cast<QtEnumEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEnumNamesChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QStringList(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEnumIconsChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QMap<int,QIcon>(*)>(_a[2]))); break;
        case 3: _t->d_func()->slotSetValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtEnumEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtEnumEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtEnumPropertyManager>::staticMetaObject, qt_meta_stringdata_QtEnumEditorFactory,
      qt_meta_data_QtEnumEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtEnumEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtEnumEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtEnumEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtEnumEditorFactory))
        return static_cast<void*>(const_cast< QtEnumEditorFactory*>(this));
    return QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacast(_clname);
}

int QtEnumEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtEnumPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_QtCursorEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   23,   22,   22, 0x08,
      66,   23,   22,   22, 0x08,
      99,   22,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtCursorEditorFactory[] = {
    "QtCursorEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QCursor)\0"
    "slotEnumChanged(QtProperty*,int)\0"
    "slotEditorDestroyed(QObject*)\0"
};

void QtCursorEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtCursorEditorFactory *_t = static_cast<QtCursorEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QCursor(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEnumChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtCursorEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtCursorEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtCursorPropertyManager>::staticMetaObject, qt_meta_stringdata_QtCursorEditorFactory,
      qt_meta_data_QtCursorEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtCursorEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtCursorEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtCursorEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtCursorEditorFactory))
        return static_cast<void*>(const_cast< QtCursorEditorFactory*>(this));
    return QtAbstractEditorFactory<QtCursorPropertyManager>::qt_metacast(_clname);
}

int QtCursorEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtCursorPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtColorEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   22,   21,   21, 0x08,
      64,   21,   21,   21, 0x08,
      94,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtColorEditorFactory[] = {
    "QtColorEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QColor)\0"
    "slotEditorDestroyed(QObject*)\0"
    "slotSetValue(QColor)\0"
};

void QtColorEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtColorEditorFactory *_t = static_cast<QtColorEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtColorEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtColorEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtColorPropertyManager>::staticMetaObject, qt_meta_stringdata_QtColorEditorFactory,
      qt_meta_data_QtColorEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtColorEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtColorEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtColorEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtColorEditorFactory))
        return static_cast<void*>(const_cast< QtColorEditorFactory*>(this));
    return QtAbstractEditorFactory<QtColorPropertyManager>::qt_metacast(_clname);
}

int QtColorEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtColorPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
static const uint qt_meta_data_QtFontEditorFactory[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   21,   20,   20, 0x08,
      62,   20,   20,   20, 0x08,
      92,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtFontEditorFactory[] = {
    "QtFontEditorFactory\0\0,\0"
    "slotPropertyChanged(QtProperty*,QFont)\0"
    "slotEditorDestroyed(QObject*)\0"
    "slotSetValue(QFont)\0"
};

void QtFontEditorFactory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtFontEditorFactory *_t = static_cast<QtFontEditorFactory *>(_o);
        switch (_id) {
        case 0: _t->d_func()->slotPropertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< const QFont(*)>(_a[2]))); break;
        case 1: _t->d_func()->slotEditorDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 2: _t->d_func()->slotSetValue((*reinterpret_cast< const QFont(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtFontEditorFactory::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtFontEditorFactory::staticMetaObject = {
    { &QtAbstractEditorFactory<QtFontPropertyManager>::staticMetaObject, qt_meta_stringdata_QtFontEditorFactory,
      qt_meta_data_QtFontEditorFactory, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtFontEditorFactory::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtFontEditorFactory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtFontEditorFactory::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtFontEditorFactory))
        return static_cast<void*>(const_cast< QtFontEditorFactory*>(this));
    return QtAbstractEditorFactory<QtFontPropertyManager>::qt_metacast(_clname);
}

int QtFontEditorFactory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtAbstractEditorFactory<QtFontPropertyManager>::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
