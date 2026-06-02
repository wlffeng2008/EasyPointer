#ifndef DIALOGCLOUDCMD_H
#define DIALOGCLOUDCMD_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class DialogCloudCmd;
}

class DialogCloudCmd : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCloudCmd(QWidget *parent = nullptr);
    ~DialogCloudCmd();

    bool startupApp(const QString&command);

private:
    Ui::DialogCloudCmd *ui;
    QStandardItemModel *m_pModel = nullptr;
    void saveLoadCommand(bool save=true);
};

#endif // DIALOGCLOUDCMD_H
