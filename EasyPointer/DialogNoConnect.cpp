#include "DialogNoConnect.h"
#include "ui_DialogNoConnect.h"
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOption>

DialogNoConnect::DialogNoConnect(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNoConnect)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);

    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet("QDialog { background-color: rgba(180, 180, 180, 0.8);}");

}

DialogNoConnect::~DialogNoConnect()
{
    delete ui;
}

bool DialogNoConnect::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *pKE = static_cast<QKeyEvent *>(event);
        if(pKE->key() == Qt::Key_Escape)
            return true;
    }
    return QDialog::event(event);
}

void DialogNoConnect::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    QDialog::paintEvent(event);
}