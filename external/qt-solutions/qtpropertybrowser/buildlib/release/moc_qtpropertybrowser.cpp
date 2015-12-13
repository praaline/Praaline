/****************************************************************************
** Meta object code from reading C++ file 'qtpropertybrowser.h'
**
** Created: Sun 7. Oct 20:28:27 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qtpropertybrowser.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtpropertybrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtAbstractPropertyManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      49,   27,   26,   26, 0x05,
     112,  103,   26,   26, 0x05,
     157,  141,   26,   26, 0x05,
     198,  103,   26,   26, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_QtAbstractPropertyManager[] = {
    "QtAbstractPropertyManager\0\0"
    "property,parent,after\0"
    "propertyInserted(QtProperty*,QtProperty*,QtProperty*)\0"
    "property\0propertyChanged(QtProperty*)\0"
    "property,parent\0"
    "propertyRemoved(QtProperty*,QtProperty*)\0"
    "propertyDestroyed(QtProperty*)\0"
};

void QtAbstractPropertyManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtAbstractPropertyManager *_t = static_cast<QtAbstractPropertyManager *>(_o);
        switch (_id) {
        case 0: _t->propertyInserted((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< QtProperty*(*)>(_a[3]))); break;
        case 1: _t->propertyChanged((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 2: _t->propertyRemoved((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2]))); break;
        case 3: _t->propertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtAbstractPropertyManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtAbstractPropertyManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtAbstractPropertyManager,
      qt_meta_data_QtAbstractPropertyManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtAbstractPropertyManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtAbstractPropertyManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtAbstractPropertyManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractPropertyManager))
        return static_cast<void*>(const_cast< QtAbstractPropertyManager*>(this));
    return QObject::qt_metacast(_clname);
}

int QtAbstractPropertyManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void QtAbstractPropertyManager::propertyInserted(QtProperty * _t1, QtProperty * _t2, QtProperty * _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtAbstractPropertyManager::propertyChanged(QtProperty * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtAbstractPropertyManager::propertyRemoved(QtProperty * _t1, QtProperty * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtAbstractPropertyManager::propertyDestroyed(QtProperty * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
static const uint qt_meta_data_QtAbstractEditorFactoryBase[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      37,   29,   28,   28, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_QtAbstractEditorFactoryBase[] = {
    "QtAbstractEditorFactoryBase\0\0manager\0"
    "managerDestroyed(QObject*)\0"
};

void QtAbstractEditorFactoryBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtAbstractEditorFactoryBase *_t = static_cast<QtAbstractEditorFactoryBase *>(_o);
        switch (_id) {
        case 0: _t->managerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtAbstractEditorFactoryBase::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtAbstractEditorFactoryBase::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtAbstractEditorFactoryBase,
      qt_meta_data_QtAbstractEditorFactoryBase, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtAbstractEditorFactoryBase::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtAbstractEditorFactoryBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtAbstractEditorFactoryBase::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractEditorFactoryBase))
        return static_cast<void*>(const_cast< QtAbstractEditorFactoryBase*>(this));
    return QObject::qt_metacast(_clname);
}

int QtAbstractEditorFactoryBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_QtAbstractPropertyBrowser[] = {

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
      27,   26,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   77,   62,   26, 0x0a,
     134,  111,   62,   26, 0x0a,
     174,   77,   26,   26, 0x0a,
     205,  202,   26,   26, 0x08,
     265,  263,   26,   26, 0x08,
     310,   26,   26,   26, 0x08,
     345,   26,   26,   26, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QtAbstractPropertyBrowser[] = {
    "QtAbstractPropertyBrowser\0\0"
    "currentItemChanged(QtBrowserItem*)\0"
    "QtBrowserItem*\0property\0"
    "addProperty(QtProperty*)\0"
    "property,afterProperty\0"
    "insertProperty(QtProperty*,QtProperty*)\0"
    "removeProperty(QtProperty*)\0,,\0"
    "slotPropertyInserted(QtProperty*,QtProperty*,QtProperty*)\0"
    ",\0slotPropertyRemoved(QtProperty*,QtProperty*)\0"
    "slotPropertyDestroyed(QtProperty*)\0"
    "slotPropertyDataChanged(QtProperty*)\0"
};

void QtAbstractPropertyBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QtAbstractPropertyBrowser *_t = static_cast<QtAbstractPropertyBrowser *>(_o);
        switch (_id) {
        case 0: _t->currentItemChanged((*reinterpret_cast< QtBrowserItem*(*)>(_a[1]))); break;
        case 1: { QtBrowserItem* _r = _t->addProperty((*reinterpret_cast< QtProperty*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QtBrowserItem**>(_a[0]) = _r; }  break;
        case 2: { QtBrowserItem* _r = _t->insertProperty((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< QtBrowserItem**>(_a[0]) = _r; }  break;
        case 3: _t->removeProperty((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 4: _t->d_func()->slotPropertyInserted((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2])),(*reinterpret_cast< QtProperty*(*)>(_a[3]))); break;
        case 5: _t->d_func()->slotPropertyRemoved((*reinterpret_cast< QtProperty*(*)>(_a[1])),(*reinterpret_cast< QtProperty*(*)>(_a[2]))); break;
        case 6: _t->d_func()->slotPropertyDestroyed((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        case 7: _t->d_func()->slotPropertyDataChanged((*reinterpret_cast< QtProperty*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QtAbstractPropertyBrowser::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QtAbstractPropertyBrowser::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QtAbstractPropertyBrowser,
      qt_meta_data_QtAbstractPropertyBrowser, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QtAbstractPropertyBrowser::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QtAbstractPropertyBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QtAbstractPropertyBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtAbstractPropertyBrowser))
        return static_cast<void*>(const_cast< QtAbstractPropertyBrowser*>(this));
    return QWidget::qt_metacast(_clname);
}

int QtAbstractPropertyBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void QtAbstractPropertyBrowser::currentItemChanged(QtBrowserItem * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
