#ifndef CAUDIOPLAYER_H
#define CAUDIOPLAYER_H

#include <QObject>
#include <QThread>

#include <QMutex>
#include <QByteArrayList>

class CAudioPlayer : public QThread
{
    Q_OBJECT
public:
    explicit CAudioPlayer(QObject *parent = nullptr);
    virtual ~CAudioPlayer(){ m_bExit = true; };

    void setAudioInfo(quint32 sampleRate,quint8 channels){m_sampleRate=sampleRate; m_channels=channels;};
    void forceExit(){ m_bExit = true; };
    void pushBuf(const QByteArray&buf);

protected:
    void run() override;

    quint32 m_sampleRate=16000;
    quint8  m_channels=1;
    QByteArrayList m_ABufs;
    QMutex m_Mutex;
    bool m_bExit = false;
};

#endif // CAUDIOPLAYER_H
