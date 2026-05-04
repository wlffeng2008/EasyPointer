#include "DialogMKeySet.h"
#include "ui_DialogMKeySet.h"

DialogMKeySet::DialogMKeySet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMKeySet)
{
    ui->setupUi(this);
}

DialogMKeySet::~DialogMKeySet()
{
    delete ui;
}
