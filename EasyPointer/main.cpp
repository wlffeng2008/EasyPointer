#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "EasyPointer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    a.setStyleSheet(R"(

    * { font-size: 14px; font-weight: 400;}

    QTableView{ border: 1px solid gray; background-color: rgb(226, 240, 255) ; gridline-color: gray;}
    QTableView::Item{padding-left:2px; background-color: rgb(226, 240, 255) ; border-top: 0px solid gray; border-bottom: 1px solid transparent;border-right: 0px solid gray;}
    QTableView::Item::selected{ background-color: #a0bb9e ; color:white; }
    QTableView QTableCornerButton::section { background-color: skyblue ; min-width: 32px; border-top: 0px solid gray; border-bottom: 1px solid gray; border-left: 0px solid gray; border-right: 1px solid gray; }

    QHeaderView::section{ background-color:skyblue;}
    QHeaderView::section:horizontal{ padding-left: 2px; border-top: 0px solid gray; border-bottom: 1px solid gray; border-right: 1px solid gray; font-weight: bold;}
    QHeaderView::section:vertical{ padding-left: 2px; text-align: right; min-width: 36px; border-top: 0px solid gray; border-bottom: 1px solid gray; border-left: 0px solid gray; border-right: 1px solid gray;}

    QTableView::indicator { width: 18px; height: 18px; }
    QTableView::indicator:checked { image: url(:/images/BoxChecked.png); }
    QTableView::indicator:unchecked { image: url(:/images/BoxUncheck.png); }

    QSlider { border-radius: 4px; }
    QSlider::groove:horizontal { height: 8px; background: transparent; border-radius: 4px; border: 1px solid #DCDCDC;}
    QSlider::sub-page:horizontal { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #FDDFBA, stop:1 #FFA73C);  border-radius: 4px; }
    QSlider::sub-page:horizontal:disabled { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 white, stop:1 #B3B3B3);  border-radius: 4px; }
    QSlider::handle:horizontal {
        width: 8px;
        height: 12px;
        margin: -4px 0px;
        border-radius: 8px;
        background: black;
        border: 4px solid white; }

    QSlider::handle:hover   { background: black; border-color: yellow; }
    QSlider::handle:pressed { background: #E0E0E0; border-color: #2D7FDD; }

    QSlider::handle:horizontal:disabled { border: 2px solid #B3B3B3; }
    QLabel#labelLarge{color:white; font-size: 24px; font-weight:600;}
    QPushButton{color:white;font-weight:600;}

    QCheckBox::indicator:unchecked {
            width: 18px;
            height: 18px;
            background: transparent;
            image: url(:/images/BoxUncheck.png); }
    QCheckBox::indicator:checked {
            width: 18px;
            height: 18px;
            background: transparent;
            image: url(:/images/BoxChecked.png); }

    QRadioButton::indicator {
        width: 22px;
        height: 22px;
        border: none;
        background: transparent;
        image: url(:/images/radio-unchecked.png); }

    QRadioButton::indicator:checked {
        width: 22px;
        height: 22px;
        background: transparent;
        border: none;
        image: url(:/images/radio-checked.png);}


    QComboBox{
    border:1px solid rgb(128, 128, 128);
    background-color: rgb(255, 255, 255);
    border-radius:4px;
    padding-left:10px;
    }

    QComboBox::drop-down{
    width:16px;
    height:16px;
    margin-right:2px;
    subcontrol-origin:padding;
    subcontrol-position: right;
    border-left-width: 0px;/* 下拉框的左边界线宽度 */
        border-left-color: darkgray;/* 下拉框的左边界线颜色 */
        border-left-style: solid; /* 下拉框的左边界线为实线 */
        border-top-right-radius: 3px;/* 下拉框的右上边界线的圆角半径（应和整个QComboBox右上边界线的圆角半径一致） */
        border-bottom-right-radius: 3px; /* 同上 */
        background: white;
        image:url(:/images/down-arrow.png);
    }
    QComboBox QAbstractItemView{
    border:1px solid rgb(0, 0, 0);
    background-color: rgb(255, 255, 255);
    }

    QComboBox QAbstractItemView::item:hover{
      background-color:#409CE1;
      color:#ffffff;
    }
    QComboBox QAbstractItemView::item:selected{
      background-color:#409CE1;
      color:#ffffff;
    }

)");

    QTranslator translatorA;
    QTranslator translatorB;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    translatorA.load("qt_zh_CN.qm", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    translatorB.load("qtbase_zh_CN.qm", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
#else
    translatorA.load("qt_zh_CN.qm", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translatorB.load("qtbase_zh_CN.qm", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif

    a.installTranslator(&translatorA);
    a.installTranslator(&translatorB);

    MainWindow w;
    w.show();
    return a.exec();
}
