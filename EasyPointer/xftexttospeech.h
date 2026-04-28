#ifndef XFTEXTTOSPEECH_H
#define XFTEXTTOSPEECH_H

#include <QObject>
#include <QWebSocket>

class XFTextToSpeech : public QObject
{
    Q_OBJECT
public:
    explicit XFTextToSpeech(QObject *parent = nullptr);

    void sendText(const QString &strText) ;

signals:
    void audioFile(const QString &strFile) ;

private:
    QWebSocket *webSocket = nullptr;
    QString m_strText ;
    QString m_strBuf ;
    void ReqAuthAudio() ;
};

#endif // XFTEXTTOSPEECH_H
