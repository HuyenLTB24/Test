/****************************************************************************
** Meta object code from reading C++ file 'account_manager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "account_manager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'account_manager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AccountDialog_t {
    QByteArrayData data[7];
    char stringdata0[73];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AccountDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AccountDialog_t qt_meta_stringdata_AccountDialog = {
    {
QT_MOC_LITERAL(0, 0, 13), // "AccountDialog"
QT_MOC_LITERAL(1, 14, 8), // "onAccept"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 8), // "onReject"
QT_MOC_LITERAL(4, 33, 14), // "testConnection"
QT_MOC_LITERAL(5, 48, 10), // "showApiKey"
QT_MOC_LITERAL(6, 59, 13) // "showApiSecret"

    },
    "AccountDialog\0onAccept\0\0onReject\0"
    "testConnection\0showApiKey\0showApiSecret"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AccountDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    0,   41,    2, 0x08 /* Private */,
       5,    0,   42,    2, 0x08 /* Private */,
       6,    0,   43,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AccountDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AccountDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onAccept(); break;
        case 1: _t->onReject(); break;
        case 2: _t->testConnection(); break;
        case 3: _t->showApiKey(); break;
        case 4: _t->showApiSecret(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject AccountDialog::staticMetaObject = { {
    &QDialog::staticMetaObject,
    qt_meta_stringdata_AccountDialog.data,
    qt_meta_data_AccountDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AccountDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AccountDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AccountDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int AccountDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
struct qt_meta_stringdata_AccountManager_t {
    QByteArrayData data[12];
    char stringdata0[167];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AccountManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AccountManager_t qt_meta_stringdata_AccountManager = {
    {
QT_MOC_LITERAL(0, 0, 14), // "AccountManager"
QT_MOC_LITERAL(1, 15, 14), // "accountChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 9), // "profileId"
QT_MOC_LITERAL(4, 41, 15), // "refreshAccounts"
QT_MOC_LITERAL(5, 57, 10), // "addAccount"
QT_MOC_LITERAL(6, 68, 11), // "editAccount"
QT_MOC_LITERAL(7, 80, 13), // "deleteAccount"
QT_MOC_LITERAL(8, 94, 16), // "duplicateAccount"
QT_MOC_LITERAL(9, 111, 14), // "exportAccounts"
QT_MOC_LITERAL(10, 126, 14), // "importAccounts"
QT_MOC_LITERAL(11, 141, 25) // "onAccountSelectionChanged"

    },
    "AccountManager\0accountChanged\0\0profileId\0"
    "refreshAccounts\0addAccount\0editAccount\0"
    "deleteAccount\0duplicateAccount\0"
    "exportAccounts\0importAccounts\0"
    "onAccountSelectionChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AccountManager[] = {

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
       4,    0,   62,    2, 0x0a /* Public */,
       5,    0,   63,    2, 0x08 /* Private */,
       6,    0,   64,    2, 0x08 /* Private */,
       7,    0,   65,    2, 0x08 /* Private */,
       8,    0,   66,    2, 0x08 /* Private */,
       9,    0,   67,    2, 0x08 /* Private */,
      10,    0,   68,    2, 0x08 /* Private */,
      11,    0,   69,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AccountManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AccountManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->accountChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->refreshAccounts(); break;
        case 2: _t->addAccount(); break;
        case 3: _t->editAccount(); break;
        case 4: _t->deleteAccount(); break;
        case 5: _t->duplicateAccount(); break;
        case 6: _t->exportAccounts(); break;
        case 7: _t->importAccounts(); break;
        case 8: _t->onAccountSelectionChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AccountManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AccountManager::accountChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AccountManager::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_AccountManager.data,
    qt_meta_data_AccountManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AccountManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AccountManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AccountManager.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AccountManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void AccountManager::accountChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
