#include "DialogDeviceSet.h"
#include "ui_DialogDeviceSet.h"

DialogDeviceSet::DialogDeviceSet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogDeviceSet)
{
    ui->setupUi(this);
}

DialogDeviceSet::~DialogDeviceSet()
{
    delete ui;
}
