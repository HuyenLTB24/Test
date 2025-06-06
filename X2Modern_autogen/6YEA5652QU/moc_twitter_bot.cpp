/****************************************************************************
** Meta object code from reading C++ file 'twitter_bot.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "twitter_bot.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'twitter_bot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TwitterBot_t {
    QByteArrayData data[27];
    char stringdata0[316];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TwitterBot_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TwitterBot_t qt_meta_stringdata_TwitterBot = {
    {
QT_MOC_LITERAL(0, 0, 10), // "TwitterBot"
QT_MOC_LITERAL(1, 11, 20), // "tweetProcessedSignal"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 7), // "timeStr"
QT_MOC_LITERAL(4, 41, 8), // "username"
QT_MOC_LITERAL(5, 50, 8), // "tweetUrl"
QT_MOC_LITERAL(6, 59, 9), // "replyText"
QT_MOC_LITERAL(7, 69, 6), // "status"
QT_MOC_LITERAL(8, 76, 11), // "likeSuccess"
QT_MOC_LITERAL(9, 88, 13), // "followSuccess"
QT_MOC_LITERAL(10, 102, 14), // "retweetSuccess"
QT_MOC_LITERAL(11, 117, 10), // "replyCount"
QT_MOC_LITERAL(12, 128, 9), // "viewCount"
QT_MOC_LITERAL(13, 138, 7), // "tweetId"
QT_MOC_LITERAL(14, 146, 15), // "originalContent"
QT_MOC_LITERAL(15, 162, 9), // "logSignal"
QT_MOC_LITERAL(16, 172, 7), // "message"
QT_MOC_LITERAL(17, 180, 12), // "statusSignal"
QT_MOC_LITERAL(18, 193, 11), // "errorSignal"
QT_MOC_LITERAL(19, 205, 5), // "error"
QT_MOC_LITERAL(20, 211, 10), // "stopSignal"
QT_MOC_LITERAL(21, 222, 16), // "onTweetProcessed"
QT_MOC_LITERAL(22, 239, 16), // "processNextTweet"
QT_MOC_LITERAL(23, 256, 18), // "handleNetworkReply"
QT_MOC_LITERAL(24, 275, 22), // "handlePageLoadFinished"
QT_MOC_LITERAL(25, 298, 2), // "ok"
QT_MOC_LITERAL(26, 301, 14) // "onTimerTimeout"

    },
    "TwitterBot\0tweetProcessedSignal\0\0"
    "timeStr\0username\0tweetUrl\0replyText\0"
    "status\0likeSuccess\0followSuccess\0"
    "retweetSuccess\0replyCount\0viewCount\0"
    "tweetId\0originalContent\0logSignal\0"
    "message\0statusSignal\0errorSignal\0error\0"
    "stopSignal\0onTweetProcessed\0"
    "processNextTweet\0handleNetworkReply\0"
    "handlePageLoadFinished\0ok\0onTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TwitterBot[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,   12,   64,    2, 0x06 /* Public */,
      15,    1,   89,    2, 0x06 /* Public */,
      17,    1,   92,    2, 0x06 /* Public */,
      18,    1,   95,    2, 0x06 /* Public */,
      20,    0,   98,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      21,   12,   99,    2, 0x0a /* Public */,
      22,    0,  124,    2, 0x08 /* Private */,
      23,    0,  125,    2, 0x08 /* Private */,
      24,    1,  126,    2, 0x08 /* Private */,
      26,    0,  129,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::QString, QMetaType::QString,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::QString, QMetaType::QString,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   25,
    QMetaType::Void,

       0        // eod
};

void TwitterBot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TwitterBot *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->tweetProcessedSignal((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< bool(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])),(*reinterpret_cast< int(*)>(_a[10])),(*reinterpret_cast< const QString(*)>(_a[11])),(*reinterpret_cast< const QString(*)>(_a[12]))); break;
        case 1: _t->logSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->statusSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->errorSignal((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->stopSignal(); break;
        case 5: _t->onTweetProcessed((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< bool(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])),(*reinterpret_cast< int(*)>(_a[10])),(*reinterpret_cast< const QString(*)>(_a[11])),(*reinterpret_cast< const QString(*)>(_a[12]))); break;
        case 6: _t->processNextTweet(); break;
        case 7: _t->handleNetworkReply(); break;
        case 8: _t->handlePageLoadFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->onTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TwitterBot::*)(const QString & , const QString & , const QString & , const QString & , const QString & , bool , bool , bool , int , int , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TwitterBot::tweetProcessedSignal)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TwitterBot::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TwitterBot::logSignal)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TwitterBot::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TwitterBot::statusSignal)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TwitterBot::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TwitterBot::errorSignal)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TwitterBot::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TwitterBot::stopSignal)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TwitterBot::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_TwitterBot.data,
    qt_meta_data_TwitterBot,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TwitterBot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TwitterBot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TwitterBot.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TwitterBot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void TwitterBot::tweetProcessedSignal(const QString & _t1, const QString & _t2, const QString & _t3, const QString & _t4, const QString & _t5, bool _t6, bool _t7, bool _t8, int _t9, int _t10, const QString & _t11, const QString & _t12)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)), const_cast<void*>(reinterpret_cast<const void*>(&_t8)), const_cast<void*>(reinterpret_cast<const void*>(&_t9)), const_cast<void*>(reinterpret_cast<const void*>(&_t10)), const_cast<void*>(reinterpret_cast<const void*>(&_t11)), const_cast<void*>(reinterpret_cast<const void*>(&_t12)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TwitterBot::logSignal(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TwitterBot::statusSignal(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TwitterBot::errorSignal(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TwitterBot::stopSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
