#ifndef KEYBOARDMONITOR_H
#define KEYBOARDMONITOR_H

#include <QObject>
#include <QThread>
#

class KeyBoardMonitor : public QThread
{
    Q_OBJECT
public:
    explicit KeyBoardMonitor(QObject *parent = nullptr);

    void DoStop();

    bool m_bExit = false;

signals:
    void onKeypress(int code,bool pressed);
    void onMousepress(int message,quint32 key,bool pressed,bool bDbClk);
    void onInputData(int code, quint32 wParam, quint32 lParam, bool mouse);

protected:
    void run() override;
};

#endif // KEYBOARDMONITOR_H
