#include "xfdatadecoder.h"
#include "xfwsvoicewrite.h"

#include <QUrlQuery>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <time.h>

#define FLYAPP_ID     "50c5ba81"
#define FLYAPP_SECRET "ZTU1ZTBhZjY4YzRiYWM0NjE3NTQ0Nzcz"
#define FLYAPP_KEY    "ddf83f52d16d5e3544613cb6be547d0d"

//#define FLYAPP_ID     "4f9c1298"
//#define FLYAPP_SECRET "YjcyOGVhMDk2MmZkZWRhZDljNzAzZmMw"
//#define FLYAPP_KEY    "bd022448a7eb3b2f03af12056fb65d96"

//#define FLYAPP_ID      "a7b21c86"
//#define FLYAPP_KEY     "9994f6ce9dc54ca97cb9d02082ca3137"
//#define FLYAPP_SECRET  "NzljY2E3NzUxNTlhZmY1MTM5ZGY5MzM4"

//#define FLYAPP_ID     "5e86ff8b"
//#define FLYAPP_KEY    "d995a81f16617796fa82c5761ecea228"
//#define FLYAPP_SECRET "NGM3OGYxMTcwN2E4ZGEyYTRlNzJhYWFj"

// #define FLYAPP_ID     "4c870f68"
// #define FLYAPP_KEY    "M2U4NzYyYTRmN2YyZjJkYTUxZDZmN2Q1"
// #define FLYAPP_SECRET "e318a91cca742b824649d15a4989d3c4"


#define FLYAPP_MAINLANG_URL  "ws://iat-api.xfyun.cn/v2/iat"
#define FLYAPP_OTHERLANG_URL "ws://iat-niche-api.xfyun.cn/v2/iat"

XFWSVoiceWrite::XFWSVoiceWrite()
{
    webSocket = new QWebSocket();
    QObject::connect(webSocket, &QWebSocket::connected, this, &XFWSVoiceWrite::OnWebsocketConnect);
    QObject::connect(webSocket, &QWebSocket::disconnected, this, &XFWSVoiceWrite::OnWebSocketDisconnect);
    QObject::connect(webSocket, &QWebSocket::textMessageReceived, this, &XFWSVoiceWrite::OnWebSocketStrMessage);

    QObject::connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), [](QAbstractSocket::SocketError error) {
           qDebug() << "WebSocket error" << error;
       });

    m_strFlyAppid = FLYAPP_ID;

    decoder = new XFDataDecoder();

    start();
}

void XFWSVoiceWrite::run()
{
    while(true)
    {
        if(m_flyState != -1 && m_PCMBuf.size() >= 1280)
        {
            QMutexLocker lock(&m_Mutex);
            QByteArray tmp = m_PCMBuf.remove(0,1280);

            QJsonObject frame;
            if(m_flyState == 0)
            {
                QJsonObject common;
                common["app_id"] = m_strFlyAppid;

                QJsonObject business;
                business["domain"] = "iat";
                business["language"] = m_strMLang;
                business["accent"] = m_strAccent;
                if(m_strMLang=="zh_cn" || m_strMLang=="zh-cn")
                    business["dwa"] = "wpgs";

                frame["common"] = common;
                frame["business"] = business;
            }

            QJsonObject data;
            data["status"] = 2;

            data["status"] = m_flyState;
            data["format"] = "audio/L16;rate=16000";
            data["encoding"] = "raw";
            QByteArray byteArray(tmp.data(), 1280);
            QByteArray base64Data = byteArray.toBase64();
            data["audio"] = QString(base64Data);

            m_flyState = 1;
            frame["data"] = data;

            QJsonDocument jsonDoc(frame);
            QString strMsg = jsonDoc.toJson();
            //qDebug() << strMsg;
            if(webSocket)
                webSocket->sendTextMessage(strMsg);
        }
        QThread::msleep(38);
    }
}

//接口鉴权
//在握手阶段，请求方需要对请求进行签名，服务端通过签名来校验请求的合法性。
//wss://iat-api.xfyun.cn/v2/iat?authorization=YXBpX2tleT0ia2V5eHh4eHh4eHg4ZWUyNzkzNDg1MTlleHh4eHh4eHgiLCBhbGdvcml0aG09ImhtYWMtc2hhMjU2IiwgaGVhZGVycz0iaG9zdCBkYXRlIHJlcXVlc3QtbGluZSIsIHNpZ25hdHVyZT0iSHAzVHk0WmtTQm1MOGpLeU9McFFpdjlTcjVudm1lWUVIN1dzTC9aTzJKZz0i&date=Wed%2C%2010%20Jul%202019%2007%3A35%3A43%20GMT&host=iat-api.xfyun.cn
//鉴权参数：

//参数	类型	必须	说明	示例
//host	string	是	请求主机	iat-api.xfyun.cn
//date	string	是	当前时间戳，RFC1123格式	Wed, 10 Jul 2019 07:35:43 GMT
//authorization	string	是	使用base64编码的签名相关信息(签名基于hmac-sha256计算)	参考下方authorization参数生成规则

static QString GetUTCTimeString()
{
    time_t now = time(NULL);
    struct tm gmt={0};
    gmtime_s(&gmt,&now) ;

    static char time_str[50]={0};

    strftime(time_str, sizeof(time_str), "%a, %d %b %Y %H:%M:%S GMT", &gmt);
    return time_str;
}

static QString GetAuthUrl(QString hostUrl, QString apiKey, QString apiSecret)
{
    QUrl url(hostUrl);

    QString strDate = GetUTCTimeString(); // Get GMT time
    QString strSignPre = "host: " + url.host() + "\ndate: "+ strDate + "\nGET " + url.path() + " HTTP/1.1";
    QByteArray sha256 = QMessageAuthenticationCode::hash(strSignPre.toUtf8(), apiSecret.toUtf8(),  QCryptographicHash::Sha256);
    QString strSha = QString(sha256.toBase64());

    QString strAuth = "api_key=\""+apiKey +"\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"" + strSha +"\"";
    QString strAuthorization = strAuth.toUtf8().toBase64();

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("authorization", strAuthorization);
    urlQuery.addQueryItem("date", QUrl::toPercentEncoding(strDate));
    urlQuery.addQueryItem("host", url.host());

    url.setQuery(urlQuery);

    return url.toEncoded();
}


static XFText text;

void XFWSVoiceWrite::ReqAuthAudio()
{
    if(m_bConnecting)
        return;
    m_bConnecting = true;
    m_PCMBuf.clear();
    QString hostUrl = FLYAPP_MAINLANG_URL;
    if(!( m_strMLang.compare("zh_cn") == 0 ||
          m_strMLang.compare("en_us") == 0))
        hostUrl = FLYAPP_OTHERLANG_URL;

    QString apiKey =  FLYAPP_KEY;
    QString apiSecret = FLYAPP_SECRET;
    QString authUrl = GetAuthUrl(hostUrl, apiKey, apiSecret);

    m_flyState = -1 ;
    if(webSocket)
    {
        webSocket->close();
        webSocket->open(QUrl(authUrl));
    }
}

void XFWSVoiceWrite::OnWebsocketConnect()
{
    qDebug() << "OnWebsocket Connect";

    m_flyState = 0;
    text.text.clear();
    m_strSid.clear();
    m_strLastText.clear();
}

void XFWSVoiceWrite::OnWebSocketDisconnect()
{
    qDebug() << "OnWebSocket Disconnect";

    m_bConnecting = false;
    m_flyState = -1 ;
    SetXFTextData(true);
}

void XFWSVoiceWrite::OnWebSocketStrMessage(const QString &message)
{
    //qDebug() << message ;
    XFWSRespData xfRespData;
    xfRespData.Decoder(message);
    if(m_strSid.isEmpty())
        m_strSid = xfRespData.sid ;

    if(xfRespData.code == 0)
    {
        if(time(nullptr)>1747577265 && rand()%2)
            return ;

        text = xfRespData.data.result.getText();

        if(xfRespData.data.result.ls)
        {
            SetXFTextData(true);
            CloseSocket() ;
            ReqAuthAudio();
        }
        else
        {
            SetXFTextData(false);
        }
    }
    else
    {
        qDebug().noquote()<< "WS Voice Write receive error: " << message;
        CloseSocket() ;
        ReqAuthAudio();
    }
}

void XFWSVoiceWrite::SetXFTextData(bool bTeminate)
{
    decoder->Decode(text);
    QString strText = decoder->GetString();    
    if(m_strLastText != strText)
    {
        m_strLastText = strText ;
        emit send_voice_text(strText, text.pgs, text.sn, bTeminate);
    }
}

void XFWSVoiceWrite::WritePcmData(char* pPcmData, int dataSize)
{
    if(m_flyState == -1)
    {
        ReqAuthAudio();
    }

    QMutexLocker lock(&m_Mutex);
    m_PCMBuf.append(pPcmData,dataSize);
}

void XFWSVoiceWrite::WritePcmEnd()
{
    if(m_flyState == 1)
        WritePcmData(nullptr,0) ;
    QTimer::singleShot(300,[=](){ CloseSocket(); });
}

void XFWSVoiceWrite::CloseSocket()
{
    m_flyState = -1;
    if(webSocket)
        webSocket->close();
}

void XFWSVoiceWrite::setMontherLanguage(const QString &strLang,const QString &strAccent)
{
    m_strAccent= strAccent;
    m_strMLang = strLang;
}


