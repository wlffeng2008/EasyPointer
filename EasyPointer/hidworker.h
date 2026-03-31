#ifndef HIDWORKER_H
#define HIDWORKER_H
#include "hidapi.h"
#include <QString>
#include <QRunnable>
#include <Qthread>
#include <QList>
#include <QStringList>
#include <QMutex>

class CHidWorker : public QThread
{
    Q_OBJECT
public:
    CHidWorker();
    ~CHidWorker();
    void readSN() ;
    void setDPI(int nIndex=0) ;
    void setURL(int nOpen=1);
    void setSnap(int nOpen=1);
    void startRecord();
    void stopRecord();
    void close() ;

    void setHidVIDPID(unsigned short VID,unsigned short PID);
    QString getHidPath(){return m_strDevPath;}

protected:
    void run() override ;

signals:
    void onDataIn(unsigned char *,int) ;
    void onDisconnect() ;

private:

    QMutex m_mutex;
    QString m_strDevPath;
    hid_device *m_pDev=nullptr ;

    //unsigned short m_VID = 0x35BB ;
    //unsigned short m_PID = 0x1600 ;
    unsigned short m_VID = 0x248A;
    unsigned short m_PID = 0x60AB;

    bool m_bEndWork = false;
    void sendCmd(unsigned char nCmd,unsigned char nVal=0xFF);
};

#endif // HIDWORKER_H
