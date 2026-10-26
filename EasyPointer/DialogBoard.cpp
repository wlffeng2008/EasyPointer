#include "DialogBoard.h"
#include "qtimer.h"
#include "ui_DialogBoard.h"
#include <QPainter>
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPainterPath>


static qreal globalScale() {
    qreal s = 1.0;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const auto screens = QGuiApplication::screens();
    for (QScreen *sc : screens)
        s = qMax(s, sc->devicePixelRatio());   // 多屏取最大，避免跨屏拖动时模糊
#else
    s = qApp->desktop()->devicePixelRatio();
#endif
    return s;
}


DialogBoard::DialogBoard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogBoard)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
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

    m_EfOpticy = 0;
    QTimer *pTMEf = new QTimer(this);
    connect(pTMEf,&QTimer::timeout,this,[=]{
        pTMEf->stop();
        int interval = 200;
        if(m_iEffect == 1)
        {
            static int nDir = 1;
            if(m_EfOpticy>240) nDir = -1;
            if(m_EfOpticy<80) nDir = 1;
            m_EfOpticy += 20 * nDir;
            update();
        }

        if(m_iEffect == 2)
        {
            if(m_EfOpticy == 240)
                m_EfOpticy = 80;
            else
                m_EfOpticy = 240;
            update();
            interval = 600;
        }
        pTMEf->start(interval);
    });
    pTMEf->start(200);
    m_display = globalScale();
}

DialogBoard::~DialogBoard()
{
    delete ui;
}

void DialogBoard::capScreen()
{
    m_screen = QApplication::screens().at(0)->grabWindow(0);
    //m_screen.save("./test.png");
}

void DialogBoard::setMode(int mode)
{
    qDebug() << "DialogBoard::setMode" << mode;
    m_mode = mode;
    m_curPos=QCursor::pos();
    clearLines();
}

void DialogBoard::showBlack(bool show)
{
    if(!show)
    {
        hide();
        return;
    }

    m_showTime=false;
    m_mode = 4;
    clearLines();
    showFullScreen();
    raise();
    QCursor::setPos(0xFFF,0xFFF);
    m_bActKeypress=false;
    QTimer::singleShot(3000,this,[=]{ m_bActKeypress = true; });
}

void DialogBoard::showMagnify(bool show)
{
    //m_bRound=true;
    emit onCallunction(show ? 1 : 20);
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

    QPoint newPos = m_curPos;
    newPos.setX(newPos.x() * m_display);
    newPos.setY(newPos.y() * m_display);

    switch(m_mode)
    {
    case 0:
    {
        int dist = m_radius0 ;
        QRect tarRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist));
        QColor tmpColor(m_color0);
        if(m_iEffect != 0)
        {
            int nEf = m_EfOpticy;
            if(nEf>255) nEf = 255;
            if(nEf < 0) nEf = 0  ;
            tmpColor.setAlpha(nEf);
        }

        painter.setBrush(tmpColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(tarRect);
    }
    break;

    case 1:
    {
        int dist = m_radius1;
        if(dist<20) dist=20;
        if(dist>500) dist=500;
        QPixmap tmp = m_screen.copy(QRect(newPos-QPoint(dist,dist),newPos+QPoint(dist,dist)));

        qreal factor = m_enlarge;
        if(factor <= 0)  factor = 1.5;
        if(factor > 4.0) factor = 4.0;
        QRect tarRect(m_curPos-QPoint(dist*factor,dist*factor),m_curPos+QPoint(dist*factor,dist*factor));

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
            path.addRect(tarRect.adjusted(0,dist*0.4,0,-dist*0.4));
        }
        painter.setClipPath(path);

        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(tarRect,tmp);
    }
    break;

    case 2:
    {
        painter.fillRect(this->rect(),m_color2);

        int dist = m_radius2;
        QRect tmp(QRect(m_curPos-QPoint(dist,dist),m_curPos+QPoint(dist,dist)));

        QPainterPath path;
        if(m_iSpot == 0) path.addEllipse(tmp);
        if(m_iSpot == 1) path.addRect(tmp);
        if(m_iSpot == 2) path.addRect(tmp.adjusted(0,dist*0.4,0,-dist*0.4));
        if(m_iSpot == 3) path.addRect(tmp.adjusted(dist*0.3,0,-dist*0.3,0));
        painter.setClipPath(path);

        painter.drawPixmap(QPoint(0,0),m_screen);
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
            QPainterPath path;
            path.moveTo(line[0]);
            for(int i=1; i<count; i++)
            {
                path.lineTo(line[i]);
            }
            painter.drawPath(path);
        }

        int count = m_record.count();
        if(count == 0) break;

        QPainterPath path;
        path.moveTo(m_record[0]);
        for(int i=1; i<count; i++)
        {
            path.lineTo(m_record[i]);
        }
        painter.drawPath(path);
    }
    break;

    case 4:
    {
        painter.fillRect(this->rect(),QBrush(Qt::black));
        QPen pen = painter.pen();
        pen.setColor(Qt::white);
        QFont font = painter.font();
        font.setPixelSize(200);
        painter.setPen(pen);
        painter.setFont(font);

        if(m_showTime)
        {
            int remain = m_tmCount;
            int hour = remain/3600;
            int min = remain%3600/60;
            int sec = remain%60;
            QString strInfo = QString::asprintf("%02d:%02d:%02d",hour,min,sec);

            painter.drawText(this->rect(), Qt::AlignCenter, strInfo);
            if(remain<=0)
                hide();
        }
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


void DialogBoard::hideEvent(QHideEvent *event)
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QDialog::hideEvent(event);
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
    QPoint cur = event->pos();
    m_curPos = cur;

    if(m_bDraging)
    {
        m_record.push_back(cur);
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
    //if(m_mode == 4 && m_bActKeypress)
    //    this->hide();

    QDialog::keyPressEvent(event);
}

void DialogBoard:: keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape && m_bActKeypress)
        this->hide();

    QDialog::keyReleaseEvent(event);
}

