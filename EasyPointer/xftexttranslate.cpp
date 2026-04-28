#include "xftexttranslate.h"

#include <QString>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtNetwork/QNetworkReply>

// #define FLYAPP_ID     "a7b21c86"
// #define FLYAPP_KEY    "9994f6ce9dc54ca97cb9d02082ca3137"
// #define FLYAPP_SECRET "NzljY2E3NzUxNTlhZmY1MTM5ZGY5MzM4"

#define FLYAPP_ID     "5e86ff8b"
#define FLYAPP_KEY    "d995a81f16617796fa82c5761ecea228"
#define FLYAPP_SECRET "NGM3OGYxMTcwN2E4ZGEyYTRlNzJhYWFj"


#define FLYAPP_HOST   "itrans.xf-yun.com"
#define FLYAPP_PATH   "/v1/its"

//#define FLYAPP_HOST    "iat.cn-huabei-1.xf-yun.com"
//#define FLYAPP_PATH    "/v1/iat"

void XFTransData::Decode(QJsonObject dataObj)
{
    QJsonObject resultObj = dataObj["result"].toObject();
    from = resultObj["from"].toString();
    to = resultObj["to"].toString();
    QJsonObject transObj = resultObj["trans_result"].toObject();
    dst = transObj["dst"].toString();
    src = transObj["src"].toString();
}

void XFTransRespData::Decode(QString str)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(str.toUtf8());
    if (jsonDocument.isNull()) {
        qDebug() << "Invalid json string" << str;
        return;
    }

    if (jsonDocument.isObject())
    {
        QJsonObject respObject = jsonDocument.object();
        sid = respObject.value(QStringLiteral("sid")).toString();
        message = respObject.value(QStringLiteral("message")).toString();
        code = respObject.value(QStringLiteral("code")).toInt();
        QJsonObject dataObj = respObject["data"].toObject();
        data.Decode(dataObj);
    }
}

void XFTransNewData::Decode(QJsonObject dataObj)
{
    QJsonObject resultObj = dataObj["result"].toObject();
    seq = resultObj["seq"].toInt();
    status = resultObj["status"].toInt();
    QString text = resultObj["text"].toString();
    QByteArray base64Data = QByteArray::fromBase64(text.toUtf8());
    QString decodedString = QString::fromUtf8(base64Data);

    QJsonDocument jsonDocument = QJsonDocument::fromJson(base64Data);
    QJsonObject textObj = jsonDocument.object();
    QJsonObject transObj = textObj["trans_result"].toObject();
    from = textObj["from"].toString();
    to = textObj["to"].toString();
    dst = transObj["dst"].toString();
    src = transObj["src"].toString();
}

void XFTransNewRespData::Decode(QString str)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(str.toUtf8());
    if (jsonDocument.isNull()) {
        qDebug() << "Invalid json string" << str;
        return;
    }
    if (jsonDocument.isObject())
    {
        QJsonObject respObject = jsonDocument.object();
        QJsonObject headerObj = respObject["header"].toObject();
        sid = headerObj.value(QStringLiteral("sid")).toString();
        message = headerObj.value(QStringLiteral("message")).toString();
        code = headerObj.value(QStringLiteral("code")).toInt();
        QJsonObject payload = respObject["payload"].toObject();
        data.Decode(payload);
    }

}

FlyTextTranslate::FlyTextTranslate(QObject *parent) : QObject(parent)
{

}

static QString GetUTCTimeString()
{
    time_t now = time(NULL);
    struct tm *gmTime = gmtime(&now);
    static char time_str[50];
    // 使用strftime函数格式化时间
    strftime(time_str, sizeof(time_str), "%a, %d %b %Y %H:%M:%S GMT", gmTime);
    return time_str;
}

QString FlyTextTranslate::getUrl()
{
    return "http://itrans.xf-yun.com/v1/its";
}

QString FlyTextTranslate::getBody(QString text, QString from, QString to)
{
    QString content = text.toUtf8().toBase64();

    QJsonObject jsonObject;

    QJsonObject headObj;
    headObj["app_id"] = FLYAPP_ID;
    headObj["status"] = 3;
    //headObj["res_id"] = "";
    jsonObject["header"] = headObj;

    QJsonObject parameterObj;
    QJsonObject itsObj;
    itsObj["from"] = from;
    itsObj["to"] = to;
    itsObj["result"] = QJsonObject();
    parameterObj["its"] = itsObj;
    jsonObject["parameter"] = parameterObj;

    QJsonObject input_data;
    input_data["encoding"] = "utf8";
    input_data["status"] = 3;
    QByteArray textBytes = text.toUtf8();
    QByteArray encodedText = textBytes.toBase64();
    input_data["text"] = QString(encodedText);
    QJsonObject payload;
    payload["input_data"] = input_data;

    jsonObject["payload"] = payload;

    QJsonDocument jsonDoc(jsonObject);
    QString strBody = jsonDoc.toJson(QJsonDocument::Compact);
    return strBody;
}


QString FlyTextTranslate::assembleURL(QString requset_url, QString data)
{
    QString headersha = generateSignature("");
    QString apiKey = FLYAPP_KEY;
    QString authorization_origin = QString("api_key=\"%1\", algorithm=\"%2\", headers=\"host date request-line\", signature=\"%3\"")
                .arg(apiKey)
                .arg("hmac-sha256")
                .arg(headersha);

    QByteArray encodedData = authorization_origin.toUtf8().toBase64();
    QString authorization = encodedData;

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("host", FLYAPP_HOST);
    urlQuery.addQueryItem("date", QUrl::toPercentEncoding(strDate));
    urlQuery.addQueryItem("authorization", authorization);

    QUrl url = QUrl(requset_url);
    url.setQuery(urlQuery);
    return url.toEncoded();
}

void FlyTextTranslate::initNewHeader(QNetworkRequest& request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("content-type", "application/json");
    request.setRawHeader("host", FLYAPP_HOST);
    request.setRawHeader("app_id", strDate.toUtf8());
}

QString FlyTextTranslate::generateSignature(QString digest)
{
    QString httpMethod = "POST";
    QString requestUri = FLYAPP_PATH;
    QString httpProto = "HTTP/1.1";

    QString signatureStr = QString("host: ") + FLYAPP_HOST + "\n";
    signatureStr += "date: " + strDate + "\n";
    signatureStr += httpMethod + " " + requestUri + " " + httpProto ;

    QString apiSecret = FLYAPP_SECRET;
    QByteArray originalData = signatureStr.toUtf8();
    QByteArray hmacSha256 = QMessageAuthenticationCode::hash(originalData, apiSecret.toUtf8(),  QCryptographicHash::Sha256);
    QByteArray encodedData = hmacSha256.toBase64();
    QString sha = QString::fromUtf8(encodedData);

    return sha;
}


void FlyTextTranslate::translateText(const QString &text,const QString &src, const QString &dst)
{
    if(src == dst)
    {
        emit finish_translate(text);
        return ;
    }
    m_src = src;
    m_dst = dst;

    strDate = GetUTCTimeString();

    if(!g_manager)
        g_manager = new QNetworkAccessManager();

    QString mid = dst;
    if(src  != "cn" && dst != "cn")
        mid="cn";

    QNetworkRequest request;
    QString request_url = getUrl();
    QString strUrl = assembleURL(request_url, text);
    request.setUrl(QUrl(strUrl));
    //qDebug()<<"translate url" << strUrl;
    QString body = getBody(text, src, mid);
    initNewHeader(request);

    QByteArray postData = body.toUtf8().data();
    //qDebug()<<"translate body " << body;
    QNetworkReply* reply = g_manager->post(request, postData);

    QObject::connect(reply, &QNetworkReply::finished, this, [=](){
         if(reply->error() == QNetworkReply::NoError){
             QByteArray response = reply->readAll();
             processResponse(response);
         }else {
             qDebug()<<"net error" << reply->error();
         }

         reply->deleteLater(); // 释放reply资源
    });
}

void FlyTextTranslate::onRecvRsp()
{
    if(g_reply->error() == QNetworkReply::NoError){
        QByteArray response = g_reply->readAll();

        XFTransNewRespData resp;
        resp.Decode(response);
        QString sendStr = resp.getText();
        emit finish_translate(sendStr);
    }

    g_reply->deleteLater();
}

void FlyTextTranslate::processResponse(QString response)
{
    XFTransNewRespData resp;
    resp.Decode(response);
    QString sendStr = resp.getText();
    emit finish_translate(sendStr);
}

