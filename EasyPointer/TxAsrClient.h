#ifndef TXASRCLIENT_H
#define TXASRCLIENT_H

#include <QUrl>
#include <QWebSocket>
#include <QObject>
#include <QAudioSource>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class HttpHandler : public QObject
{
    Q_OBJECT
public:
    explicit HttpHandler(QObject *parent = nullptr) : QObject(parent)
    {
        manager = new QNetworkAccessManager(this);
    }

    void get(const QString &url)
    {
        QNetworkRequest request((QUrl(url)));
        QNetworkReply *reply = manager->get(request);
        connect(reply, &QNetworkReply::finished, this, [=]{
            handleReply(reply);
        });
    }

    void post(const QString &url, const QByteArray &data)
    {
        QNetworkRequest request((QUrl(url)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
        QNetworkReply *reply = manager->post(request, data);
        connect(reply, &QNetworkReply::finished, this, [=]{
            handleReply(reply);
        });
    }

    void post(const QNetworkRequest &req, const QByteArray &data)
    {
        QNetworkReply *reply = manager->post(req, data);
        connect(reply, &QNetworkReply::finished, this, [=]{
            handleReply(reply);
        });
    }

private:
    void handleReply(QNetworkReply *reply)
    {
        if(reply->error() != QNetworkReply::NoError)
        {
            QString error = reply->errorString();
            qDebug() << "HttpHandler Error:" << error;
        }
        else
        {
            int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString text = reply->readAll();
            emit onHttpReturn(text,code);
        }
        reply->deleteLater();
    }

signals:
    void onHttpReturn(const QString&text,int code);
    void onHttpWrror(const QString&error);

private:
    QNetworkAccessManager *manager=nullptr;
};



class TxAsrClient : public QObject
{
    Q_OBJECT
public:
    TxAsrClient(const QUrl& url,bool bUseMic=true, QObject* parent = nullptr);
    ~TxAsrClient();
    void stop();
    void start();
    void userMic(bool use = true);
    void writePCM(const QByteArray&data);
    void translateText(const QString &text,const QString &from="zh",const QString &to="en");

signals:
    void onASRText(const QString&text,int state);
    void onTransText(const QString&text,int state);
    void onASRConnect(bool connect);
    void onMicPcmData(const QByteArray&pcm);

private:QByteArray m_buf;
    bool m_useMic = true;
    bool m_working = false;
    bool m_connect=false;
    QUrl  m_url;
    QWebSocket   *m_ws = nullptr;
    QIODevice    *m_audioDevice = nullptr;
    QAudioSource *m_audioSource = nullptr;
    void startCapture();
    QString m_strText;
    QTimer *m_pTMPush = nullptr;

    HttpHandler *m_http = nullptr;
};

TxAsrClient *DoASRWork(bool toStart=true);

#endif // TXASRCLIENT_H
