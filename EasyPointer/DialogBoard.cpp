#include "DialogBoard.h"
#include "qtimer.h"
#include "ui_DialogBoard.h"
#include <QPainter>
#include <QScreenCapture>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainterPath>


DialogBoard::DialogBoard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogBoard)
{
    ui->setupUi(this);    
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(true);

    m_tmBlack = new QTimer(this);
    connect(m_tmBlack,&QTimer::timeout,this,[=]{
        if(m_mode == 4 && this->isVisible())
        {
            m_tmCount--;
            update();
        }
    });
    m_tmBlack->start(1000);
}

DialogBoard::~DialogBoard()
{
    delete ui;
}

void DialogBoard::capScreen()
{
    m_screen = QApplication::screens().at(0)->grabWindow();
}

void DialogBoard::setMode(int mode)
{
    m_mode = mode;
    m_tmCount=1800;
    clearLines();
}

bool DialogBoard::event(QEvent *event)
{
   return QDialog::event(event);
}

void DialogBoard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.drawPixmap(0,0,m_screen);

    switch(m_mode)
    {
    case 0:
    {
        int dist = m_radius0;
        QRect tarRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist));
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_color0);
        painter.drawEllipse(tarRect);
    }
    break;

    case 1:
    {
        int dist = m_radius1;
        QPixmap tmp = m_screen.copy(QRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist)));

        qreal factor = 1.5;
        QRect tarRect(m_curPos-QPoint(dist*factor,dist*factor),m_curPos+QPoint(dist*factor,dist*factor));

        QPainterPath path;
        path.addEllipse(tarRect);
        painter.setClipPath(path);

        painter.drawPixmap(tarRect,tmp);
    }
    break;

    case 2:
    {
        painter.fillRect(this->rect(),m_color2);

        int dist = m_radius2;

        QPainterPath path;
        path.addEllipse(QRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist)));
        if(m_bRound)
            painter.setClipPath(path);

        painter.drawPixmap(m_curPos-QPoint(dist,dist),m_screen,QRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist)));
    }
    break;

    case 3:
    {
        painter.setRenderHint(QPainter::Antialiasing);
        QPen LinePen(m_color3,m_radius3,Qt::SolidLine,Qt::RoundCap);
        painter.setPen(LinePen);
        for(const linePoints &line:std::as_const(m_lines))
        {
            int count = line.count();
            for(int i=0; i<count-1; i++)
            {
                painter.drawLine(line[i],line[i+1]);
            }
        }

        int count = m_record.count();
        for(int i=0; i<count-1; i++)
        {
            painter.drawLine(m_record[i],m_record[i+1]);
        }
    }
    break;

    case 4:
    {
        painter.fillRect(this->rect(),QBrush(Qt::black));
        QPen pen = painter.pen();
        pen.setColor(Qt::white);
        QFont font = painter.font();
        font.setPixelSize(100);
        painter.setPen(pen);
        painter.setFont(font);

        int remain = m_tmCount;
        int hour = remain/3600;
        int min = remain%3600/60;
        int sec = remain%60;
        QString strInfo = QString::asprintf("%02d:%02d:%02d",hour,min,sec);

        painter.drawText(this->rect(), Qt::AlignCenter, strInfo);
    }
    break;
    }
}

void DialogBoard::clearLines()
{
    m_lines.clear();
    m_record.clear();
    update();
}

void DialogBoard::setDrage(bool set)
{
    m_bDraging=set;
    if(set)
    {
        m_record.clear();
        m_record.push_back(QCursor::pos());
    }
    else
    {
        m_lines.push_back(m_record);
        update();
    }
}

void DialogBoard::mousePressEvent(QMouseEvent *event)
{
    m_bDraging=true;
    m_record.clear();
    m_record.push_back(event->pos());
    update();
    QDialog::mousePressEvent(event);
}

void DialogBoard::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bDraging)
    {
        m_lines.push_back(m_record);
        update();
    }
    m_bDraging=false;
    QDialog::mouseReleaseEvent(event);
}

void DialogBoard::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_lines.clear();
    m_record.clear();
    update();
}

void DialogBoard::mouseMoveEvent(QMouseEvent *event)
{
    m_curPos = event->pos();
    if(m_bDraging)
    {
        m_record.push_back(event->pos());
    }
    update();

    QDialog::mouseMoveEvent(event);
}

void DialogBoard::wheelEvent(QWheelEvent *event)
{
    QDialog::wheelEvent(event);
}

void DialogBoard::keyPressEvent(QKeyEvent *event)
{
    qDebug() << event;
    if(m_mode==4)
        hide();
    QDialog::keyPressEvent(event);
}

void DialogBoard:: keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
        hide();
    QDialog::keyReleaseEvent(event);
}

