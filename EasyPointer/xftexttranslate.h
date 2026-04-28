#ifndef XFTEXTTRANSLATE_H
#define XFTEXTTRANSLATE_H

#include <cstdint>
#include <QString>
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QNetworkAccessManager>

class XFTransData{
public:
    QString from;
    QString to;
    QString dst;
    QString src;

    void Decode(QJsonObject dataObj);
};

class XFTransRespData
{
public:
    QString sid;
    QString message;
    int code;
    XFTransData data;

    void Decode(QString strMsg);
    QString getText(){return data.dst;}
    QString getSrc(){return data.src;}
};

// 机器翻译新
class XFTransNewData{
public:
    int seq;
    int status;
    QString from;
    QString to;
    QString dst;
    QString src;

    void Decode(QJsonObject dataObj);
};

class XFTransNewRespData
{
public:
    QString sid;
    QString message;
    int code;
    XFTransNewData data;

    void Decode(QString strMsg);
    QString getText(){return data.dst;}
    QString getSrc(){return data.src;}
};

class FlyTextTranslate : public QObject
{
    Q_OBJECT

public:
    explicit FlyTextTranslate(QObject *parent = nullptr);

    void translateText(const QString &text,const QString &src, const QString &dst);

private:
    QString getBody(QString text, QString from, QString to);
    QString assembleURL(QString url, QString data);
    void initNewHeader(QNetworkRequest& request);
    QString getUrl();

    QString generateSignature(QString value);
    void processResponse(QString response);

private slots:
    void onRecvRsp();

signals:
    void finish_translate(const QString &strText);

private:

    QString m_src;
    QString m_dst;

    QString strDate;

    QNetworkAccessManager* g_manager = nullptr;
    QNetworkReply *g_reply = nullptr;
};

#endif // XFTEXTTRANSLATE_H
