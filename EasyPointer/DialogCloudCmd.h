#ifndef DIALOGCLOUDCMD_H
#define DIALOGCLOUDCMD_H

#include <QDialog>

namespace Ui {
class DialogCloudCmd;
}

class DialogCloudCmd : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCloudCmd(QWidget *parent = nullptr);
    ~DialogCloudCmd();

private:
    Ui::DialogCloudCmd *ui;
};

#endif // DIALOGCLOUDCMD_H
