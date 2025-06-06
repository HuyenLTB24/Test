/****************************************************************************
** Meta object code from reading C++ file 'bot_worker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "bot_worker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bot_worker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BotWorker_t {
    QByteArrayData data[21];
    char stringdata0[237];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BotWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BotWorker_t qt_meta_stringdata_BotWorker = {
    {
QT_MOC_LITERAL(0, 0, 9), // "BotWorker"
QT_MOC_LITERAL(1, 10, 9), // "logSignal"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 7), // "message"
QT_MOC_LITERAL(4, 29, 20), // "tweetProcessedSignal"
QT_MOC_LITERAL(5, 50, 7), // "timeStr"
QT_MOC_LITERAL(6, 58, 8), // "username"
QT_MOC_LITERAL(7, 67, 8), // "tweetUrl"
QT_MOC_LITERAL(8, 76, 9), // "replyText"
QT_MOC_LITERAL(9, 86, 6), // "status"
QT_MOC_LITERAL(10, 93, 11), // "likeSuccess"
QT_MOC_LITERAL(11, 105, 13), // "followSuccess"
QT_MOC_LITERAL(12, 119, 14), // "retweetSuccess"
QT_MOC_LITERAL(13, 134, 10), // "replyCount"
QT_MOC_LITERAL(14, 145, 9), // "viewCount"
QT_MOC_LITERAL(15, 155, 7), // "tweetId"
QT_MOC_LITERAL(16, 163, 15), // "originalContent"
QT_MOC_LITERAL(17, 179, 14), // "finishedSignal"
QT_MOC_LITERAL(18, 194, 9), // "handleLog"
QT_MOC_LITERAL(19, 204, 11), // "handleError"
QT_MOC_LITERAL(20, 216, 20) // "handleTweetProcessed"

    },
    "BotWorker\0logSignal\0\0message\0"
    "tweetProcessedSignal\0timeStr\0username\0"
    "tweetUrl\0replyText\0status\0likeSuccess\0"
    "followSuccess\0retweetSuccess\0replyCount\0"
    "viewCount\0tweetId\0originalContent\0"
    "finishedSignal\0handleLog\0handleError\0"
    "handleTweetProcessed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BotWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,   12,   47,    2, 0x06 /* Public */,
      17,    0,   72,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      18,    1,   73,    2, 0x08 /* Private */,
      19,    1,   76,    2, 0x08 /* Private */,
      20,   12,   79,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::QString, QMetaType::QString,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::QString, QMetaType::QString,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,

       0        // eod
};

void BotWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BotWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->logSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->tweetProcessedSignal((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< bool(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])),(*reinterpret_cast< int(*)>(_a[10])),(*reinterpret_cast< const QString(*)>(_a[11])),(*reinterpret_cast< const QString(*)>(_a[12]))); break;
        case 2: _t->finishedSignal(); break;
        case 3: _t->handleLog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->handleError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->handleTweetProcessed((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< bool(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])),(*reinterpret_cast< int(*)>(_a[10])),(*reinterpret_cast< const QString(*)>(_a[11])),(*reinterpret_cast< const QString(*)>(_a[12]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BotWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BotWorker::logSignal)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BotWorker::*)(const QString & , const QString & , const QString & , const QString & , const QString & , bool , bool , bool , int , int , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BotWorker::tweetProcessedSignal)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BotWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&BotWorker::finishedSignal)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject BotWorker::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_BotWorker.data,
    qt_meta_data_BotWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BotWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BotWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BotWorker.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int BotWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void BotWorker::logSignal(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BotWorker::tweetProcessedSignal(const QString & _t1, const QString & _t2, const QString & _t3, const QString & _t4, const QString & _t5, bool _t6, bool _t7, bool _t8, int _t9, int _t10, const QString & _t11, const QString & _t12)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)), const_cast<void*>(reinterpret_cast<const void*>(&_t8)), const_cast<void*>(reinterpret_cast<const void*>(&_t9)), const_cast<void*>(reinterpret_cast<const void*>(&_t10)), const_cast<void*>(reinterpret_cast<const void*>(&_t11)), const_cast<void*>(reinterpret_cast<const void*>(&_t12)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BotWorker::finishedSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
