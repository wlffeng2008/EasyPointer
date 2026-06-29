#include "DialogHealth.h"
#include "ui_DialogHealth.h"
#include "BleWorker.h"

DialogHealth::DialogHealth(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogHealth)
{
    ui->setupUi(this);

    connect(ui->pushButtonRead,&QPushButton::clicked,this,[=]{
        m_Reader->ReadRealData();
    });
    connect(ui->pushButtonStop,&QPushButton::clicked,this,[=]{
        m_Reader->ReadRealData(false);
    });
    connect(ui->pushButtonHWInfo,&QPushButton::clicked,this,[=]{
        m_Reader->ReadHWInfo();
    });
}

DialogHealth::~DialogHealth()
{
    delete ui;
}
