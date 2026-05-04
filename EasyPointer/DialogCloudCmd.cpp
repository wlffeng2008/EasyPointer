#include "DialogCloudCmd.h"
#include "ui_DialogCloudCmd.h"

DialogCloudCmd::DialogCloudCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCloudCmd)
{
    ui->setupUi(this);
}

DialogCloudCmd::~DialogCloudCmd()
{
    delete ui;
}
