QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include($$PWD/QXlsx/QXlsx.pri)

# 你可以取消注释这行，禁用过时的 Qt API。
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000



win32 {
    RC_ICONS = $$PWD/resource/logo.ico
}


SOURCES += \
    DatabaseManager.cpp \
    ExcelManager.cpp \
    ScoreViewer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    DatabaseManager.h \
    ExcelManager.h \
    ScoreViewer.h \
    mainwindow.h

FORMS += \
    ScoreViewer.ui \
    mainwindow.ui


# 默认的部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    logo.qrc
