#ifndef KEYBOARDMONITOR_H
#define KEYBOARDMONITOR_H

#include <QObject>
#include <QThread>

class KeyBoardMonitor : public QThread
{
    Q_OBJECT
public:
    explicit KeyBoardMonitor(QObject *parent = nullptr);

protected:
    void run() override;
};

#endif // KEYBOARDMONITOR_H
