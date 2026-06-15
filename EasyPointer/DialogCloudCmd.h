#ifndef DIALOGCLOUDCMD_H
#define DIALOGCLOUDCMD_H

#include "hidworker.h"
#include "DialogBoard.h"

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

    CHidWorker *m_pWork = nullptr;
    DialogBoard *m_pPad = nullptr;

private:
    Ui::DialogCloudCmd *ui;
    QStandardItemModel *m_pModel = nullptr;
    QStandardItemModel *m_pModel1 = nullptr;
    void saveLoadCommand(bool save=true);
    void saveLoadDefault(bool save=true);
};

#endif // DIALOGCLOUDCMD_H
