#include "DialogManager.h"
#include "ui_DialogManager.h"

DialogManager::DialogManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogManager)
{
    ui->setupUi(this);

    setStyleSheet("color:black; font-size:10px;");
}

DialogManager::~DialogManager()
{
    delete ui;
}
