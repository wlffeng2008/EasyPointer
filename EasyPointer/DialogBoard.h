#ifndef DIALOGBOARD_H
#define DIALOGBOARD_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class DialogBoard;
}


typedef QList<QPoint> linePoints;


class DialogBoard : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBoard(QWidget *parent = nullptr);
    ~DialogBoard();
    void capScreen();
    void setMode(int mode);
    void clearLines();
    void setDrage(bool set=true);

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
    quint32 m_tmCount = 1800;
    quint32 m_onlyBlack = false;

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::DialogBoard *ui;

    QTimer *m_tmBlack = nullptr;
    QPoint m_curPos;
    int m_mode=0;
    bool m_bDraging=false;
    QPixmap m_screen;
    linePoints m_record;
    QList<linePoints> m_lines;
};

#endif // DIALOGBOARD_H
