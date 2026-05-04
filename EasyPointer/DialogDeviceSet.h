#ifndef DIALOGDEVICESET_H
#define DIALOGDEVICESET_H

#include <QDialog>

namespace Ui {
class DialogDeviceSet;
}

class DialogDeviceSet : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDeviceSet(QWidget *parent = nullptr);
    ~DialogDeviceSet();

private:
    Ui::DialogDeviceSet *ui;
};

#endif // DIALOGDEVICESET_H
