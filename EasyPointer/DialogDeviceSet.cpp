#include "DialogDeviceSet.h"
#include "ui_DialogDeviceSet.h"
#include <QProcess>

DialogDeviceSet::DialogDeviceSet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogDeviceSet)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Dialog|Qt::MSWindowsFixedSizeDialogHint);
}

DialogDeviceSet::~DialogDeviceSet()
{
    delete ui;
}


void DialogDeviceSet::on_pushButtonSendKey_clicked()
{
    m_pWork->sendKey(0x2E,0x08);
}

void DialogDeviceSet::on_pushButtonMonitor_clicked()
{
    QProcess::execute("powershell",
                      QStringList() << "-Command"
                                    << "(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1,20)");
}

