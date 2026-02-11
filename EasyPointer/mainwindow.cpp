#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>

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
        if(m_pActive)m_pActive->setChecked(false);
        m_pActive = ui->buttonGroupMain->buttons().at(index);
        ui->stackedWidget0->setCurrentIndex(index);
        ui->stackedWidget1->setCurrentIndex(index);

        ui->frame->setHidden(index == 4);
        ui->stackedWidget1->setHidden(index == 4);
        ui->labelNotice->setHidden(index != 4);
        ui->labelContect->setHidden(true);
    });

    connect(ui->buttonGroupBottom,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        if(m_pActive)m_pActive->setChecked(false);
        m_pActive = ui->buttonGroupBottom->buttons().at(index);
        ui->stackedWidget0->setCurrentIndex(5+index);

        ui->frame->setHidden(true);
        ui->stackedWidget1->setHidden(true);
        ui->labelNotice->setHidden(true);
        if(index == 0)
        {
            ui->labelContect->setHidden(true);
            ui->labelNotice->setHidden(false);
        }

        if(index == 1)
        {
            ui->labelContect->setHidden(false);
            ui->labelNotice->setHidden(true);
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
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
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
        move(event->globalPos() - m_dragPosition);
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

