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

    * { font-size: 14px; font-weight: 400 ;}

    QSlider { border-radius: 4px; }
    QSlider::groove:horizontal { height: 8px; background: #DCDCDC; border-radius: 4px; }
    QSlider::sub-page:horizontal { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #FDDFBA, stop:1 #FFA73C);  border-radius: 4px; }
    QSlider::sub-page:horizontal:disabled { background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 white, stop:1 #B3B3B3);  border-radius: 4px; }
    QSlider::handle:horizontal {
        width: 6px;
        height: 12px;
        margin: -3px 0px;
        border-radius: 3px;
        background: black;
        border: 4px solid white; }

    QSlider::handle:horizontal:disabled { border: 2px solid #B3B3B3; }

    QSlider::handle:hover {
        background: black;
        border: 4px solid white; }
    QSlider::handle:pressed { background: #E0E0E0; border-color: #2D7FDD; }
    QLabel{color:white;font-weight:600;}
)");

    connect(ui->pushButtonExit,&QPushButton::clicked,this,[=]{ qApp->exit(); });

    connect(ui->buttonGroupMain,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        ui->stackedWidget0->setCurrentIndex(index);
    });

    connect(ui->buttonGroupColor,&QButtonGroup::idClicked,this,[=](int id){
        int index = abs(id)-2;
        QStringList colors={"#FF0000","#00FF00","#0000FF","#FFFFFF","#FF8000","#800080","#FFFF00","#00FFFF","#000000"};
        ui->labelColor->setStyleSheet(QString("QLabel{background-color:%1; border-radius:40px;}").arg(colors[index]));
    });

    ui->pushButtonMain->click();
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
    p.drawImage(QRect(2,2,132,44), QImage("./images/nmylogo.png"));

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

