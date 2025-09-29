/****************************************************************************
** Meta object code from reading C++ file 'plugyitemssplitter.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.17)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/plugyitemssplitter.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'plugyitemssplitter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.17. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlugyItemsSplitter_t {
    QByteArrayData data[21];
    char stringdata0[282];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlugyItemsSplitter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlugyItemsSplitter_t qt_meta_stringdata_PlugyItemsSplitter = {
    {
QT_MOC_LITERAL(0, 0, 18), // "PlugyItemsSplitter"
QT_MOC_LITERAL(1, 19, 11), // "pageChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11), // "stashSorted"
QT_MOC_LITERAL(4, 44, 15), // "previous10Pages"
QT_MOC_LITERAL(5, 60, 12), // "previousPage"
QT_MOC_LITERAL(6, 73, 8), // "nextPage"
QT_MOC_LITERAL(7, 82, 11), // "next10Pages"
QT_MOC_LITERAL(8, 94, 16), // "previous100Pages"
QT_MOC_LITERAL(9, 111, 9), // "firstPage"
QT_MOC_LITERAL(10, 121, 8), // "lastPage"
QT_MOC_LITERAL(11, 130, 12), // "next100Pages"
QT_MOC_LITERAL(12, 143, 24), // "setApplyActionToAllPages"
QT_MOC_LITERAL(13, 168, 1), // "b"
QT_MOC_LITERAL(14, 170, 25), // "updateItemsForCurrentPage"
QT_MOC_LITERAL(15, 196, 12), // "pageChanged_"
QT_MOC_LITERAL(16, 209, 11), // "leftClicked"
QT_MOC_LITERAL(17, 221, 12), // "rightClicked"
QT_MOC_LITERAL(18, 234, 13), // "left10Clicked"
QT_MOC_LITERAL(19, 248, 14), // "right10Clicked"
QT_MOC_LITERAL(20, 263, 18) // "moveBetweenStashes"

    },
    "PlugyItemsSplitter\0pageChanged\0\0"
    "stashSorted\0previous10Pages\0previousPage\0"
    "nextPage\0next10Pages\0previous100Pages\0"
    "firstPage\0lastPage\0next100Pages\0"
    "setApplyActionToAllPages\0b\0"
    "updateItemsForCurrentPage\0pageChanged_\0"
    "leftClicked\0rightClicked\0left10Clicked\0"
    "right10Clicked\0moveBetweenStashes"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlugyItemsSplitter[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  104,    2, 0x06 /* Public */,
       3,    0,  105,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,  106,    2, 0x0a /* Public */,
       5,    0,  107,    2, 0x0a /* Public */,
       6,    0,  108,    2, 0x0a /* Public */,
       7,    0,  109,    2, 0x0a /* Public */,
       8,    0,  110,    2, 0x0a /* Public */,
       9,    0,  111,    2, 0x0a /* Public */,
      10,    0,  112,    2, 0x0a /* Public */,
      11,    0,  113,    2, 0x0a /* Public */,
      12,    1,  114,    2, 0x0a /* Public */,
      14,    1,  117,    2, 0x08 /* Private */,
      14,    0,  120,    2, 0x28 /* Private | MethodCloned */,
      16,    0,  121,    2, 0x08 /* Private */,
      17,    0,  122,    2, 0x08 /* Private */,
      18,    0,  123,    2, 0x08 /* Private */,
      19,    0,  124,    2, 0x08 /* Private */,
      20,    0,  125,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,

       0        // eod
};

void PlugyItemsSplitter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlugyItemsSplitter *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->pageChanged(); break;
        case 1: _t->stashSorted(); break;
        case 2: _t->previous10Pages(); break;
        case 3: _t->previousPage(); break;
        case 4: _t->nextPage(); break;
        case 5: _t->next10Pages(); break;
        case 6: _t->previous100Pages(); break;
        case 7: _t->firstPage(); break;
        case 8: _t->lastPage(); break;
        case 9: _t->next100Pages(); break;
        case 10: _t->setApplyActionToAllPages((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->updateItemsForCurrentPage((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->updateItemsForCurrentPage(); break;
        case 13: _t->leftClicked(); break;
        case 14: _t->rightClicked(); break;
        case 15: _t->left10Clicked(); break;
        case 16: _t->right10Clicked(); break;
        case 17: { bool _r = _t->moveBetweenStashes();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlugyItemsSplitter::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlugyItemsSplitter::pageChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlugyItemsSplitter::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlugyItemsSplitter::stashSorted)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PlugyItemsSplitter::staticMetaObject = { {
    QMetaObject::SuperData::link<ItemsPropertiesSplitter::staticMetaObject>(),
    qt_meta_stringdata_PlugyItemsSplitter.data,
    qt_meta_data_PlugyItemsSplitter,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlugyItemsSplitter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlugyItemsSplitter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlugyItemsSplitter.stringdata0))
        return static_cast<void*>(this);
    return ItemsPropertiesSplitter::qt_metacast(_clname);
}

int PlugyItemsSplitter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ItemsPropertiesSplitter::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void PlugyItemsSplitter::pageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PlugyItemsSplitter::stashSorted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
struct qt_meta_stringdata_MXLOTItemsSplitter_t {
    QByteArrayData data[1];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MXLOTItemsSplitter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MXLOTItemsSplitter_t qt_meta_stringdata_MXLOTItemsSplitter = {
    {
QT_MOC_LITERAL(0, 0, 18) // "MXLOTItemsSplitter"

    },
    "MXLOTItemsSplitter"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MXLOTItemsSplitter[] = {

 // content:
       8,       // revision
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

void MXLOTItemsSplitter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject MXLOTItemsSplitter::staticMetaObject = { {
    QMetaObject::SuperData::link<PlugyItemsSplitter::staticMetaObject>(),
    qt_meta_stringdata_MXLOTItemsSplitter.data,
    qt_meta_data_MXLOTItemsSplitter,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MXLOTItemsSplitter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MXLOTItemsSplitter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MXLOTItemsSplitter.stringdata0))
        return static_cast<void*>(this);
    return PlugyItemsSplitter::qt_metacast(_clname);
}

int MXLOTItemsSplitter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PlugyItemsSplitter::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
