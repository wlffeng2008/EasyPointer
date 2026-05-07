#include "DialogTip.h"
#include "ui_DialogTip.h"

#include <QApplication>
#include <QStyleOption>
#include <QPainterPath>
#include <QTimer>
#include <QPainter>

DialogTip::DialogTip(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogTip)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("QDialog { background-color: rgba(180, 180, 180, 0.8);  border: none; border-radius: 24px;}");

    setFixedSize(120,120);
}

DialogTip::~DialogTip()
{
    delete ui;
}

 void DialogTip::showMode(int mode)
{
    this->mode = mode;
    show();
    QTimer::singleShot(3000,this,[=]{hide();});
 }

void DialogTip::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(rect(),24,24);
    painter.setClipPath(path);

    QStringList images={"数码光.jpg","放大镜.jpg","聚光灯.jpg","标注.jpg"};

    QString strImage=QApplication::applicationDirPath() + QString("/images/") + images[mode];
    painter.drawImage(rect(),QImage(strImage));

    QDialog::paintEvent(event);
}