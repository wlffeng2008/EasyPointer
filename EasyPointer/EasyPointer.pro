QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


DESTDIR = ../../bin
RC_ICONS = nmy.ico
VERSION = 1.0.0.0
TARGET = NmyStudio
QMAKE_TARGET_COMPANY = "Nmy"
QMAKE_TARGET_PRODUCT = "Nmy"
QMAKE_TARGET_DESCRIPTION = "Nmy: Created by Qt6.10.0"
QMAKE_TARGET_COPYRIGHT = "Nmy(2026.01)"
RC_LANG = 0x0004

# 仅Windows平台生效
win32 {
    LIBS += -luser32 -lkernel32 -lpsapi # 链接窗口/进程相关库
}

LIBS += -L$$PWD -lhidapi


SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    EasyPointer_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
