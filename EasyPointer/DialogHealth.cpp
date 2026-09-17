#include "DialogHealth.h"
#include "ui_DialogHealth.h"
#include "BleWorker.h"

#include <QMouseEvent>

DialogHealth::DialogHealth(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogHealth)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->pushButtonRead,&QPushButton::clicked,this,[=]{
        m_Reader->ReadRealData();
    });
    connect(ui->pushButtonStop,&QPushButton::clicked,this,[=]{
        m_Reader->ReadRealData(false);
    });
    connect(ui->pushButtonHWInfo,&QPushButton::clicked,this,[=]{
        m_Reader->ReadHWInfo();
    });
    connect(ui->pushButtonClose,&QPushButton::clicked,this,[=]{
        hide();
    });
}

DialogHealth::~DialogHealth()
{
    delete ui;
}

void DialogHealth::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->pos().y() < 45)
        {
            event->accept();
            m_dragging = true;
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            return;
        }
    }

    QDialog::mousePressEvent(event);
}

void DialogHealth::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_dragging)
    {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }

    QDialog::mouseMoveEvent(event);
}

void DialogHealth::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }

    QDialog::mouseReleaseEvent(event);
}