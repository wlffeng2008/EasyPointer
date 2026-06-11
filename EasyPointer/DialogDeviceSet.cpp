#include "DialogDeviceSet.h"
#include "ui_DialogDeviceSet.h"

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