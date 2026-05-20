#include "DialogTip.h"
#include "ui_DialogTip.h"

#include <QApplication>
#include <QStyleOption>
#include <QPainterPath>
#include <QTimer>
#include <QPainter>
#include <QScreen>

DialogTip::DialogTip(QWidget *parent)
    : QDialog(nullptr)
    , ui(new Ui::DialogTip)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::Tool | Qt::FramelessWindowHint |Qt::WindowStaysOnTopHint);
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
    QTimer::singleShot(2000,this,[=]{hide();});

    int nPosX = QApplication::screens().at(0)->geometry().width()/2 -60;
    int nPosY = QApplication::screens().at(0)->geometry().height() - 220;
    QRect rcTip(nPosX,nPosY,120,120);

    setGeometry(rcTip);
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

    static QStringList images = {"数码光.jpg","放大镜.jpg","聚光灯.jpg","标注.jpg","扩音模式.jpg","打字模式.jpg","翻译模式.jpg"};

    QString strImage = QApplication::applicationDirPath() + QString("/images/") + images[mode];
    painter.drawImage(rect(),QImage(strImage));

    QDialog::paintEvent(event);
}