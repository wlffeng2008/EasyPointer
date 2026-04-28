#include "xftexttospeech.h"

#include <QUrlQuery>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// #define FLYAPP_ID      "a7b21c86"
// #define FLYAPP_KEY     "9994f6ce9dc54ca97cb9d02082ca3137"
// #define FLYAPP_SECRET  "NzljY2E3NzUxNTlhZmY1MTM5ZGY5MzM4"

#define FLYAPP_ID     "5e86ff8b"
#define FLYAPP_KEY    "d995a81f16617796fa82c5761ecea228"
#define FLYAPP_SECRET "NGM3OGYxMTcwN2E4ZGEyYTRlNzJhYWFj"



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


void XFTextToSpeech::ReqAuthAudio()
{
    QString hostUrl = "ws://tts-api.xfyun.cn/v2/tts";

    QString apiKey =  FLYAPP_KEY;
    QString apiSecret = FLYAPP_SECRET;
    QString authUrl = GetAuthUrl(hostUrl, apiKey, apiSecret);

    if(webSocket)
    {
        webSocket->close();
        webSocket->open(QUrl(authUrl));
    }
}

void XFTextToSpeech::sendText(const QString &strText)
{
    m_strBuf.clear() ;
    m_strText = strText;
    ReqAuthAudio() ;
}


XFTextToSpeech::XFTextToSpeech(QObject *parent)
    : QObject{parent}
{
    webSocket = new QWebSocket();
    QObject::connect(webSocket, &QWebSocket::connected, this, [this](){
        qDebug() << "OnWebsocketConnect" ;

        QJsonObject frame;

        QJsonObject common;
        common["app_id"] = FLYAPP_ID;

        QJsonObject business;
        business["aue"] = "lame";//raw
        business["tte"] = "UTF8";
        //business["ent"] = "intp65";
        business["vcn"] = "xiaoyan";
        business["pitch"] = 50;
        business["speed"] = 50;

        frame["common"] = common;
        frame["business"] = business;

        QJsonObject data;
        data["status"] = 2;
        QByteArray byteArray(m_strText.toUtf8());
        QByteArray base64Data = byteArray.toBase64();
        data["text"] = QString(base64Data);

        frame["data"] = data;

        QJsonDocument jsonDoc(frame);
        QString strMsg = jsonDoc.toJson();

        webSocket->sendTextMessage(strMsg);
    });

    QObject::connect(webSocket, &QWebSocket::disconnected, this, [](){
        qDebug() << "OnWebSocketDisconnect" ;
    });

    QObject::connect(webSocket, &QWebSocket::textMessageReceived, this, [this](const QString &message){
        //qDebug().noquote() << message ;
        QJsonObject jOB = QJsonDocument::fromJson(message.toUtf8()).object() ;
        QJsonObject jDT = jOB["data"].toObject();
        m_strBuf.append(jDT["audio"].toString()) ;
        if(jDT["status"].toInt() == 2)
        {
            qDebug() << "TTS Ending";
            QString strAudoFile=("D:\\abcde.mp3") ;
            QFile A(strAudoFile) ;
            if(A.open(QIODevice::WriteOnly))
            {
                QByteArray audio = QByteArray::fromBase64(m_strBuf.toLatin1());
                qDebug() << "PCM File Saving:" << strAudoFile;
                A.write(audio) ;
                A.close() ;
                emit audioFile(strAudoFile) ;

                //PCMFile2WAVFile("D:\\abcd.pcm","D:\\abcd.wav");
            }
            webSocket->close() ;
        }
    });

    QObject::connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), [](QAbstractSocket::SocketError error) {
        qDebug() << "WebSocket error:" << error;
    });

}
