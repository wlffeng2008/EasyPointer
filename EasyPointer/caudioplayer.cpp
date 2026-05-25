#include "caudioplayer.h"
#include "qdebug.h"
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSink>
#include <QBuffer>
#include <QCoreApplication>
#include <QIODevice>
#include <QTimer>

CAudioPlayer::CAudioPlayer(QObject *parent)
    : QThread{parent}
{
    start();

    connect(this,&CAudioPlayer::onGetBuf,this,[=](const QByteArray&buf){
        QMutexLocker lock(&m_Mutex);
        m_ABufs.push_back(buf);
    });
}

void CAudioPlayer::pushBuf(const QByteArray&buf)
{
    emit onGetBuf(buf);
}

void CAudioPlayer::setAudioInfo(quint32 sampleRate,quint8 channels)
{
    m_sampleRate=sampleRate;
    m_channels=channels;
    m_bset = true;
}

void CAudioPlayer::run()
{
    m_bExit = false;
    while(!m_bExit)
    {
        if(m_bExit) break;

        QAudioFormat format;
        format.setSampleRate(m_sampleRate);
        format.setChannelCount(m_channels);
        format.setSampleFormat(QAudioFormat::Int16); // 16位PCM

        QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();

        QAudioSink audioSink(audioDevice, format);

        QIODevice *pDevice = audioSink.start();
        audioSink.setVolume(0.99);
        audioSink.setBufferSize(32000);
        m_bset = false;
        while(true)
        {
            if(m_bset)
            {
                qDebug() << "Set QAudioFormat";
                break;
            }

            if(m_ABufs.isEmpty())
            {
                QCoreApplication::processEvents();
                QThread::usleep(10);
                continue;
            }

            QMutexLocker lock(&m_Mutex);
            QByteArray data = m_ABufs.takeFirst();
            pDevice->write(data);
            pDevice->waitForBytesWritten(100);
        }
    }
}
