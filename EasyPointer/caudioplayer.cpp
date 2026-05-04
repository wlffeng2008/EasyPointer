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
    QTimer::singleShot(500,this,[=]{
    });
    start();
}

void CAudioPlayer::pushBuf(const QByteArray&buf)
{
    QMutexLocker lock(&m_Mutex);
    m_ABufs.push_back(buf);
}

void CAudioPlayer::run()
{
    QAudioFormat format;
    format.setSampleRate(m_sampleRate);   // G.711通常使用8kHz采样率
    format.setChannelCount(m_channels);   // 单声道
    format.setSampleFormat(QAudioFormat::Int16); // 16位PCM

    QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();

    QAudioSink audioSink(audioDevice, format);

    QIODevice *pDevice = audioSink.start();
    audioSink.setVolume(0.99);
    audioSink.setBufferSize(32000);

    while(!m_bExit)
    {
        QMutexLocker lock(&m_Mutex);
        if(m_ABufs.isEmpty())
        {
            QCoreApplication::processEvents();
            continue;
        }

        QByteArray tmp = m_ABufs[0];
        m_ABufs.pop_front();

        pDevice->write(tmp);
        QThread::usleep(10);
    }
}
