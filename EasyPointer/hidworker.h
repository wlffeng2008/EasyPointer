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
#include <QFile>
#include <QIODevice>


#include <QJsonDocument>
#include <QJsonObject>


#include <QWebSocket>
#include <QAudioInput>
#include <QAudioSource>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUrl>
#include <QDateTime>
#include <QUuid>
#include <QRandomGenerator>


class AsrClient : public QObject
{
    Q_OBJECT
public:
    AsrClient(const QUrl& url, QObject* parent = nullptr)
        : QObject(parent)
    {
        m_ws = new QWebSocket();
        connect(m_ws, &QWebSocket::connected, this, &AsrClient::onConnected);
        connect(m_ws, &QWebSocket::textMessageReceived, this, &AsrClient::onTextMsg);
        connect(m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                [](QAbstractSocket::SocketError e){ qDebug()<<"WS Error:"<<e; });

        m_url = url ;
        start();
    }
    ~AsrClient(){}

    void stop()
    {
        if (m_ws->state() == QAbstractSocket::ConnectedState)
            m_ws->sendTextMessage("{\"type\":\"end\"}");
        m_audioDevice->close();
        m_audioSource->stop();
        m_ws->close();
    }

    void start(){ m_ws->open(m_url); }

private slots:
    void onConnected()
    {
        qDebug() << "ASR WebSocket Connected!";
        startCapture();
    }

    void onTextMsg(const QString& msg)
    {
        // {"code":0,"result":{"slice_type":2,"voice_text_str":"你好"},...}
        QJsonDocument jDoc = QJsonDocument::fromJson(msg.toUtf8());
        QJsonObject jObj = jDoc.object();
        if (jObj["code"].toInt() == 0)
        {
            QJsonObject r = jObj["result"].toObject();
            if (r["slice_type"].toInt() == 2)  // 句子结束
                qDebug() << "识别结果:" << r["voice_text_str"].toString();
        }
        else
        {
            qWarning().noquote() << "ASR Error:" << msg;
        }
    }

    void startCapture()
    {
        QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
        QAudioFormat fmt = inputDevice.preferredFormat();
        fmt.setSampleRate(16000);
        fmt.setChannelCount(1);
        fmt.setSampleFormat(QAudioFormat::Int16);

        m_audioSource = new QAudioSource(inputDevice,fmt);
        // 建议每 40ms 读一次 → 16000 * 2 * 0.04 = 1280 bytes
        m_audioDevice = m_audioSource->start();
        m_audioSource->setBufferSize(1280);
        connect(m_audioDevice, &QIODevice::readyRead, this, [this]() {
            while (m_audioDevice->bytesAvailable() >= 1280)
            {
                QByteArray pcm = m_audioDevice->read(1280);
                if (!pcm.isEmpty() && m_ws->state() == QAbstractSocket::ConnectedState)
                    m_ws->sendBinaryMessage(pcm);
            }
        });
    }

private:
    QUrl  m_url ;
    QWebSocket   *m_ws = nullptr;
    QIODevice    *m_audioDevice = nullptr;
    QAudioSource *m_audioSource = nullptr;
};


bool PCMFile2WAVFile(const QString&strPCMFile,const QString&strWAVFile);
bool WAVFile2MP3File(const QString&strWAVFile,const QString&strMP3File);

void DoASRWork(bool toStart=true);

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

    void StarRecorFile(const QString&strFile);
    void StopRecorFile();

protected:
    void run() override;

signals:
    void onDataIn(quint8 *,int);
    void onPCMData(const QByteArray&data);
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

    QFile m_RecFile;
    bool m_bRecorFile=false;
    QString m_strFile;
    QString m_strTemp;
    bool WritePCMData(const QByteArray&data);
};

#endif // HIDWORKER_H
