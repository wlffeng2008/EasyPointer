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
#include <QAudioSource>

#include <QFile>


bool PCMFile2WAVFile(const QString&strPCMFile,const QString&strWAVFile);
bool WAVFile2MP3File(const QString&strWAVFile,const QString&strMP3File);

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
    void askBattery();
    void setOnline(bool on=true);
    void setSample(quint8 index=1);
    void sendKey(quint8 key1, quint8 key0=0);
    void sendCmbKey(quint16 key);
    void setTime();
    void userNew24G(bool use);

    void close();

    void setHidVIDPID(quint16 VID, quint16 PID);
    QString getHidPath(){return m_strDevPath;}

    void StarRecorFile(const QString&strFile);
    void StopRecorFile();

protected:
    void run() override;

signals:
    void onDataIn(quint8 *,int);
    void onDevPcmData(const QByteArray&pcm);
    void onMicPcmData(const QByteArray&pcm);
    void onHIDConnect(int nMode, bool connected=true);
    void onRecordFile(quint32 duration);

private:

    QString m_strDevPath;
    hid_device *m_pDev=nullptr;

    CAudioPlayer *m_pAPlayer=nullptr;

    quint16 m_VID = 0x248A;
    quint16 m_PID = 0x60AB;

    bool m_bOutPlay=false;
    bool m_bRecord=false;
    bool m_bNew24G=false;

    bool m_bEndWork = false;
    void sendCmd(quint8 *pCmd);


    QIODevice    *m_audioDevice = nullptr;
    QAudioSource *m_audioSource = nullptr;
    bool startCapture();

    quint32 m_recordTime=0;
    QFile   m_RecFile;
    QString m_strFile;
    QString m_strTemp;
    bool WritePCMData(const QByteArray&data);
};

#endif // HIDWORKER_H
