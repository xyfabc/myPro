/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Fri Mar 30 11:33:15 2018
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      42,   11,   11,   11, 0x08,
      68,   11,   11,   11, 0x08,
      97,   92,   11,   11, 0x08,
     141,   11,   11,   11, 0x08,
     167,   11,   11,   11, 0x08,
     193,   11,   11,   11, 0x08,
     221,   11,   11,   11, 0x08,
     247,   11,   11,   11, 0x08,
     271,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0on_build_pushButton_clicked()\0"
    "on_pushButton_2_clicked()\0"
    "on_pushButton_clicked()\0item\0"
    "on_listWidget_itemPressed(QListWidgetItem*)\0"
    "on_pushButton_3_clicked()\0"
    "on_UPtoolButton_clicked()\0"
    "on_DowntoolButton_clicked()\0"
    "on_pushButton_4_clicked()\0"
    "on_toolButton_clicked()\0handleTimeout()\0"
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_build_pushButton_clicked(); break;
        case 1: on_pushButton_2_clicked(); break;
        case 2: on_pushButton_clicked(); break;
        case 3: on_listWidget_itemPressed((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 4: on_pushButton_3_clicked(); break;
        case 5: on_UPtoolButton_clicked(); break;
        case 6: on_DowntoolButton_clicked(); break;
        case 7: on_pushButton_4_clicked(); break;
        case 8: on_toolButton_clicked(); break;
        case 9: handleTimeout(); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE