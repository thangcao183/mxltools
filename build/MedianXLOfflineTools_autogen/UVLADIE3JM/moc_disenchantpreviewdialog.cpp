/****************************************************************************
** Meta object code from reading C++ file 'disenchantpreviewdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/disenchantpreviewdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'disenchantpreviewdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DisenchantPreviewDialog_t {
    QByteArrayData data[8];
    char stringdata0[122];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DisenchantPreviewDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DisenchantPreviewDialog_t qt_meta_stringdata_DisenchantPreviewDialog = {
    {
QT_MOC_LITERAL(0, 0, 23), // "DisenchantPreviewDialog"
QT_MOC_LITERAL(1, 24, 4), // "done"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 1), // "r"
QT_MOC_LITERAL(4, 32, 23), // "showTreeViewContextMenu"
QT_MOC_LITERAL(5, 56, 3), // "pos"
QT_MOC_LITERAL(6, 60, 29), // "changeSelectedItemsCheckState"
QT_MOC_LITERAL(7, 90, 31) // "updateLabelTextAndOkButtonState"

    },
    "DisenchantPreviewDialog\0done\0\0r\0"
    "showTreeViewContextMenu\0pos\0"
    "changeSelectedItemsCheckState\0"
    "updateLabelTextAndOkButtonState"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DisenchantPreviewDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x0a /* Public */,
       4,    1,   37,    2, 0x08 /* Private */,
       6,    0,   40,    2, 0x08 /* Private */,
       7,    0,   41,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QPoint,    5,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DisenchantPreviewDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DisenchantPreviewDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->done((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->showTreeViewContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 2: _t->changeSelectedItemsCheckState(); break;
        case 3: _t->updateLabelTextAndOkButtonState(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DisenchantPreviewDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_DisenchantPreviewDialog.data,
    qt_meta_data_DisenchantPreviewDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DisenchantPreviewDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DisenchantPreviewDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DisenchantPreviewDialog.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "ShowSelectedItemInterface"))
        return static_cast< ShowSelectedItemInterface*>(this);
    return QDialog::qt_metacast(_clname);
}

int DisenchantPreviewDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
