QT       += core gui multimedia websockets network bluetooth

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include(./QXlsx/QXlsx.pri)

DESTDIR = ../../bin
RC_ICONS = nmy.ico
VERSION = 1.0.0.0
TARGET = NmyStudio
QMAKE_TARGET_COMPANY = "Nmy"
QMAKE_TARGET_PRODUCT = "Nmy"
QMAKE_TARGET_DESCRIPTION = "Nmy Pointer"
QMAKE_TARGET_COPYRIGHT = "Nmy(2026.01)"
RC_LANG = 0x0004

# 仅Windows平台生效
win32 {
    LIBS += -luser32 -lkernel32 -lpsapi  -ldxva2# 链接窗口/进程相关库
}

LIBS += -L$$PWD -lhidapi
# LIBS += -luser32 -ladvapi32 -lole32 -lpropsys -luuid

SOURCES += \
    BleWorker.cpp \
    DialogBoard.cpp \
    DialogCloudCmd.cpp \
    DialogDeviceSet.cpp \
    DialogMKeySet.cpp \
    DialogNoConnect.cpp \
    DialogRecord.cpp \
    DialogTip.cpp \
    DialogTypeWord.cpp \
    KeyBoardMonitor.cpp \
    ToggleButton.cpp \
    TxAsrClient.cpp \
    caudioplayer.cpp \
    frameapptemplate.cpp \
    hidworker.cpp \
    main.cpp \
    mainwindow.cpp \
    typeword.cpp \
    xfdatadecoder.cpp \
    xftexttospeech.cpp \
    xftexttranslate.cpp \
    xfwsvoicewrite.cpp

HEADERS += \
    BleWorker.h \
    DialogBoard.h \
    DialogCloudCmd.h \
    DialogDeviceSet.h \
    DialogMKeySet.h \
    DialogNoConnect.h \
    DialogRecord.h \
    DialogTip.h \
    DialogTypeWord.h \
    KeyBoardMonitor.h \
    ToggleButton.h \
    TxAsrClient.h \
    caudioplayer.h \
    frameapptemplate.h \
    hidworker.h \
    mainwindow.h \
    typeword.h \
    xfdatadecoder.h \
    xftexttospeech.h \
    xftexttranslate.h \
    xfwsvoicewrite.h

FORMS += \
    DialogBoard.ui \
    DialogCloudCmd.ui \
    DialogDeviceSet.ui \
    DialogMKeySet.ui \
    DialogNoConnect.ui \
    DialogRecord.ui \
    DialogTip.ui \
    DialogTypeWord.ui \
    frameapptemplate.ui \
    mainwindow.ui \
    typeword.ui

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
