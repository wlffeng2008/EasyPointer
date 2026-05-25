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

    void setAudioInfo(quint32 sampleRate,quint8 channels=1);
    void forceExit(){ m_bExit = true; m_bset = true; };
    void pushBuf(const QByteArray&buf);

signals:
    void onGetBuf(const QByteArray&buf);

protected:
    void run() override;

    quint32 m_sampleRate=16000;
    quint8  m_channels=1;
    QByteArrayList m_ABufs;
    QMutex m_Mutex;
    bool m_bExit=false;
    bool m_bset=false;
};

#endif // CAUDIOPLAYER_H
