#ifndef XFWSVOICEWRITE_H
#define XFWSVOICEWRITE_H

#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWebSocket>

class XFDataDecoder;
class XFWSVoiceWrite : public QThread
{
    Q_OBJECT

public:
    XFWSVoiceWrite();

public:
    void ReqAuthAudio();

    void WritePcmData(char* ptr, int size);
    void WritePcmEnd();
    void CloseSocket();

    void setMontherLanguage(const QString &strLang,const QString &strAccent);

signals:
    void send_voice_text(const QString &strText,const QString &strpgs, int nTextSN, bool bTeminate);
private slots:
    void OnWebsocketConnect();
    void OnWebSocketDisconnect();
    void OnWebSocketStrMessage(const QString &message);

private:
    void SetXFTextData(bool bTeminate); // bTeminate 当前websocket结束
    void run() override;

private:
    QWebSocket* webSocket = nullptr;
    XFDataDecoder* decoder = nullptr;
    qint32 m_flyState = 0;
    QString m_strFlyAppid;

    bool m_bConnecting = false;

    QString m_strMLang = "zh-cn";
    QString m_strAccent;
    QString m_strSid;
    QString m_strLastText;

    QMutex m_Mutex;
    QByteArray m_PCMBuf;
};

#endif // XFWSVOICEWRITE_H
