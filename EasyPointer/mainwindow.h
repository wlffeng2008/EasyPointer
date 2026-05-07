#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTranslator>
#include <QSystemTrayIcon>

#include "hidworker.h"
#include "DialogTip.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DialogBoard;

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

    int m_mode  = -1;
    int m_press = 0;
    CHidWorker *m_pHID = nullptr;
    DialogBoard *pFuncPad = nullptr;

    quint16 m_radius0;
    quint16 m_radius1;
    quint16 m_radius2;
    quint16 m_radius3;

    bool m_bRound = true;

    QColor m_color0;
    QColor m_color1;
    QColor m_color2;
    QColor m_color3;
    int m_iColor0=0;
    int m_iColor3=0;

    int m_index=0;
    int m_iColor=0;
    void updateValue();
    DialogTip *m_ModeTip = nullptr;

    QAction *m_act0 = nullptr;
    QAction *m_act1 = nullptr;
    QAction *m_act2 = nullptr;
    QSystemTrayIcon *trayIcon = nullptr;
};
#endif // MAINWINDOW_H
