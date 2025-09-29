/****************************************************************************
** Meta object code from reading C++ file 'finditemsdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.17)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/finditemsdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'finditemsdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.17. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FindItemsDialog_t {
    QByteArrayData data[14];
    char stringdata0[193];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FindItemsDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FindItemsDialog_t qt_meta_stringdata_FindItemsDialog = {
    {
QT_MOC_LITERAL(0, 0, 15), // "FindItemsDialog"
QT_MOC_LITERAL(1, 16, 9), // "itemFound"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 9), // "ItemInfo*"
QT_MOC_LITERAL(4, 37, 4), // "item"
QT_MOC_LITERAL(5, 42, 17), // "resetSearchStatus"
QT_MOC_LITERAL(6, 60, 6), // "reject"
QT_MOC_LITERAL(7, 67, 8), // "findNext"
QT_MOC_LITERAL(8, 76, 12), // "findPrevious"
QT_MOC_LITERAL(9, 89, 13), // "toggleResults"
QT_MOC_LITERAL(10, 103, 25), // "updateCurrentIndexForItem"
QT_MOC_LITERAL(11, 129, 17), // "searchTextChanged"
QT_MOC_LITERAL(12, 147, 29), // "changeComboboxCaseSensitivity"
QT_MOC_LITERAL(13, 177, 15) // "isCaseSensitive"

    },
    "FindItemsDialog\0itemFound\0\0ItemInfo*\0"
    "item\0resetSearchStatus\0reject\0findNext\0"
    "findPrevious\0toggleResults\0"
    "updateCurrentIndexForItem\0searchTextChanged\0"
    "changeComboboxCaseSensitivity\0"
    "isCaseSensitive"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FindItemsDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   62,    2, 0x0a /* Public */,
       6,    0,   63,    2, 0x0a /* Public */,
       7,    0,   64,    2, 0x08 /* Private */,
       8,    0,   65,    2, 0x08 /* Private */,
       9,    0,   66,    2, 0x08 /* Private */,
      10,    1,   67,    2, 0x08 /* Private */,
      11,    0,   70,    2, 0x08 /* Private */,
      12,    1,   71,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,

       0        // eod
};

void FindItemsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FindItemsDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->itemFound((*reinterpret_cast< ItemInfo*(*)>(_a[1]))); break;
        case 1: _t->resetSearchStatus(); break;
        case 2: _t->reject(); break;
        case 3: _t->findNext(); break;
        case 4: _t->findPrevious(); break;
        case 5: _t->toggleResults(); break;
        case 6: _t->updateCurrentIndexForItem((*reinterpret_cast< ItemInfo*(*)>(_a[1]))); break;
        case 7: _t->searchTextChanged(); break;
        case 8: _t->changeComboboxCaseSensitivity((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FindItemsDialog::*)(ItemInfo * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FindItemsDialog::itemFound)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FindItemsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_FindItemsDialog.data,
    qt_meta_data_FindItemsDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FindItemsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FindItemsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FindItemsDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FindItemsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void FindItemsDialog::itemFound(ItemInfo * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
