#include "ToggleButton.h"

static QColor s_colorChecked   = Qt::green;
static QColor s_colorUnchecked = Qt::gray;

void ToggleButton::setGlobalColor(const QColor&checkedColor,const QColor&UncheckedColor)
{
    s_colorChecked   = checkedColor;
    s_colorUnchecked = UncheckedColor;
}

ToggleButton::ToggleButton(QWidget *parent) : QCheckBox(parent)
{
   // setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet(R"(
            QCheckBox {
                min-width: 52px;
                min-height: 24px;
                border: none; }
        )");

    setColor(s_colorChecked,s_colorUnchecked);
    setCursor(Qt::PointingHandCursor);
}


void ToggleButton::setColor(const QColor&checkedColor,const QColor&UncheckedColor)
{
    m_colorChecked   = checkedColor;
    m_colorUnchecked = UncheckedColor;
    update();
}

void ToggleButton::setCheckedColor(const QColor&color)
{
    m_colorChecked = color;
    update();
}

void ToggleButton::setUnheckedColor(const QColor&color)
{
    m_colorUnchecked = color;
    update();
}

bool ToggleButton::event(QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease && isEnabled())
    {
        bool checked = this->isChecked();
        QCheckBox::setChecked(!checked);
        emit clicked(!checked);
        update();
        event->accept();
        return true;
    }
    return QCheckBox::event(event);
}

void ToggleButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    QRect rect = this->rect().adjusted(1,1,-1,-1);
    int nH = rect.height() ;
    bool checked = this->isChecked();
    if(checked)
    {
        painter.setPen(m_colorChecked);
        painter.setBrush(m_colorChecked);
    }
    else
    {
        painter.setPen(m_colorUnchecked);
        painter.setBrush(m_colorUnchecked);
    }
    painter.drawRoundedRect(rect,nH/2,nH/2);

    int nRadius = nH/2 -2;
    QRect rc = rect.adjusted(2,2,-2,-2);
    if(checked)
    {
        rc.setLeft(rc.right() - nRadius*2);
    }
    else
    {
        rc.setRight(rc.left() + nRadius*2);
    }

    if(checked)
    {
        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);
    }
    else
    {
        painter.setPen(0xE0E0E0);
        painter.setBrush(0xE0E0E0);
    }

    painter.drawRoundedRect(rc,nRadius,nRadius);

    event->accept();
}
