#include "TxAsrClient.h"

#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioInput>
#include <QAudioSource>
#include <QAudioSink>
#include <QIODevice>

#include <QJsonDocument>
#include <QJsonObject>

#include <QDateTime>
#include <QWebSocket>

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUuid>
#include <QRandomGenerator>

TxAsrClient::TxAsrClient(const QUrl& url, bool bUseMic, QObject* parent)
    : QObject(parent)
{
    m_useMic = bUseMic;
    m_ws = new QWebSocket();

    connect(m_ws, &QWebSocket::connected, this, [=]{
        qDebug() << "ASR WebSocket Connected!";
        m_connect = true;
        if(m_useMic)
            startCapture();
    });

    connect(m_ws, &QWebSocket::textMessageReceived, this, [=](const QString& msg){
        // {"code":0,"result":{"slice_type":2,"voice_text_str":"你好"},...}
        QJsonDocument jDoc = QJsonDocument::fromJson(msg.toUtf8());
        QJsonObject jObj = jDoc.object();
        if (jObj["code"].toInt() == 0)
        {
            QJsonObject res = jObj["result"].toObject();
            QString strText(res["voice_text_str"].toString().trimmed());
            quint32 state = res["slice_type"].toInt() ; // 2 句子结束
            if (state == 2)
            {
                qDebug() << "识别结果:" << strText;
            }
            emit onASRText(strText,state);
        }
        else
        {
            qWarning().noquote() << "ASR Error:" << msg;
        }
    });

    connect(m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),this,[=](QAbstractSocket::SocketError e){
        qDebug() << "WS Error:" << e;
        stop();
    });

    m_url = url;
    start();
}

TxAsrClient::~TxAsrClient()
{

}

void TxAsrClient::stop()
{
    if (m_ws->state() == QAbstractSocket::ConnectedState)
        m_ws->sendTextMessage("{\"type\":\"end\"}");
    m_ws->close();
    m_working = false;
    m_connect = false;

    if(m_audioDevice) m_audioDevice->close();
    if(m_audioSource) m_audioSource->stop();
}

void TxAsrClient::start()
{
    m_buf.clear();
    m_working = true;
    m_ws->open(m_url);
}

void TxAsrClient::userMic(bool use)
{
    m_useMic = use;
}

void TxAsrClient::writePCM(const QByteArray&data)
{
    if (!data.isEmpty())
    {
        m_buf.append(data);
        if(m_buf.size() < 1280 || !m_connect)
            return;
        QByteArray tmp = m_buf.left(1280);
        m_buf.remove(0,1280);
        m_ws->sendBinaryMessage(tmp);
    }
};

void TxAsrClient::startCapture()
{
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    QAudioFormat fmt = inputDevice.preferredFormat();
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);
    m_working=true;
    m_audioSource = new QAudioSource(inputDevice,fmt);
    // 建议每 40ms 读一次 → 16000 * 2 * 0.04 = 1280 bytes
    m_audioDevice = m_audioSource->start();
    m_audioSource->setBufferSize(1280);
    connect(m_audioDevice, &QIODevice::readyRead, this, [=]{
        while (m_audioDevice->bytesAvailable() >= 1280 && m_useMic)
        {
            QByteArray pcm = m_audioDevice->read(1280);
            writePCM(pcm);
            if(!m_working)
            {
                stop();
                break;
            }
        }
    });
}


static QUrl buildAsrWsUrl(const QString& appId, const QString& secretId, const QString& secretKey)
{
    qint64 now = time(nullptr);;QDateTime::currentSecsSinceEpoch();
    qint64 expired = now + 86400;
    QString nonce = QString::number(QRandomGenerator::global()->generate() % 900000 + 100000);
    QString voiceId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QMap<QString, QString> params;
    params["secretid"]         = secretId;
    params["timestamp"]        = QString::number(now);
    params["expired"]          = QString::number(expired);
    params["nonce"]            = nonce;
    params["engine_model_type"]= "16k_zh";   // 腾讯混元大模型ASR用此或 16k_zh
    params["voice_format"]     = "1";        // 1=PCM
    params["needvad"]          = "1";
    params["voice_id"]         = voiceId;

    QString strRoot = QString("asr.cloud.tencent.com/asr/v2/") + appId + "?";

    bool first = true;
    QString signSrc = strRoot;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it)
    {
        if (!first) signSrc += "&"; first = false;
        signSrc += (it.key() + "=" + it.value());
    }
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha1, secretKey.toUtf8());
    hmac.addData(signSrc.toUtf8());
    QByteArray sig = hmac.result().toBase64();
    QString sigEncoded = QUrl::toPercentEncoding(sig);

    QString urlStr = QString("wss://") + strRoot;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        urlStr += (it.key() + "=" + QUrl::toPercentEncoding(it.value()) + "&");
    }
    urlStr += ("signature=" + sigEncoded);

    return QUrl(urlStr);
}

TxAsrClient *DoASRWork(bool toStart)
{
    QString NMYAId = "1253870935";
    QString NMYBId = QString("ebTCl") + QString("5Y6Vl") + QString("quvpc") + QString("DezCGA") + QString("sPmz1vi");
    QString NMYCId = "4zjLjxypUyGOSYZSAzmRs76vZ3OXb5e4";
    QString strKD = NMYBId.insert(0,"AKID") + QString("OtDP");
    QUrl url = buildAsrWsUrl(NMYAId.trimmed(), strKD.trimmed(), NMYCId.trimmed());
    static auto* client = new TxAsrClient(url);

    if(toStart)
        client->start();
    else
        client->stop();

    return client;
}