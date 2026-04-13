#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>


static void QLog(const char *buf,int nlen=16)
{
    QByteArray data(buf + 1, nlen);
    qDebug() << "tmp :" << data.left(nlen).toHex(' ').toUpper();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("Nmy Pointer");
    resize(828,466);

    setStyleSheet(R"(

    * { font-size: 14px; font-weight: 400;}

    QSlider { border-radius: 4px; }
    QSlider::groove:horizontal { height: 8px; background: transparent; border-radius: 4px; border: 1px solid #DCDCDC;}
    QSlider::sub-page:horizontal { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #FDDFBA, stop:1 #FFA73C);  border-radius: 4px; }
    QSlider::sub-page:horizontal:disabled { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 white, stop:1 #B3B3B3);  border-radius: 4px; }
    QSlider::handle:horizontal {
        width: 8px;
        height: 12px;
        margin: -4px 0px;
        border-radius: 8px;
        background: black;
        border: 4px solid white; }

    QSlider::handle:hover   { background: black; border-color: yellow; }
    QSlider::handle:pressed { background: #E0E0E0; border-color: #2D7FDD; }

    QSlider::handle:horizontal:disabled { border: 2px solid #B3B3B3; }

    QLabel{color:white;font-weight:600;}
    QLabel#labelLarge{color:white; font-size: 24px; font-weight:600;}
    QPushButton{color:white;font-weight:600;}

    QCheckBox::indicator:unchecked {
            width: 18px;
            height: 18px;
            background: transparent;
            image: url(:/images/BoxUncheck.png); }
    QCheckBox::indicator:checked {
            width: 18px;
            height: 18px;
            background: transparent;
            image: url(:/images/BoxChecked.png); }

    QRadioButton::indicator {
        width: 22px;
        height: 22px;
        border: none;
        background: transparent;
        image: url(:/images/radio-unchecked.png); }

    QRadioButton::indicator:checked {
        width: 22px;
        height: 22px;
        background: transparent;
        border: none;
        image: url(:/images/radio-checked.png);}
)");

    connect(ui->pushButtonExit,&QPushButton::clicked,this,[=]{ qApp->exit(); });

    connect(ui->buttonGroupMain,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        ui->stackedWidget0->setCurrentIndex(index);
        ui->stackedWidget1->setCurrentIndex(index);

        ui->labelContact->setHidden(true);
        ui->frameColor->setHidden(true);
        ui->stackedWidget1->setHidden(true);
        ui->labelNotice->setHidden(true);

        switch(index)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if(index == 3 || index == 0)
            ui->frameColor->setHidden(false);
            ui->stackedWidget1->setHidden(false);
            break;
        case 4:
            ui->labelNotice->setHidden(false);
            break;
        case 5:
            ui->labelNotice->setHidden(false);
            break;

        case 6:
            ui->labelContact->setHidden(false);
            break;

        case 7:
            break;
        }


    });

    connect(ui->buttonGroupColor,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        QStringList colors={"#FF0000","#00FF00","#0000FF","#FFFFFF","#FF8000","#800080","#FFFF00","#00FFFF","#000000"};
        ui->labelColor->setStyleSheet(QString("QLabel{background-color:%1; border-radius:40px;}").arg(colors[index]));
    });

    ui->pushButtonMain->click();

    QImage batt(":/images/batt.png");

    QPainter p(&batt);
    p.fillRect(QRect(3,3,23,10),QBrush(Qt::green));
    ui->labelBattery->setPixmap(QPixmap::fromImage(batt));

    m_pHID = new CHidWorker();
    //m_pHID->start();

    return;
    {
        QList<quint16> VidList = {0x3151, 0x38EE, 0x25A7, 0x05AC, 0x0461};
        foreach (quint16 VID, VidList)
        {
            hid_device_info *pRoot = hid_enumerate(VID, 0);
            hid_device_info *pTemp = pRoot;

            QString path1,path2;
            while (pTemp)
            {
                if(pTemp->usage_page == 0xFFFF)
                {
                    if(pTemp->usage == 1) path1 = pTemp->path;
                    if(pTemp->usage == 2) path2 = pTemp->path;
                }

                if(!path1.isEmpty() && !path2.isEmpty())
                {
                    hid_device *pDev = hid_open_path(path2.toStdString().c_str());
                    if (!pDev) continue;

                    quint16 PID = pTemp->product_id;

                    QByteArray cmd(120, 0);
                    cmd[1] = 0x8F;
                    cmd[8] = 0xFF - cmd[1];

                    hid_send_feature_report(pDev, (quint8 *)cmd.data(), 65);
                    QThread::msleep(20);

                    char buf[128] = {0};
                    int nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                    if (nlen > 0)
                    {
                        QByteArray data(buf + 1, nlen - 1);
                        quint32 id = *(quint32 *)(data.data() + 1);

                        if(id>0x1000)
                        {
                            {
                                QByteArray tmp(120, 0);
                                tmp[1] = 0xf6;
                                tmp[2] = 0x0A;
                                tmp[8] = 0xFF - tmp[1] - tmp[2];
                                hid_send_feature_report(pDev, (quint8 *)tmp.data(), 65);
                                QThread::msleep(10);
                                nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                                QLog(buf);

                                while(1)
                                {
                                    tmp[1] = 0xf7;
                                    tmp[2] = 0x00;
                                    tmp[8] = 0xFF - tmp[1] - tmp[2];
                                    hid_send_feature_report(pDev, (quint8 *)tmp.data(), 65);
                                    QThread::msleep(15);
                                    nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                                    if(buf[6] == 1)
                                        break;
                                }
                                QLog(buf);

                                tmp[1] = 0x8F;
                                tmp[2] = 0x00;
                                tmp[8] = 0xFF - tmp[1] - tmp[2];
                                hid_send_feature_report(pDev, (quint8 *)tmp.data(), 65);
                                QThread::msleep(20);
                                nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);

                                while(1)
                                {
                                    tmp[1] = 0xf7;
                                    tmp[2] = 0x00;
                                    tmp[8] = 0xFF - tmp[1] - tmp[2];
                                    hid_send_feature_report(pDev, (quint8 *)tmp.data(), 65);
                                    QThread::msleep(15);
                                    nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                                    if(buf[0] == 0)
                                        break;
                                }

                                tmp[1] = 0xfc;
                                tmp[2] = 0x00;
                                tmp[8] = 0xFF - tmp[1] - tmp[2];
                                hid_send_feature_report(pDev, (quint8 *)tmp.data(), 65);
                                QThread::msleep(15);
                                nlen = hid_get_feature_report(pDev, (quint8 *)buf, 65);
                            }

                            if (nlen > 0)
                            {
                                QByteArray data(buf + 1, nlen - 1);
                                id = *(quint32 *)(data.data() + 1);
                                qDebug() << "get_:" << data.left(16).toHex(' ').toUpper() << id << Qt::hex << PID;
                            }
                        }
                        else
                        {
                            qDebug() << "get_:" << data.left(16).toHex(' ').toUpper() << id << Qt::hex << PID;
                        }

                        path1.clear();
                        path2.clear();
                    }

                    hid_close(pDev);
                }
                pTemp = pTemp->next;
            }
            hid_free_enumeration(pRoot);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.setRenderHint(QPainter::Antialiasing, true);

    p.drawImage(this->rect(), QImage("./images/bground.png"));
    p.drawImage(QRect(0,2,132,44), QImage("./images/nmylogo.png"));

    QMainWindow::paintEvent(event);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->pos().y() < 200)
        {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
            m_dragging = true;
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_dragging)
    {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }
}

