/****************************************************************************
** Meta object code from reading C++ file 'twitter_bot_gui.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "twitter_bot_gui.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'twitter_bot_gui.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TwitterBotGUI_t {
    QByteArrayData data[54];
    char stringdata0[697];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TwitterBotGUI_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TwitterBotGUI_t qt_meta_stringdata_TwitterBotGUI = {
    {
QT_MOC_LITERAL(0, 0, 13), // "TwitterBotGUI"
QT_MOC_LITERAL(1, 14, 19), // "updateAccountWidget"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 9), // "profileId"
QT_MOC_LITERAL(4, 45, 11), // "newSettings"
QT_MOC_LITERAL(5, 57, 16), // "showAccountsPage"
QT_MOC_LITERAL(6, 74, 16), // "showSettingsPage"
QT_MOC_LITERAL(7, 91, 13), // "showStatsPage"
QT_MOC_LITERAL(8, 105, 11), // "showLogPage"
QT_MOC_LITERAL(9, 117, 18), // "showAccountManager"
QT_MOC_LITERAL(10, 136, 13), // "addNewAccount"
QT_MOC_LITERAL(11, 150, 11), // "editAccount"
QT_MOC_LITERAL(12, 162, 16), // "onAccountChanged"
QT_MOC_LITERAL(13, 179, 19), // "showAccountSettings"
QT_MOC_LITERAL(14, 199, 8), // "startBot"
QT_MOC_LITERAL(15, 208, 7), // "stopBot"
QT_MOC_LITERAL(16, 216, 8), // "pauseBot"
QT_MOC_LITERAL(17, 225, 9), // "toggleBot"
QT_MOC_LITERAL(18, 235, 7), // "checked"
QT_MOC_LITERAL(19, 243, 9), // "configBot"
QT_MOC_LITERAL(20, 253, 12), // "startAllBots"
QT_MOC_LITERAL(21, 266, 11), // "stopAllBots"
QT_MOC_LITERAL(22, 278, 20), // "onAccountModeChanged"
QT_MOC_LITERAL(23, 299, 9), // "modeIndex"
QT_MOC_LITERAL(24, 309, 19), // "onGlobalModeChanged"
QT_MOC_LITERAL(25, 329, 13), // "onModeChanged"
QT_MOC_LITERAL(26, 343, 5), // "index"
QT_MOC_LITERAL(27, 349, 16), // "onTweetProcessed"
QT_MOC_LITERAL(28, 366, 7), // "timeStr"
QT_MOC_LITERAL(29, 374, 8), // "username"
QT_MOC_LITERAL(30, 383, 9), // "tweetLink"
QT_MOC_LITERAL(31, 393, 9), // "replyText"
QT_MOC_LITERAL(32, 403, 6), // "status"
QT_MOC_LITERAL(33, 410, 11), // "likeSuccess"
QT_MOC_LITERAL(34, 422, 13), // "followSuccess"
QT_MOC_LITERAL(35, 436, 14), // "retweetSuccess"
QT_MOC_LITERAL(36, 451, 12), // "responseTime"
QT_MOC_LITERAL(37, 464, 9), // "charCount"
QT_MOC_LITERAL(38, 474, 19), // "additionalStatsJson"
QT_MOC_LITERAL(39, 494, 10), // "currentUrl"
QT_MOC_LITERAL(40, 505, 8), // "onBotLog"
QT_MOC_LITERAL(41, 514, 7), // "message"
QT_MOC_LITERAL(42, 522, 10), // "onBotError"
QT_MOC_LITERAL(43, 533, 13), // "onBotFinished"
QT_MOC_LITERAL(44, 547, 17), // "refreshTweetStats"
QT_MOC_LITERAL(45, 565, 19), // "loadRecentTweetData"
QT_MOC_LITERAL(46, 585, 11), // "loadLogData"
QT_MOC_LITERAL(47, 597, 15), // "clearLogFilters"
QT_MOC_LITERAL(48, 613, 15), // "applyLogFilters"
QT_MOC_LITERAL(49, 629, 9), // "clearLogs"
QT_MOC_LITERAL(50, 639, 10), // "exportLogs"
QT_MOC_LITERAL(51, 650, 19), // "startLogAutoRefresh"
QT_MOC_LITERAL(52, 670, 20), // "onAutoRefreshChanged"
QT_MOC_LITERAL(53, 691, 5) // "value"

    },
    "TwitterBotGUI\0updateAccountWidget\0\0"
    "profileId\0newSettings\0showAccountsPage\0"
    "showSettingsPage\0showStatsPage\0"
    "showLogPage\0showAccountManager\0"
    "addNewAccount\0editAccount\0onAccountChanged\0"
    "showAccountSettings\0startBot\0stopBot\0"
    "pauseBot\0toggleBot\0checked\0configBot\0"
    "startAllBots\0stopAllBots\0onAccountModeChanged\0"
    "modeIndex\0onGlobalModeChanged\0"
    "onModeChanged\0index\0onTweetProcessed\0"
    "timeStr\0username\0tweetLink\0replyText\0"
    "status\0likeSuccess\0followSuccess\0"
    "retweetSuccess\0responseTime\0charCount\0"
    "additionalStatsJson\0currentUrl\0onBotLog\0"
    "message\0onBotError\0onBotFinished\0"
    "refreshTweetStats\0loadRecentTweetData\0"
    "loadLogData\0clearLogFilters\0applyLogFilters\0"
    "clearLogs\0exportLogs\0startLogAutoRefresh\0"
    "onAutoRefreshChanged\0value"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TwitterBotGUI[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,  179,    2, 0x0a /* Public */,
       5,    0,  184,    2, 0x08 /* Private */,
       6,    0,  185,    2, 0x08 /* Private */,
       7,    0,  186,    2, 0x08 /* Private */,
       8,    0,  187,    2, 0x08 /* Private */,
       9,    0,  188,    2, 0x08 /* Private */,
      10,    0,  189,    2, 0x08 /* Private */,
      11,    1,  190,    2, 0x08 /* Private */,
      12,    1,  193,    2, 0x08 /* Private */,
      13,    1,  196,    2, 0x08 /* Private */,
      14,    1,  199,    2, 0x08 /* Private */,
      15,    1,  202,    2, 0x08 /* Private */,
      16,    1,  205,    2, 0x08 /* Private */,
      17,    2,  208,    2, 0x08 /* Private */,
      19,    1,  213,    2, 0x08 /* Private */,
      20,    0,  216,    2, 0x08 /* Private */,
      21,    0,  217,    2, 0x08 /* Private */,
      22,    2,  218,    2, 0x08 /* Private */,
      24,    1,  223,    2, 0x08 /* Private */,
      25,    1,  226,    2, 0x08 /* Private */,
      27,   12,  229,    2, 0x08 /* Private */,
      40,    1,  254,    2, 0x08 /* Private */,
      42,    1,  257,    2, 0x08 /* Private */,
      43,    1,  260,    2, 0x08 /* Private */,
      44,    0,  263,    2, 0x08 /* Private */,
      45,    0,  264,    2, 0x08 /* Private */,
      46,    0,  265,    2, 0x08 /* Private */,
      47,    0,  266,    2, 0x08 /* Private */,
      48,    0,  267,    2, 0x08 /* Private */,
      49,    0,  268,    2, 0x08 /* Private */,
      50,    0,  269,    2, 0x08 /* Private */,
      51,    0,  270,    2, 0x08 /* Private */,
      52,    1,  271,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QVariantMap,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,   18,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    3,   23,
    QMetaType::Void, QMetaType::Int,   23,
    QMetaType::Void, QMetaType::Int,   26,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::QString, QMetaType::Bool, QMetaType::Bool, QMetaType::Bool, QMetaType::Int, QMetaType::Int, QMetaType::QString, QMetaType::QString,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   53,

       0        // eod
};

void TwitterBotGUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TwitterBotGUI *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateAccountWidget((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QVariantMap(*)>(_a[2]))); break;
        case 1: _t->showAccountsPage(); break;
        case 2: _t->showSettingsPage(); break;
        case 3: _t->showStatsPage(); break;
        case 4: _t->showLogPage(); break;
        case 5: _t->showAccountManager(); break;
        case 6: _t->addNewAccount(); break;
        case 7: _t->editAccount((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->onAccountChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->showAccountSettings((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->startBot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->stopBot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->pauseBot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->toggleBot((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 14: _t->configBot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->startAllBots(); break;
        case 16: _t->stopAllBots(); break;
        case 17: _t->onAccountModeChanged((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 18: _t->onGlobalModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->onModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->onTweetProcessed((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< const QString(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])),(*reinterpret_cast< bool(*)>(_a[7])),(*reinterpret_cast< bool(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])),(*reinterpret_cast< int(*)>(_a[10])),(*reinterpret_cast< const QString(*)>(_a[11])),(*reinterpret_cast< const QString(*)>(_a[12]))); break;
        case 21: _t->onBotLog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 22: _t->onBotError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 23: _t->onBotFinished((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: _t->refreshTweetStats(); break;
        case 25: _t->loadRecentTweetData(); break;
        case 26: _t->loadLogData(); break;
        case 27: _t->clearLogFilters(); break;
        case 28: _t->applyLogFilters(); break;
        case 29: _t->clearLogs(); break;
        case 30: _t->exportLogs(); break;
        case 31: _t->startLogAutoRefresh(); break;
        case 32: _t->onAutoRefreshChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TwitterBotGUI::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_TwitterBotGUI.data,
    qt_meta_data_TwitterBotGUI,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TwitterBotGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TwitterBotGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TwitterBotGUI.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int TwitterBotGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 33;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
