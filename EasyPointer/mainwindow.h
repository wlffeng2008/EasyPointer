#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qpushbutton.h"
#include <QMainWindow>
#include <QTimer>
#include <QTranslator>
#include <QSystemTrayIcon>

#include "hidworker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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
    QAbstractButton *m_pActive=nullptr;
    CHidWorker *m_pHID = nullptr;
    int m_mode = 0;
    int m_press = 0;

    QSystemTrayIcon *trayIcon = nullptr;
    QAction *m_act0 = nullptr;
    QAction *m_act1 = nullptr;
    QAction *m_act2 = nullptr;
};
#endif // MAINWINDOW_H
