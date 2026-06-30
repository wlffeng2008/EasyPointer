#include "DialogManager.h"
#include "ui_DialogManager.h"

DialogManager::DialogManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogManager)
{
    ui->setupUi(this);
}

DialogManager::~DialogManager()
{
    delete ui;
}
