#include "ToggleButton.h"

static QColor s_colorChecked   = 0x6329B6;
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
                min-width: 48px;
                min-height: 24px;
                border: none; }
        )");
    m_bchecked = this->isChecked() ;
    setColor(s_colorChecked,s_colorUnchecked);
    setCursor(Qt::PointingHandCursor);
}

bool ToggleButton::getChecked()
{
    return m_bchecked;
}

void ToggleButton::setChecked(bool checked)
{
    m_bchecked = checked;
    emit clicked(checked);
    update();
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
    if(event->type() == QEvent::MouseButtonRelease)
    {
        m_bchecked = !m_bchecked;
        update();
        emit clicked(m_bchecked);
        event->accept();
        return true;
    }
    return QCheckBox::event(event);
}

void ToggleButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this) ;
    painter.setRenderHint(QPainter::Antialiasing,true);
    QRect rect = this->rect().adjusted(1,1,-1,-1);
    int nH = rect.height() ;
    if(m_bchecked)
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
    if(m_bchecked)
    {
        rc.setLeft(rc.right() - nRadius*2);
    }
    else
    {
        rc.setRight(rc.left() + nRadius*2);
    }

    if(m_bchecked)
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
