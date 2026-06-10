#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QCheckBox>
#include <QWidget>
#include <QStyle>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>

class ToggleButton : public QCheckBox
{
    Q_OBJECT

public:
    ToggleButton(QWidget *parent = nullptr);

    bool getChecked();
    void setChecked(bool checked);
    void setColor(const QColor&checkedColor,const QColor&UncheckedColor);
    void setCheckedColor(const QColor&color);
    void setUnheckedColor(const QColor&color);

    static void setGlobalColor(const QColor&checkedColor,const QColor&UncheckedColor) ;

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_bchecked = false ;
    QColor m_colorChecked = 0x6329B6 ;
    QColor m_colorUnchecked = Qt::gray ;
};

#endif // TOGGLEBUTTON_H
