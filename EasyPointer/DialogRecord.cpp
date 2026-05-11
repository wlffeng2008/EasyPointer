#include "DialogRecord.h"
#include "ui_DialogRecord.h"

#include <QScreen>

DialogRecord::DialogRecord(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRecord)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint|Qt::WindowStaysOnTopHint);
}

DialogRecord::~DialogRecord()
{
    delete ui;
}

void DialogRecord::showEvent(QShowEvent *event)
{
    int nPosX = QApplication::screens().at(0)->geometry().width() - size().width() - 20;
    int nPosY = QApplication::screens().at(0)->geometry().height() - size().height() - 30;
    QRect rcTip(nPosX,nPosY,size().width(),size().height());

    setGeometry(rcTip);
    QDialog::showEvent(event);
}
