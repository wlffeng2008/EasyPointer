#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer>
#include <QMenu>
#include <QPainterPath>
#include <QApplication>

#include "DialogBoard.h"
#include "DialogTypeWord.h"
#include "DialogCloudCmd.h"
#include "DialogDeviceSet.h"
#include "DialogMKeySet.h"

static QList<quint32> colors={0xFF0000,0x00FF00,0x0000FF,0xFFFFFF,0xFF8000,0x800080,0xFFFF00,0x00FFFF,0x000000};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("Nmy Pointer");
    //resize(828,466);

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

    pFuncPad  = new DialogBoard();
    m_ModeTip = new DialogTip(this);

    QCoreApplication::setOrganizationName("NMY");
    QCoreApplication::setApplicationName("NMYPointer");
    m_set = new QSettings();

    connect(ui->pushButtonExit,&QPushButton::clicked,this,[=]{ qApp->exit(); });
    connect(ui->pushButtonMin,&QPushButton::clicked,this,[=]{ this->showMinimized(); });

    connect(ui->buttonGroupMain,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id);
        if(index >= 4)
            index -= 4;
        else
            index += 3;

        ui->stackedWidget0->setCurrentIndex(index);
        ui->stackedWidget1->setCurrentIndex(index);

        ui->labelContact->setHidden(true);
        ui->frameColor->setHidden(true);
        ui->stackedWidget1->setHidden(true);
        ui->labelNotice->setHidden(true);

        m_index = index;
        switch(index)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if(index == 3 || index == 0)
            ui->frameColor->setHidden(false);
            ui->stackedWidget1->setHidden(false);
            //m_ModeTip->showMode(index);
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
        }

        if(index == 0) ui->buttonGroupColor->button(-m_iColor0-2)->click();
        if(index == 3) ui->buttonGroupColor->button(-m_iColor3-2)->click();

        updateValue();
        saveLoadParams();
    });

    connect(ui->buttonGroupColor,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        m_iColor = index;
        updateValue();
        saveLoadParams();
    });
    connect(ui->buttonGroupCount,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        m_show = index;
        updateValue();
        saveLoadParams();
    });
    connect(ui->buttonGroupVoice,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        m_voice = index;
        updateValue();
        saveLoadParams();
    });

    connect(ui->horizontalSlider0,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue00->setText(QString("%1").arg(value));
    });    
    connect(ui->horizontalSlider1,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue01->setText(QString::asprintf("%1%%").arg(value));
    });

    connect(ui->horizontalSlider2,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue10->setText(QString::asprintf("%1").arg(value));
    });

    connect(ui->horizontalSlider4,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue20->setText(QString::asprintf("%1").arg(value));
    });
    connect(ui->horizontalSlider5,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue21->setText(QString::asprintf("%1%%").arg(value));
    });

    connect(ui->horizontalSlider6,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue30->setText(QString::asprintf("%1").arg(value));
    });
    connect(ui->horizontalSlider7,&QSlider::valueChanged,this,[=](int value){
        updateValue();
        ui->labelValue31->setText(QString::asprintf("%1%%").arg(value));
    });

    m_pHID = new CHidWorker();

    connect(m_pHID,&CHidWorker::onDataIn,this,[=](quint8 *data,int len){

        quint8 cmd = data[3];
        // 0c 4d 14 61 4d 4c 20 24 12 14 02 40 00 07 05 23 00 ce 5f 27 00 00 00 00 00 00 00 00 00 00 00 00
        // 10字节设备SN号
        // 1字节录音状态：1: 16k  0:8k
        // 1字节光电类型：附录给出定义
        // 1字节当前DPI档位
        // 1字节设备型号低8位
        // 1字节电量值：0-0x64
        // 1字节软件版本号
        // 1字节设备型号高8位
        static quint8 nFunc = 0;
        switch(cmd)
        {
        case 0x91: qDebug() << "单击" ;
            m_pHID->setMouse(false);
            pFuncPad->hide();
            m_press = 0;
            break;

        case 0x92:
        {
            m_pHID->setLaser(false);
            qDebug() << "双击";   // 仅切换功能
            m_mode = nFunc % 4;
            m_ModeTip->showMode(m_mode);
            if(m_mode == 0) ui->pushButton0->click();
            if(m_mode == 1) ui->pushButton1->click();
            if(m_mode == 2) ui->pushButton2->click();
            if(m_mode == 3) ui->pushButton3->click();
            nFunc++;
            m_press = 0;
            pFuncPad->hide();
        }
            break;

        case 0x93: qDebug() << "长按" ;
            if(m_mode == -1) break;
            m_pHID->setMouse(true);
            m_press ++;
            if(m_mode != 3 || m_press >= 1)
                pFuncPad->setDrage();

            QTimer::singleShot(100,this,[=]{
                hide();
                pFuncPad->capScreen();
                pFuncPad->setMode(m_mode);
                pFuncPad->showFullScreen();
                pFuncPad->raise();
                QApplication::setOverrideCursor(Qt::BlankCursor);
            });
            break;

        case 0x94: qDebug() << "松开" ;
            QApplication::restoreOverrideCursor();
            QApplication::setOverrideCursor(Qt::ArrowCursor);
            if(m_mode != 3)
            {
                show();
                pFuncPad->hide();
                //m_pHID->setMouse(false);
            }

            pFuncPad->setDrage(false);
            break;

        case 0x9b:
        {
            updateValue();
            if(pFuncPad->isHidden())
            {
                QApplication::setOverrideCursor(Qt::BlankCursor);
                pFuncPad->setMode(4);
                pFuncPad->showFullScreen();
                pFuncPad->raise();
            }
            else
            {
                pFuncPad->hide();
                QApplication::setOverrideCursor(Qt::ArrowCursor);
            }
        }
            break;
        case 0x9d: // s2 单击
            break;
        case 0x9e: // s2 双击
            break;

        case 0x9f: qDebug() << "声音外放....." ;
            m_pHID->startRecord();
            break;
        case 0xa0: qDebug() << "结束" ; break;

        case 0x61:
        {
            quint8 *stBuf=nullptr;
            if(data[2] == 0x14)
            {
                QString strSN = QString("SN: ");
                for(int i=0; i<10; i++)
                    strSN += QString::asprintf("%02X",data[i+6]);

                ui->labelSN->setText(strSN);

                stBuf = data + 16;
                //m_pHID->askStatus();
            }

            if(data[2] == 0x08)
            {

            }

            if(stBuf)
            {
                quint8 offset = 0;
                quint8 status = stBuf[offset];
                quint8 type = stBuf[offset+1];
                quint8 dpi = stBuf[offset+2];
                quint16 model = (stBuf[offset+6] << 8) | stBuf[offset+3];
                quint8 batt = stBuf[offset+4];
                quint8 ver = stBuf[offset+5];

                qDebug() << status << type << dpi << model << batt << ver;
            }
        }
            break;
        }
    });

    connect(ui->pushButtonTypeword,&QPushButton::clicked,this,[=]{
        DialogTypeWord dlg(this);
        dlg.exec();
    });

    connect(ui->pushButtonRound,&QPushButton::clicked,this,[=]{
        updateValue();
    });
    connect(ui->pushButtonRect,&QPushButton::clicked,this,[=]{
        updateValue();
    });

    connect(ui->pushButtonDevice,&QPushButton::clicked,this,[=]{
        DialogDeviceSet dlg(this);
        dlg.exec();
    });
    connect(ui->pushButtonMKey,&QPushButton::clicked,this,[=]{
        DialogMKeySet dlg(this);
        dlg.exec();
    });

    {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(":/images/logo.png"));
        trayIcon->setToolTip("NMY Pen");
        trayIcon->show();

        connect(trayIcon,&QSystemTrayIcon::activated,this,[=](QSystemTrayIcon::ActivationReason reason){
            if(reason != QSystemTrayIcon::Context)
            {
                if(this->isHidden() || this->isMinimized())
                    this->showNormal();
                else
                    this->hide();
            }
        });

        QAction *showAction = new QAction("", this);
        QAction *hideAction = new QAction("", this);
        QAction *exitAction = new QAction("", this);

        m_act0 = showAction;
        m_act1 = hideAction;
        m_act2 = exitAction;

        QMenu *trayMenu = new QMenu(this);
        trayMenu->addAction(showAction);
        trayMenu->addAction(hideAction);
        trayMenu->addSeparator();
        trayMenu->addAction(exitAction);

        connect(exitAction, &QAction::triggered, this, &QApplication::quit);
        connect(showAction, &QAction::triggered, this, &QMainWindow::showNormal);
        connect(hideAction, &QAction::triggered, this, &QMainWindow::hide);
        trayIcon->setContextMenu(trayMenu);
    }

    ui->labelColor->hide();

    ui->labelAudioSet->installEventFilter(this);
    ui->labelCloudCmd->installEventFilter(this);
    ui->page00->installEventFilter(this);
    ui->page01->installEventFilter(this);
    ui->page02->installEventFilter(this);
    ui->page03->installEventFilter(this);

    saveLoadParams(false);
    ui->pushButton0->click();
    updateValue();
}

void MainWindow::saveLoadParams(bool save)
{
    m_set->beginGroup("settings");
    if(save)
    {
        m_set->setValue("icolor0",m_iColor0);
        m_set->setValue("icolor3",m_iColor3);
        m_set->setValue("radius0",ui->horizontalSlider0->value());
        m_set->setValue("opacity0",ui->horizontalSlider1->value());
        m_set->setValue("radius1",ui->horizontalSlider2->value());
        m_set->setValue("radius2",ui->horizontalSlider4->value());
        m_set->setValue("opacity1",ui->horizontalSlider5->value());
        m_set->setValue("radius3",ui->horizontalSlider6->value());
        m_set->setValue("opacity3",ui->horizontalSlider7->value());
        m_set->setValue("round1",ui->pushButtonRound->isChecked());
        m_set->setValue("showBlack",ui->checkBoxBlack->isChecked());
        m_set->setValue("showTime",ui->lineEditSetTime->text());
        m_set->setValue("showIndex",m_show);
        m_set->setValue("showVoice",m_voice);
    }
    else
    {
        ui->horizontalSlider0->setValue(m_set->value("radius0",20).toInt());
        ui->horizontalSlider1->setValue(m_set->value("opacity0",80).toInt());
        ui->horizontalSlider2->setValue(m_set->value("radius1",200).toInt());
        ui->horizontalSlider4->setValue(m_set->value("radius2",200).toInt());
        ui->horizontalSlider5->setValue(m_set->value("opacity1",80).toInt());
        ui->horizontalSlider6->setValue(m_set->value("radius3",10).toInt());
        ui->horizontalSlider7->setValue(m_set->value("opacity3",80).toInt());
        ui->lineEditSetTime->setText(m_set->value("showTime","90").toString());
        ui->checkBoxBlack->setChecked(m_set->value("showBlack",false).toBool());
        m_iColor0 = m_set->value("icolor0").toInt();
        m_iColor3 = m_set->value("icolor3").toInt();
        m_show = m_set->value("showIndex",2).toInt();
        m_voice = m_set->value("showVoice",2).toInt();
        m_bRound = m_set->value("round1",true).toBool();

        ui->buttonGroupCount->button(-m_show-2)->click();
        ui->buttonGroupVoice->button(-m_voice-2)->click();
        ui->buttonGroupColor->button(-m_iColor0-2)->click();
        if(!m_bRound) ui->pushButtonRect->click();
    }
    m_set->endGroup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateValue()
{
    int nAlph = 0;
    switch(m_index)
    {
    case 0:
        m_iColor0= m_iColor;
        m_radius0 = ui->horizontalSlider0->value() / 2;
        nAlph = ui->horizontalSlider1->value() * 255 / 100.0;

        m_color0 = colors[m_iColor];
        m_color0.setAlpha(nAlph);
        break;

    case 1:
        m_radius1 = ui->horizontalSlider2->value() / 2;
        m_bRound = ui->pushButtonRound->isChecked();
        break;

    case 2:
        m_radius2 = ui->horizontalSlider4->value() / 2;
        nAlph = ui->horizontalSlider5->value() * 255 / 100.0;
        m_color2 = Qt::black;
        m_color2.setAlpha(nAlph);
        break;

    case 3:
        m_iColor3= m_iColor;
        m_radius3 = ui->horizontalSlider6->value();
        nAlph = ui->horizontalSlider7->value() * 255 / 100.0;
        m_color3 = colors[m_iColor];
        m_color3.setAlpha(nAlph);
        break;
    }

    pFuncPad->m_radius0 = m_radius0;
    pFuncPad->m_radius1 = m_radius1;
    pFuncPad->m_radius2 = m_radius2;
    pFuncPad->m_radius3 = m_radius3;

    pFuncPad->m_bRound = m_bRound;
    int showTime = 3600;
    if(m_show == 1) showTime = 1800;
    if(m_show == 2) showTime = 600;
    if(m_show == 3) showTime = ui->lineEditSetTime->text().toInt() * 60;
    pFuncPad->m_tmCount= showTime;
    pFuncPad->m_onlyBlack= ui->checkBoxBlack->isChecked();

    pFuncPad->m_color0 = m_color0;
    pFuncPad->m_color1 = m_color1;
    pFuncPad->m_color2 = m_color2;
    pFuncPad->m_color3 = m_color3;

    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.setRenderHint(QPainter::Antialiasing, true);

    p.drawImage(this->rect(),      QImage(QApplication::applicationDirPath()+"/images/bground.png"));
    p.drawImage(QRect(0,2,132,44), QImage(QApplication::applicationDirPath()+"/images/nmylogo.png"));
    {
        QImage batt(":/images/batt.png");

        QPainter p(&batt);
        p.fillRect(QRect(3,3,50/100.0 * 23,10),QBrush(Qt::green));
        ui->labelBattery->setPixmap(QPixmap::fromImage(batt));
    }

    QMainWindow::paintEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->labelAudioSet && event->type() == QEvent::MouseButtonRelease)
    {
        QProcess::startDetached("control", QStringList() << "mmsys.cpl" << "sounds,,0");
        QProcess::startDetached("sndvol", QStringList{});
    }

    if(watched == ui->labelCloudCmd && event->type() == QEvent::MouseButtonRelease)
    {
        DialogCloudCmd dlg(this);
        dlg.exec();
    }

    if(event->type() == QEvent::Paint)
    {
        static QImage bkImg = QImage(":/images/bk0.png");
        QWidget *Page = static_cast<QWidget *>(watched);
        QRect rc = Page->rect();
        QPainter p(Page);
        int mx = rc.center().x()-20;
        int my = rc.center().y();
        QPoint  Center(mx,my);

        if(ui->labelCloudCmd != watched && ui->labelAudioSet != watched)
            p.drawImage(rc,bkImg);

        if(watched == ui->page00)
        {
            int radius = m_radius0;
            QRect rcFlag = QRect(mx-radius,my-radius,radius*2,radius*2);

            p.setBrush(m_color0);
            p.setPen(Qt::NoPen);
            p.drawEllipse(rcFlag);
        }

        if(watched == ui->page01)
        {
            int dist = m_radius1;
            qreal factor = 1.5;

            QImage tmp = bkImg.copy(QRect(Center-QPoint(dist,dist),Center+QPoint(dist,dist)));
            QRect tarRect(Center-QPoint(dist*factor,dist*factor),Center+QPoint(dist*factor,dist*factor));

            QPainterPath path;
            path.addEllipse(tarRect);
            if(m_bRound)
                p.setClipPath(path);

            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.drawImage(tarRect,tmp);
        }

        if(watched == ui->page02)
        {
            int dist = m_radius2;
            QRect rcDest = QRect(Center-QPoint(dist,dist),Center+QPoint(dist,dist));

            p.fillRect(rc,m_color2);

            QPainterPath path;
            path.addEllipse(rcDest);
            p.setClipPath(path);

            p.drawImage(Center-QPoint(dist,dist),bkImg,rcDest);
        }

        if(watched == ui->page03)
        {
            p.fillRect(rc,Qt::gray);

            QPen LinePen(m_color3);
            LinePen.setWidth(m_radius3);
            LinePen.setCapStyle(Qt::RoundCap);
            p.setPen(LinePen);

            qreal fStep = rc.width()/20.0;

            QPoint A(20,my);
            for(int i=1; i<20; i++)
            {
                QPoint B(20 + i*fStep, my + rand()%50 * (rand() % 2 ? -1 : 1));
                p.drawLine(A,B);
                A = B;
            }
        }
    }

    return QMainWindow::eventFilter(watched,event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << event->key();
    QMainWindow::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (event->pos().y() < 60)
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
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << event->button();
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug() << "mouseDoubleClickEvent";
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }
    QMainWindow::mouseDoubleClickEvent(event);
}
