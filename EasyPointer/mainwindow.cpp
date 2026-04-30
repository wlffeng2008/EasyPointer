#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer>
#include <QApplication>

#include "DialogBoard.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <devicetopology.h>
#include <propkey.h>
#include <Functiondiscoverykeys_devpkey.h>
const PROPERTYKEY PKEY_AudioEndpoint_ControlPanel_ListenToThisDevice =
    {
        {0xa45c254e, 0xdf1c, 0x4efd, {0x80,0x20,0x67,0xd1,0x46,0xa8,0x50,0xe0}},
        19
};

// 打开/关闭 麦克风侦听此设备
// enable = true 开启侦听，false 关闭侦听
bool setMicrophoneListen(bool enable)
{
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;

    // 1. 创建设备枚举器
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                          nullptr,
                          CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator),
                          (void**)&pEnumerator);
    if (FAILED(hr)) return false;

    // 2. 获取默认录音设备（麦克风）
    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    if (FAILED(hr)) {
        pEnumerator->Release();
        return false;
    }

    // 3. 打开设备属性
    IPropertyStore* pProps = nullptr;
    hr = pDevice->OpenPropertyStore(STGM_READWRITE, &pProps);
    if (FAILED(hr)) {
        pDevice->Release();
        pEnumerator->Release();
        return false;
    }

    // 4. 关键：侦听此设备 对应的 PROPERTYKEY
    PROPERTYKEY key = PKEY_AudioEndpoint_ControlPanel_ListenToThisDevice;

    // 5. 写入值 VT_BOOL
    PROPVARIANT var;
    PropVariantInit(&var);
    var.vt = VT_BOOL;
    var.boolVal = enable ? VARIANT_TRUE : VARIANT_FALSE;

    hr = pProps->SetValue(key, var);
    PropVariantClear(&var);

    // 释放资源
    pProps->Release();
    pDevice->Release();
    pEnumerator->Release();

    return SUCCEEDED(hr);
}

// 获取当前侦听状态
bool getMicrophoneListenStatus()
{
    // 逻辑类似上面，我直接给你完整可用版本
    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                     __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

    pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
    IPropertyStore* pProps = nullptr;
    pDevice->OpenPropertyStore(STGM_READ, &pProps);

    PROPERTYKEY key = PKEY_AudioEndpoint_ControlPanel_ListenToThisDevice;
    PROPVARIANT var;
    PropVariantInit(&var);
    pProps->GetValue(key, &var);

    bool status = (var.boolVal == VARIANT_TRUE);
    PropVariantClear(&var);

    pProps->Release();
    pDevice->Release();
    pEnumerator->Release();

    return status;
}

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
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    static DialogBoard *pFuncPad = new DialogBoard();

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
            //QProcess::execute("./123.bat",{});
            //setMicrophoneListen(false);
            //qDebug() << "当前侦听状态：" << getMicrophoneListenStatus();
            //QProcess::startDetached("control", QStringList() << "mmsys.cpl" << "sounds,,0");

            {
                pFuncPad->capScreen();
                pFuncPad->setMode(4);
                QTimer::singleShot(50,this,[=]{
                    pFuncPad->showFullScreen();
                });
            }
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

    connect(m_pHID,&CHidWorker::onDataIn,this,[=](unsigned char *data,int len){

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
            break;

        case 0x92:
        {
            qDebug() << "双击";
            int mode = nFunc % 5;
            nFunc++;
            pFuncPad->hide();
            QTimer::singleShot(100,this,[=]{
                m_pHID->setMouse(true);
                pFuncPad->capScreen();
                pFuncPad->setMode(mode);
                pFuncPad->showFullScreen();
                pFuncPad->raise();
            });
        }
            break;

        case 0x93: qDebug() << "长按" ;
            pFuncPad->setDrage();
            break;

        case 0x94: qDebug() << "松开" ;
            pFuncPad->setDrage(false);
            break;

        case 0x9f: qDebug() << "声音外放....." ; break;
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

    p.drawImage(this->rect(), QImage(QApplication::applicationDirPath()+"/images/bground.png"));
    p.drawImage(QRect(0,2,132,44), QImage(QApplication::applicationDirPath()+"/images/nmylogo.png"));

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

