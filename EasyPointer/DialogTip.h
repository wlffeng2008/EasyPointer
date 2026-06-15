#ifndef DIALOGTIP_H
#define DIALOGTIP_H

#include <QDialog>

namespace Ui {
class DialogTip;
}

class DialogTip : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTip(QWidget *parent = nullptr);
    ~DialogTip();

    void showMode(int mode);

protected:
    void paintEvent(QPaintEvent *event) final;

private:
    Ui::DialogTip *ui;
    int mode=0;
    QTimer *m_pTMHide = nullptr;
};

#endif // DIALOGTIP_H
