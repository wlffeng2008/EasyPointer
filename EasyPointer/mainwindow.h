#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "hidworker.h"

#include <QMainWindow>
#include <QTimer>
#include <QTranslator>
#include <QSystemTrayIcon>

#include <QSettings>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DialogTip;
class DialogBoard;
class DialogRecord;
class DialogCloudCmd;
class DialogTypeWord;
class DialogDeviceSet;
class DialogMKeySet;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:
    Ui::MainWindow *ui;

    QPoint m_dragPosition;
    bool m_dragging = false;

    quint8 m_mode   = -1;
    quint8 m_modeV  = -1;
    quint8 m_press  =  0;
    CHidWorker *m_pHID = nullptr;
    DialogBoard *pFuncPad = nullptr;
    DialogRecord *m_RecPad = nullptr;

    DialogTypeWord *m_pTSet = nullptr;
    DialogCloudCmd *m_pCmd = nullptr;

    DialogDeviceSet *m_pDSet;
    DialogMKeySet *m_pMSet;

    quint16 m_radius0;
    quint16 m_radius1;
    quint16 m_radius2;
    quint16 m_radius3;

    bool m_bLoading = false;
    bool m_bRound = true;
    qreal m_enlarge= 1.5;
    quint8 m_battery = 10;
    qint16 m_EfOpticy = 0;

    QColor m_color0;
    QColor m_color1;
    QColor m_color2;
    QColor m_color3;
    int m_iColor0=0;
    int m_iColor3=0;

    bool m_record=false;
    int m_voice=0;
    int m_show=0;
    int m_index=0;
    int m_iColor=0;
    int m_iSpot=0;
    int m_iEffect=0;
    void updateValue();
    DialogTip *m_ModeTip = nullptr;

    QSettings *m_set=nullptr;
    void saveLoadParams(bool save=true);

    QAction *m_act0 = nullptr;
    QAction *m_act1 = nullptr;
    QAction *m_act2 = nullptr;
    QSystemTrayIcon *trayIcon = nullptr;
};
#endif // MAINWINDOW_H
