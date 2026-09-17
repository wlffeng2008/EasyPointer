#ifndef DIALOGHEALTH_H
#define DIALOGHEALTH_H

#include <QDialog>
class BleWorker;

namespace Ui {
class DialogHealth;
}

class DialogHealth : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHealth(QWidget *parent = nullptr);
    ~DialogHealth();

    BleWorker *m_Reader = nullptr;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::DialogHealth *ui;
    QPoint m_dragPosition={0,0};
    bool m_dragging = false;
};

#endif // DIALOGHEALTH_H
