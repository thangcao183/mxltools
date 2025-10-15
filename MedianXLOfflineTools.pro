TARGET = MedianXLOfflineTools
include(app.pri)

# QtSingleApplication
SOURCES += qtsingleapplication/qtsingleapplication.cpp \
           qtsingleapplication/qtlocalpeer.cpp

HEADERS += qtsingleapplication/QtSingleApplication \
           qtsingleapplication/qtsingleapplication.h \
           qtsingleapplication/qtlocalpeer.h

INCLUDEPATH += qtsingleapplication
QT += core gui widgets sql
!macx: DEFINES += HAS_QTSINGLEAPPLICATION=1
