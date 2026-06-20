#ifndef DIALOGNOCONNECT_H
#define DIALOGNOCONNECT_H

#include <QDialog>

namespace Ui {
class DialogNoConnect;
}

class DialogNoConnect : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNoConnect(QWidget *parent = nullptr);
    ~DialogNoConnect();
protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event);

private:
    Ui::DialogNoConnect *ui;
};

#endif // DIALOGNOCONNECT_H
