#ifndef TXASRCLIENT_H
#define TXASRCLIENT_H

#include <QUrl>
#include <QWebSocket>
#include <QObject>
#include <QAudioSource>


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

signals:
    void onASRText(const QString&text,int state);

private:QByteArray m_buf;
    bool m_useMic = true;
    bool m_working = false;
    bool m_connect=false;
    QUrl  m_url;
    QWebSocket   *m_ws = nullptr;
    QIODevice    *m_audioDevice = nullptr;
    QAudioSource *m_audioSource = nullptr;
    void startCapture();
};

TxAsrClient *DoASRWork(bool toStart=true);

#endif // TXASRCLIENT_H
