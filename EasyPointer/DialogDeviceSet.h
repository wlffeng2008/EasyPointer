#ifndef DIALOGDEVICESET_H
#define DIALOGDEVICESET_H

#include <QDialog>
#include <hidworker.h>

namespace Ui {
class DialogDeviceSet;
}

class DialogDeviceSet : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDeviceSet(QWidget *parent = nullptr);
    ~DialogDeviceSet();
    CHidWorker *m_pWork = nullptr;
private slots:
    void on_pushButtonSendKey_clicked();
private:
    Ui::DialogDeviceSet *ui;
};

#endif // DIALOGDEVICESET_H
