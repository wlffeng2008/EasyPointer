#ifndef HIDWORKER_H
#define HIDWORKER_H

#include "hidapi.h"

#include "caudioplayer.h"

#include <QString>
#include <QRunnable>
#include <QThread>
#include <QList>
#include <QStringList>
#include <QMutex>

#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSink>
#include <QBuffer>
#include <QCoreApplication>
#include <QIODevice>

class CHidWorker : public QThread
{
    Q_OBJECT
public:
    CHidWorker();
    ~CHidWorker();
    void readSN();
    void setDPI(quint8 index=1);
    void setURL(bool on=true);
    void startRecord();
    void stopRecord();
    void changeRecord();
    void setRecordPlay(bool set=true);

    void hearBeat();
    void setMouse(bool on=true);
    void setMouseBtn(quint8 func=0xC0);
    void setLaser(bool on=true);
    void askStatus();
    void setSample(quint8 index=1);
    void setOnline(bool on=true);
    void sendKey(quint8 key1, quint8 key0=0);

    void close();

    void setHidVIDPID(quint16 VID, quint16 PID);
    QString getHidPath(){return m_strDevPath;}

protected:
    void run() override;

signals:
    void onDataIn(quint8 *,int);
    void onPCMData(quint8 *,int);
    void onDisconnect();

private:

    QMutex m_mutex;
    QString m_strDevPath;
    hid_device *m_pDev=nullptr;

    CAudioPlayer *m_pAPlayer=nullptr;

    quint16 m_VID = 0x248A;
    quint16 m_PID = 0x60AB;

    bool m_bRecord=false;
    bool m_bOutPlay=false;

    bool m_bEndWork = false;
    void sendCmd(quint8 *pCmd);
};

#endif // HIDWORKER_H
