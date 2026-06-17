#include "TxAsrClient.h"
#include "qtimer.h"

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

    m_http = new HttpHandler();

    m_pTMPush = new QTimer(this);

    connect(m_ws, &QWebSocket::connected, this, [=]{
        qDebug() << "ASR WebSocket Connected!";
        m_connect = true;
        emit onASRConnect(true);
        if(m_useMic)
            startCapture();
    });

    connect(m_pTMPush,&QTimer::timeout,this,[=]{
        m_pTMPush->stop();
        emit onASRText(m_strText,2);
    });

    connect(m_ws, &QWebSocket::textMessageReceived, this, [=](const QString& msg){
        // {"code":0,"result":{"slice_type":2,"voice_text_str":"你好"},...}
        QJsonDocument jDoc = QJsonDocument::fromJson(msg.toUtf8());
        QJsonObject jObj = jDoc.object();
        if (jObj["code"].toInt() == 0)
        {
            QJsonObject res = jObj["result"].toObject();
            m_strText = res["voice_text_str"].toString().trimmed();
            quint32 state = res["slice_type"].toInt() ; // 2 句子结束
            m_pTMPush->stop();
            if (state == 2)
            {
                m_pTMPush->start(10);
                qDebug() << "识别结果:" << m_strText;
            }
            else
            {
                m_pTMPush->start(800);
            }
        }
        else
        {
            qWarning().noquote() << "ASR Error:" << msg;
        }
    });

    connect(m_ws, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),this,[=](QAbstractSocket::SocketError e){
        qDebug() << "QWebSocket Error:" << e;
        stop();
    });

    m_url = url;
}

TxAsrClient::~TxAsrClient()
{

}

void TxAsrClient::stop()
{
    if (m_ws->state() == QAbstractSocket::ConnectedState)
    {
        m_ws->sendTextMessage("{\"type\":\"end\"}");
        m_ws->close();
    }
    m_working = false;
    m_connect = false;
    emit onASRConnect(false);

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
    m_working = true;
    m_audioSource = new QAudioSource(inputDevice,fmt);
    m_audioDevice = m_audioSource->start();
    m_audioSource->setBufferSize(1280);  // 建议每 40ms 读一次 → 16000 * 2 * 0.04 = 1280 bytes
    connect(m_audioDevice, &QIODevice::readyRead, this, [=]{
        while (m_audioDevice->bytesAvailable() >= 1280 && m_useMic)
        {
            QByteArray pcm = m_audioDevice->read(1280);
            writePCM(pcm);
            emit onMicPcmData(pcm);
            if(!m_working)
            {
                stop();
                break;
            }
        }
    });
}


void TxAsrClient::translateText(const QString &sourceText,const QString &from,const QString &to)
{
    QString NMYAId = "1253870935";
    QString NMYBId = QString("ebTCl") + QString("5Y6Vl") + QString("quvpc") + QString("DezCGA") + QString("sPmz1vi");
    QString NMYCId = "4zjLjxypUyGOSYZSAzmRs76vZ3OXb5e4";
    QString strKD = NMYBId.insert(0,"AKID") + QString("OtDP");

    const QString SecretId = strKD;
    const QString SecretKey = NMYCId;
    const QString Region = "ap-shanghai";

    // --- 1. 准备公共参数 ---
    QString action = "TextTranslate";
    QString service = "tmt";
    QString version = "2018-03-21";
    QString host = "tmt.tencentcloudapi.com";
    QString urlStr = "https://" + host;

    // 获取 UTC 时间戳
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();
    QString date = QDateTime::fromSecsSinceEpoch(timestamp).toUTC().toString("yyyy-MM-dd");

    // --- 2. 构造请求体 (Payload) ---
    QJsonObject payloadObj;
    payloadObj.insert("SourceText", sourceText);
    payloadObj.insert("Source", from);    // 源语言：中文
    payloadObj.insert("Target", to);      // 目标语言：英文
    payloadObj.insert("ProjectId", 0);

    QJsonDocument payloadDoc(payloadObj);
    QByteArray payload = payloadDoc.toJson(QJsonDocument::Compact);

    // --- 3. 生成 TC3-HMAC-SHA256 签名 ---
    // 3.1 拼接规范请求串
    QString canonicalReq = QString("POST\n/\n\n")
                           + "content-type:application/json; charset=utf-8\nhost:" + host + "\n\n"
                           + "content-type;host\n"
                           + QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex();

    // 3.2 拼接待签名字符串
    QString credentialScope = date + "/" + service + "/tc3_request";
    QString stringToSign = "TC3-HMAC-SHA256\n" + QString::number(timestamp) + "\n"
                           + credentialScope + "\n"
                           + QCryptographicHash::hash(canonicalReq.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 3.3 计算派生密钥
    QMessageAuthenticationCode keyDate(QCryptographicHash::Sha256);
    keyDate.setKey(("TC3" + SecretKey).toUtf8());
    keyDate.addData(date.toUtf8());

    QMessageAuthenticationCode keyService(QCryptographicHash::Sha256);
    keyService.setKey(keyDate.result());
    keyService.addData(service.toUtf8());

    QMessageAuthenticationCode keySigning(QCryptographicHash::Sha256);
    keySigning.setKey(keyService.result());
    keySigning.addData("tc3_request");

    // 3.4 计算签名
    QMessageAuthenticationCode finalSign(QCryptographicHash::Sha256);
    finalSign.setKey(keySigning.result());
    finalSign.addData(stringToSign.toUtf8());
    QString signature = finalSign.result().toHex();

    // 3.5 拼接 Authorization
    QString authorization = "TC3-HMAC-SHA256 "
                            "Credential=" + SecretId + "/" + credentialScope + ", "
                                                                 "SignedHeaders=content-type;host, "
                                                                 "Signature=" + signature;

    // --- 4. 发送 HTTPS 请求 ---
    QNetworkRequest request;
    request.setUrl(QUrl(urlStr));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setRawHeader("Host", host.toUtf8());
    request.setRawHeader("X-TC-Action", action.toUtf8());
    request.setRawHeader("X-TC-Version", version.toUtf8());
    request.setRawHeader("X-TC-Region", Region.toUtf8());
    request.setRawHeader("X-TC-Timestamp", QString::number(timestamp).toUtf8());
    request.setRawHeader("Authorization", authorization.toUtf8());

    static QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply *reply){
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray responseData = reply->readAll();
            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &jsonError);

            if (jsonError.error == QJsonParseError::NoError)
            {
                QJsonObject root = doc.object();
                qDebug() << root;
                if (root.contains("Response"))
                {
                    QJsonObject response = root.value("Response").toObject();
                    QString text = response.value("TargetText").toString();
                    qDebug() << "翻译结果：" << text;

                    emit onTransText(text,0);
                }
            }
        }
        else
        {
            qDebug() << "网络错误：" << reply->errorString();
        }
        reply->deleteLater();
    });

    manager->post(request, payload);
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
    static auto* client = new TxAsrClient(url,false);

    if(toStart)
        client->start();
    else
        client->stop();

    return client;
}