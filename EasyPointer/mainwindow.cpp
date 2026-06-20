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


#include "DialogTip.h"
#include "DialogRecord.h"
#include "DialogCloudCmd.h"
#include "DialogTypeWord.h"
#include "DialogDeviceSet.h"
#include "DialogMKeySet.h"

#include "TxASRClient.h"
#include "KeyBoardMonitor.h"

#include <Windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <QScreen>
#include <QApplication>

bool g_bCommentVer=true;

static QList<quint32> colors={0xFF0000,0x00FF00,0x0000FF,0xFFFFFF,0xFF8000,0x800080,0xFFFF00,0x00FFFF,0x000000};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("Nmy Pointer");

    // KeyBoardMonitor *pKBM = new KeyBoardMonitor(this);

    pFuncPad  = new DialogBoard();
    m_ModeTip = new DialogTip();
    m_RecPad  = new DialogRecord();

    m_pNoCnn = new DialogNoConnect(this);

    m_pTSet = new DialogTypeWord(this);
    m_pCmd = new DialogCloudCmd(this);
    m_RecPad->setRelate(m_pCmd,m_pTSet);

    m_pDSet = new DialogDeviceSet();
    m_pMSet = new DialogMKeySet();

    m_pHID = new CHidWorker();
    m_pCmd->m_pWork = m_pHID;
    m_pDSet->m_pWork = m_pHID;
    m_pCmd->m_pPad = pFuncPad;

    ui->labelSN->hide();
    ui->labelColor->hide();

    if(g_bCommentVer)
        ui->pushButton6->hide();

    //setStyleSheet("QLabel{color:white; font-weight:600;}");

    QCoreApplication::setOrganizationName("NMY");
    QCoreApplication::setApplicationName("NMYPointer");
    m_set = new QSettings();

    connect(ui->pushButtonExit,&QPushButton::clicked,this,[=]{ qApp->exit(); });
    connect(ui->pushButtonMin,&QPushButton::clicked,this,[=]{
        m_pNoCnn->hide();
        this->showMinimized();
    });

    connect(ui->buttonGroupMain,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id);
        if(index >= 4)
            index -= 4;
        else
            index += 3;

        ui->stackedWidget0->setCurrentIndex(index);
        ui->stackedWidget1->setCurrentIndex(index);
        ui->pushButtonRect2->setFixedSize(24,16);

        ui->labelContact->setHidden(true);
        ui->frameColor->setHidden(true);
        ui->stackedWidget1->setHidden(true);
        ui->labelNotice->setHidden(true);
        ui->frameSetting->setHidden(false);

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
            break;

        case 4:
            ui->labelNotice->setHidden(false);
            break;

        case 5:
            ui->labelNotice->setHidden(false);
            break;

        case 6:
            ui->labelContact->setHidden(false);
            //ui->frameSetting->setHidden(true);
            break;
        }

        if(index == 0) ui->buttonGroupColor->button(-m_iColor0-2)->click();
        if(index == 3) ui->buttonGroupColor->button(-m_iColor3-2)->click();

        ui->frameShape->setVisible(index == 2);
        ui->horizontalFrame->setVisible(index == 0);
        updateValue();
        saveLoadParams();
    });

    connect(ui->buttonGroupColor,&QButtonGroup::idClicked,this,[=](int id){
        if(m_bLoading) return;
        int index = abs(id)-2;
        m_iColor = index;

        updateValue();
        saveLoadParams();
    });
    connect(ui->buttonGroupSpot,&QButtonGroup::idClicked,this,[=](int id){
        if(m_bLoading) return;
        int index = abs(id)-2;
        m_iSpot = index;
        updateValue();
        saveLoadParams();
    });
    connect(ui->buttonGroupShape,&QButtonGroup::idClicked,this,[=](int id){
        if(m_bLoading) return;
        int index = abs(id)-2;
        m_nMgfShape = index;
        updateValue();
        saveLoadParams();
    });
    connect(ui->buttonGroupCount,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        m_show = index;
        updateValue();
        saveLoadParams();
    });

    for(int i=125; i<=500; i+=25)
    {
        ui->comboBoxEnlarge->addItem(QString::asprintf("%d%%",i));
    }

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

    connect(ui->comboBoxEnlarge,&QComboBox::activated,this,[=](int index){        
        if(m_bLoading) return;
        m_enlarge = (ui->comboBoxEnlarge->currentIndex() * 25 + 100)/100.0;
        updateValue();
    });

    connect(ui->comboBoxEffect,&QComboBox::activated,this,[=](int index){
        if(m_bLoading) return;
        m_iEffect= index;
        updateValue();
    });

    static QSettings regSet("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    ui->checkBoxStart->setChecked(!regSet.value("NMYPointer").toString().isEmpty());
    connect(ui->checkBoxStart,&QCheckBox::clicked,this,[=](bool checked){
        regSet.remove("NMYPointer");
        if(checked)
        {
            QString strFile = QString("\"%1\"").arg(QApplication::applicationFilePath().replace("/","\\"));
            regSet.setValue("NMYPointer",strFile);
        }
    });

    static auto *pAsrClient = DoASRWork(false);
    connect(pAsrClient,&TxAsrClient::onASRText,this,[=](const QString&text,int state){
        if(m_nModeS2 == 2)
        {
            m_RecPad->setOutsizeText(text,1);
            pAsrClient->translateText(text);
        }
        else
        {
            m_RecPad->setOutsizeText(text,state);
        }
   });
    connect(pAsrClient,&TxAsrClient::onTransText,this,[=](const QString&text,int state){
        m_RecPad->setOutsizeText(text,2);
    });
    connect(pAsrClient,&TxAsrClient::onASRConnect,this,[=](bool connect){
        ui->checkBoxTXASR->setChecked(connect);
    });
    connect(ui->checkBoxTXASR,&QCheckBox::clicked,this,[=](bool checked){
        DoASRWork(checked);
        if(!m_bConnected)
            pAsrClient->userMic(true);

        if(checked)
        {
            m_RecPad->show();
            m_RecPad->setMode(3);
        }
        else
        {
            m_RecPad->hide();
        }
    });

    connect(ui->checkBoxRecord,&QCheckBox::clicked,this,[=](bool checked){
        if(checked)
        {
            m_RecPad->show();
            m_RecPad->setMode(4);
            m_pHID->StarRecorFile(QString::asprintf("%d.wav",time(nullptr)));
        }
        else
        {
            m_RecPad->hide();
            m_pHID->StopRecorFile();
        }
    });

    connect(m_RecPad,&DialogRecord::onFlushText,this,[=](const QString&text){
        m_pCmd->startupApp(text);
    });

    connect(m_pHID,&CHidWorker::onConnect,this,[=](int nMode,bool connected){
        m_bConnected = connected;
        if(connected)
        {
            ui->labelConnect->setPixmap(QPixmap(nMode == 1 ? ":/images/2.4g.png" : ":/images/blue.png").scaled(24,24));
            trayIcon->setIcon(QIcon(":/images/logo.jpg"));
            setWindowIcon(QIcon(":/images/logo.jpg"));
            m_pNoCnn->accept();
        }
        else
        {
            ui->labelConnect->clear();
            ui->labelConnect->setText("未连接");
            trayIcon->setIcon(QIcon(":/images/logo-gray.png"));
            setWindowIcon(QIcon(":/images/logo-gray.png"));

            m_pNoCnn->setGeometry(this->geometry().adjusted(0,50,0,0));
            if(this->isVisible() && !this->isMinimized())
                m_pNoCnn->show();
            m_pTSet->hide();
            m_pCmd->hide();
        }
    });

    connect(m_pHID,&CHidWorker::onDevPcmData,this,[=](const QByteArray&pcm){
        if(m_nModeS2 == 255)
        {
            m_pHID->stopRecord();
            return;
        }

        if(m_nModeS2 != 0)
        {
            pAsrClient->userMic(false);
            pAsrClient->writePCM(pcm);
            m_RecPad->writePCM(pcm);
            m_RecPad->show();
        }
    });

    connect(m_pHID,&CHidWorker::onMicPcmData,this,[=](const QByteArray&pcm){
        m_RecPad->writePCM(pcm);
    });

    connect(pAsrClient,&TxAsrClient::onMicPcmData,this,[=](const QByteArray&pcm){
        m_RecPad->writePCM(pcm);
    });

    connect(pFuncPad,&DialogBoard::onCallunction,this,[=](int function=0){
        switch (function) {
        case 1:
            hide();
            m_RecPad->hide();
            pFuncPad->capScreen();
            pFuncPad->setMode(1);
            pFuncPad->showFullScreen();
            pFuncPad->raise();
            m_pHID->setMouse(true);
            QApplication::setOverrideCursor(Qt::BlankCursor);
            break;

        case 20:
            pFuncPad->hide();
            QApplication::setOverrideCursor(Qt::ArrowCursor);
            break;
        default:
            break;
        }
    });

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
        switch(cmd)
        {
        case 0xc0:
        case 0x91: qDebug() << "单击";
            //m_pHID->setMouse(true);
            pFuncPad->hide();
            m_press = 0;
            break;

        //
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc4:
        case 0xc5:
        case 0x92:
        {
            updateValue();
            m_pHID->setLaser(false);
            qDebug() << "双击";   // 仅切换功能
            m_nModeS1++;
            if(m_nModeS1 > 3)
                m_nModeS1 = 0;

            if(m_nModeS1 == 3)
                m_pHID->setMouse(true);

            m_ModeTip->showMode(m_nModeS1);
            if(m_nModeS1 == 0) ui->pushButton0->click();
            if(m_nModeS1 == 1) ui->pushButton1->click();
            if(m_nModeS1 == 2) ui->pushButton2->click();
            if(m_nModeS1 == 3) ui->pushButton3->click();
            m_press = 0;
            pFuncPad->hide();
        }
            break;

        case 0xc7:
        case 0xc8:
        case 0xc9:
        case 0xca:
        case 0x93: qDebug() << "长按" ;
            if(m_nModeS1 != 3)
            {
                if(pFuncPad->isVisible())
                {
                    m_pHID->setMouse(false);
                    QApplication::setOverrideCursor(Qt::BlankCursor);
                    pFuncPad->hide();
                    return;
                }
            }

            m_ModeTip->hide();
            if(m_nModeS1 == 0xFF) break;
            m_pHID->setMouse(true);
            m_press ++;
            if(m_nModeS1 != 3 || m_press >= 1)
                pFuncPad->setDrage();

            QTimer::singleShot(100,this,[=]{
                hide();
                pFuncPad->capScreen();
                pFuncPad->setMode(m_nModeS1);
                pFuncPad->showFullScreen();
                pFuncPad->raise();
                QApplication::setOverrideCursor(Qt::BlankCursor);
            });
            break;

        //case 0xcb:
        //case 0xcc:
        //case 0xcd:
        //case 0xce:
        //case 0xcf:
        case 0x94: qDebug() << "松开" ;
            QApplication::restoreOverrideCursor();
            m_pHID->setMouse(true);
            if(m_nModeS1 != 3)
            {
                QApplication::setOverrideCursor(Qt::BlankCursor);
                m_pHID->setMouse(false);
                if(!this->isMinimized())
                    show();
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

        case 0xd8:
        case 0xdb:
        case 0x9d: // s2 单击
            qDebug() << "m_nModeS2:" << m_nModeS2;
            if(m_nModeS2 == 0xFF)
            {
                break;
            }

            if(m_nModeS2 == 3)
            {
                ui->checkBoxRecord->click();
                break;
            }

            m_pHID->setRecordPlay(false);
            m_record = !m_record;
            if(m_nModeS2 == 0)
            {
                if(m_record)
                {
                    m_pHID->setRecordPlay(true);
                    m_pHID->startRecord();
                }
                else
                {
                    m_pHID->stopRecord();
                }
                m_RecPad->hide();
                break;
            }

            m_pHID->setRecordPlay(false);
            m_RecPad->setMode(m_nModeS2);
            if(m_record)
            {
                DoASRWork(true);
                m_pHID->startRecord();
                m_RecPad->show();
            }
            else
            {
                DoASRWork(false);
                m_pHID->stopRecord();
                m_RecPad->hide();
            }
            break;

        case 0xdc:
        case 0xdd:
        case 0xdf:

        case 0xe2:
        case 0x9e: // s2 双击
            m_record = false;
            m_pHID->stopRecord();
            m_pHID->setRecordPlay(false);
            m_RecPad->setPaintText(false);
            m_nModeS2++;
            if(m_nModeS2 > 3)
                m_nModeS2 = 0;
            if(m_nModeS2 == 1 || m_nModeS2 == 2)
                m_RecPad->setPaintText(true);
            m_ModeTip->showMode(4+m_nModeS2);
            break;

        case 0xe6:
        case 0x9f: qDebug() << "语音搜索" ; //云指令
            DoASRWork(true);
            m_pHID->setRecordPlay(false);
            m_pHID->startRecord();
            m_RecPad->setMode(3);
            m_RecPad->show();
            m_record = false;
            m_nModeS2 = 3;
            break;

        case 0xa0: qDebug() << "结束" ;
            DoASRWork(false);
            if(m_nModeS2 == 3)
                m_RecPad->DoFlush(true);
            m_pHID->setRecordPlay(false);
            m_pHID->stopRecord();
            m_RecPad->hide();
            m_record = false;
            m_nModeS2 = -1;
            break;

        case 0x1b:
            break;

        case 0x90:
            m_battery = data[5];
            ui->labelBattery->update();
            break;

        case 0x61:
        {
            quint8 *stBuf = nullptr;
            if(data[2] == 0x14)
            {
                QString strSN = QString("SN: ");
                for(int i=0; i<10; i++)
                    strSN += QString::asprintf("%02X",data[i+6]);

                ui->labelSN->setText(strSN);

                stBuf = data + 16;
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

                // qDebug() << status << type << dpi << model << batt << ver;
            }
        }
            break;
        }
    });

    connect(ui->pushButtonTypeword,&QPushButton::clicked,this,[=]{
        m_pTSet->show();
    });

    connect(ui->pushButtonRound,&QPushButton::clicked,this,[=]{
        updateValue();
    });
    connect(ui->pushButtonRect,&QPushButton::clicked,this,[=]{
        updateValue();
    });

    connect(ui->pushButtonDevice,&QPushButton::clicked,this,[=]{
        m_pDSet->show();
    });
    connect(ui->pushButtonMKey,&QPushButton::clicked,this,[=]{
        m_pMSet->show();
    });

    {
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(QIcon(":/images/logo.jpg"));
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

        QAction *showAction = new QAction("显示主界面", this);
        QAction *hideAction = new QAction("隐藏主界面", this);
        QAction *exitAction = new QAction("退出程序", this);

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
        trayMenu->setStyleSheet("color:red;");
    }

    ui->labelAudioSet->installEventFilter(this);
    ui->labelCloudCmd->installEventFilter(this);
    ui->labelAudioSet1->installEventFilter(this);
    ui->labelCloudCmd1->installEventFilter(this);
    ui->labelAutoStart->installEventFilter(this);
    ui->labelBattery->installEventFilter(this);
    ui->page00->installEventFilter(this);
    ui->page01->installEventFilter(this);
    ui->page02->installEventFilter(this);
    ui->page03->installEventFilter(this);

    saveLoadParams(false);
    ui->pushButton0->click();
    updateValue();

    m_EfOpticy = 0;
    QTimer *pTMEf = new QTimer(this);
    connect(pTMEf,&QTimer::timeout,this,[=]{
        if(ui->stackedWidget0->currentIndex() != 0)
            return;

        pTMEf->stop();
        int interval = 200;
        if(m_iEffect == 1)
        {
            static int nDir = 1;
            if(m_EfOpticy>240) nDir = -1;
            if(m_EfOpticy<80) nDir = 1;
            m_EfOpticy += 20 * nDir;
            ui->stackedWidget0->update();
        }

        if(m_iEffect == 2)
        {
            if(m_EfOpticy == 240)
                m_EfOpticy = 80;
            else
                m_EfOpticy = 240;
            ui->stackedWidget0->update();
            interval = 600;
        }
        pTMEf->start(interval);
    });
    pTMEf->start(20);
}

void MainWindow::saveLoadParams(bool save)
{
    if(m_bLoading) return;
    m_set->beginGroup("settings");
    if(save)
    {
        m_set->setValue("icolor0",m_iColor0);
        m_set->setValue("icolor3",m_iColor3);
        m_set->setValue("iSpot",m_iSpot);
        m_set->setValue("iEffect",m_iEffect);
        m_set->setValue("radius0",ui->horizontalSlider0->value());
        m_set->setValue("opacity0",ui->horizontalSlider1->value());
        m_set->setValue("radius1",ui->horizontalSlider2->value());
        m_set->setValue("radius2",ui->horizontalSlider4->value());
        m_set->setValue("opacity1",ui->horizontalSlider5->value());
        m_set->setValue("radius3",ui->horizontalSlider6->value());
        m_set->setValue("opacity3",ui->horizontalSlider7->value());
        m_set->setValue("MgfShape",m_nMgfShape);
        m_set->setValue("enlarge",ui->comboBoxEnlarge->currentIndex());
        m_set->setValue("showBlack",ui->checkBoxBlack->isChecked());
        m_set->setValue("showTime",ui->lineEditSetTime->text());
        m_set->setValue("showIndex",m_show);
        m_set->setValue("showVoice",m_voice);
    }
    else
    {
        m_bLoading = true;
        ui->horizontalSlider0->setValue(m_set->value("radius0",20).toInt());
        ui->horizontalSlider1->setValue(m_set->value("opacity0",80).toInt());
        ui->horizontalSlider2->setValue(m_set->value("radius1",200).toInt());
        ui->horizontalSlider4->setValue(m_set->value("radius2",200).toInt());
        ui->horizontalSlider5->setValue(m_set->value("opacity1",80).toInt());
        ui->horizontalSlider6->setValue(m_set->value("radius3",10).toInt());
        ui->horizontalSlider7->setValue(m_set->value("opacity3",80).toInt());
        ui->lineEditSetTime->setText(m_set->value("showTime","90").toString());
        ui->checkBoxBlack->setChecked(m_set->value("showBlack",false).toBool());
        ui->comboBoxEnlarge->setCurrentIndex(m_set->value("enlarge",3).toInt());
        m_iColor0 = m_set->value("icolor0").toInt();
        m_iColor3 = m_set->value("icolor3").toInt();
        m_iSpot   = m_set->value("iSpot").toInt();
        m_iEffect = m_set->value("iEffect").toInt();
        m_show    = m_set->value("showIndex",2).toInt();
        m_voice   = m_set->value("showVoice",2).toInt();
        m_nMgfShape  = m_set->value("MgfShape").toInt();
        m_iColor = m_iColor0;
        QTimer::singleShot(100,this,[=]{
            ui->buttonGroupCount->button(-m_show-2)->click();
            ui->buttonGroupSpot->button(-m_iSpot-2)->click();
            ui->buttonGroupColor->button(-m_iColor0-2)->click();
            ui->buttonGroupShape->button(-m_nMgfShape-2)->click();
            ui->comboBoxEffect->setCurrentIndex(m_iEffect);
            m_bLoading = false;
        });
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
        m_iColor0 = m_iColor;
        m_radius0 = ui->horizontalSlider0->value() / 2;
        nAlph = ui->horizontalSlider1->value() * 255 / 100.0;
        m_color0 = colors[m_iColor];
        m_color0.setAlpha(nAlph);
        break;

    case 1:
        m_radius1 = ui->horizontalSlider2->value() / 2;
        //m_nMgfShape = ui->pushButtonRound->isChecked();
        break;

    case 2:
        m_radius2 = ui->horizontalSlider4->value();
        nAlph = ui->horizontalSlider5->value() * 255 / 100.0;
        m_color2 = Qt::black;
        m_color2.setAlpha(nAlph);
        break;

    case 3:
        m_iColor3 = m_iColor;
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

    pFuncPad->m_nMgfShape = m_nMgfShape;
    pFuncPad->m_enlarge = m_enlarge;
    int showTime = 3600;
    if(m_show == 1) showTime = 1800;
    if(m_show == 2) showTime = 600;
    if(m_show == 3) showTime = ui->lineEditSetTime->text().toInt() * 60;
    pFuncPad->m_tmCount= showTime;
    pFuncPad->m_showTime= ui->checkBoxBlack->isChecked();

    pFuncPad->m_color0 = m_color0;
    pFuncPad->m_color1 = m_color1;
    pFuncPad->m_color2 = m_color2;
    pFuncPad->m_color3 = m_color3;
    pFuncPad->m_iSpot  = m_iSpot;
    pFuncPad->m_iEffect  = m_iEffect;

    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    p.setRenderHint(QPainter::Antialiasing, true);

    p.drawImage(this->rect(),      QImage(QApplication::applicationDirPath()+"/images/bground.png"));
    if(!g_bCommentVer)
    p.drawImage(QRect(0,2,132,44), QImage(QApplication::applicationDirPath()+"/images/nmylogo.png"));
}

bool MainWindow::event(QEvent *event)
{
    //qDebug() << event->type();
    if( (event->type() == QEvent::WindowActivate || event->type() == QEvent::Show)&& !m_bConnected)
    {
        if(!m_pNoCnn->isMaximized())
        {
            m_pNoCnn->setGeometry(this->geometry().adjusted(0,50,0,0));
            m_pNoCnn->show();

            m_pTSet->hide();
            m_pCmd->hide();

            this->raise();
        }
        else
        {
            m_pNoCnn->hide();
        }
    }
//event->type() == QEvent::WindowDeactivate ||
    if( event->type() == QEvent::Hide)
    {
        m_pNoCnn->hide();
    }
    return QMainWindow::event(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->labelAudioSet || watched == ui->labelAudioSet1)&& event->type() == QEvent::MouseButtonRelease)
    {
        QProcess::startDetached("control", QStringList() << "mmsys.cpl" << "sounds,,0");
        QProcess::startDetached("sndvol", QStringList{});
    }

    if((watched == ui->labelCloudCmd || watched == ui->labelCloudCmd1) && event->type() == QEvent::MouseButtonRelease)
    {
        m_pCmd->show();
    }

    if(event->type() == QEvent::Paint)
    {
        if(ui->labelBattery == watched)
        {
            QImage batt(":/images/batt.png");

            QPainter p(&batt);
            p.fillRect(QRect(3,3,m_battery/100.0 * 23,10),QBrush(Qt::green));

            QFont font = p.font();
            font.setBold(true);
            font.setPixelSize(12);
            p.setFont(font);
            QTextOption to;
            to.setAlignment(Qt::AlignCenter);
            QColor color = Qt::blue;
            if(m_battery<50)color = Qt::yellow;
            if(m_battery<20)color = Qt::red;
            p.setPen(color);
            p.drawText(ui->labelBattery->rect(),QString("%1").arg(m_battery),to);
            ui->labelBattery->setPixmap(QPixmap::fromImage(batt));
            ui->labelBattery->setToolTip(tr("剩余电量") + QString(": %1%").arg(m_battery));
        }

        static QImage bkImg = QImage(":/images/bk0.png");
        QWidget *Page = static_cast<QWidget *>(watched);
        QRect rc = Page->rect();
        QPainter p(Page);
        int mx = rc.center().x()-20;
        int my = rc.center().y();
        QPoint  Center(mx,my);

        if(watched == ui->page00)
        {
            p.drawImage(rc,bkImg);
            int radius = m_radius0;
            QRect rcFlag = QRect(mx-radius,my-radius,radius*2,radius*2);

            QColor tmpColor(m_color0);
            if(m_iEffect != 0)
            {
                int nEf = m_EfOpticy;
                if(nEf>240) nEf = 240;
                if(nEf <80) nEf = 80 ;
                tmpColor.setAlpha(nEf);
            }

            p.setBrush(tmpColor);
            p.setPen(Qt::NoPen);
            p.drawEllipse(rcFlag);
        }

        if(watched == ui->page01)
        {
            p.drawImage(rc,bkImg);
            int dist = m_radius1;
            qreal factor = m_enlarge;

            QImage tmp = bkImg.copy(QRect(Center-QPoint(dist,dist),Center+QPoint(dist,dist)));
            QRect tarRect(Center-QPoint(dist*factor,dist*factor),Center+QPoint(dist*factor,dist*factor));

            QPainterPath path;
            if(m_nMgfShape == 0)
            {
                path.addEllipse(tarRect);
            }
            if(m_nMgfShape == 1)
            {
                path.addRect(tarRect);
            }
            if(m_nMgfShape == 2)
            {
                int nH = tarRect.height();
                path.addRect(tarRect.adjusted(0,nH*0.3,0,-nH*0.3));
            }
            p.setClipPath(path);

            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.drawImage(tarRect,tmp);
        }

        if(watched == ui->page02)
        {
            p.drawImage(rc,bkImg);
            int dist = m_radius2;
            QRect rcDest = QRect(Center-QPoint(dist,dist),Center+QPoint(dist,dist));

            p.fillRect(rc,m_color2);

            QPainterPath path;
            if(m_iSpot == 0) path.addEllipse(rcDest);
            if(m_iSpot == 1) path.addRect(rcDest);
            if(m_iSpot == 2) path.addRect(rcDest.adjusted(0,dist*0.4,0,-dist*0.4));
            if(m_iSpot == 3) path.addRect(rcDest.adjusted(dist*0.3,0,-dist*0.3,0));
            p.setClipPath(path);

            p.drawImage(Center-QPoint(dist,dist),bkImg,rcDest);
        }

        if(watched == ui->page03)
        {
            p.drawImage(rc,bkImg);
            p.fillRect(rc,Qt::black);
            p.fillRect(rc.adjusted(5,5,-5,-5),QColor(36,184,172));

            QPen LinePen(m_color3);
            LinePen.setWidth(m_radius3);
            LinePen.setCapStyle(Qt::RoundCap);
            p.setPen(LinePen);

            qreal fStep = rc.width()/20.0;

            QPoint A(20,my);
            for(int i=1; i<20; i++)
            {
                QPoint B(10 + i*fStep, my + rand()%50 * (rand() % 2 ? -1 : 1));
                p.drawLine(A,B);
                A = B;
            }
        }
    }

    return QMainWindow::eventFilter(watched,event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "MainWindow::keyPressEvent" << event->key() << event;

    QMainWindow::keyPressEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_pNoCnn->isVisible())
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
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }

    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        event->accept();
    }

    QMainWindow::mouseDoubleClickEvent(event);
}


