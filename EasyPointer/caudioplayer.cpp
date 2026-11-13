#include "caudioplayer.h"
#include "qdebug.h"


#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    #include <QAudioFormat>
    #include <QAudioDevice>
    #include <QMediaDevices>
    #include <QAudioSink>

#else

    #include <QAudioOutput>
    #include <QAudioDeviceInfo>

#endif

#include <QIODevice>
#include <QBuffer>
#include <QCoreApplication>
#include <QTimer>

CAudioPlayer::CAudioPlayer(QObject *parent)
    : QThread{parent}
{
    start();

    connect(this,&CAudioPlayer::onGetBuf,this,[=](const QByteArray&buf){
        QMutexLocker lock(&m_Mutex);
        if(!buf.isEmpty()) m_ABufs.push_back(buf);
    });
}

void CAudioPlayer::pushBuf(const QByteArray&buf)
{
    emit onGetBuf(buf);
}

void CAudioPlayer::setAudioInfo(quint32 sampleRate,quint8 channels)
{
    m_sampleRate = sampleRate;
    m_channels = channels;
    m_bset = true;
}

void CAudioPlayer::run()
{
    m_bExit = false;

    while(true)
    {
        if(m_bExit) break;


#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

        QAudioFormat format;
        format.setSampleRate(m_sampleRate);
        format.setChannelCount(m_channels);
        format.setSampleFormat(QAudioFormat::Int16); // 16位PCM

        QAudioDevice audioOutput = QMediaDevices::defaultAudioOutput();
        qDebug() << audioOutput.description();
        QAudioSink audioSink(audioOutput, format);
        QIODevice *pDevice = audioSink.start();
        if(!pDevice)
        {
            qCritical() << "无法播放声音!" ;
            return;
        }

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
                QThread::msleep(1);
                continue;
            }
            QByteArray data;
            {
                QMutexLocker lock(&m_Mutex);
                data = m_ABufs.takeFirst();
            }
            pDevice->write(data);
            pDevice->waitForBytesWritten(100);
        }

        if(pDevice) pDevice->close();

        audioSink.stop();

#else
        // ---------- 设备与格式 ----------
        QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
        if (info.isNull()) {
            qCritical() << QStringLiteral("无可用音频输出设备");
            return;
        }
        qDebug() << QStringLiteral("Audio output:") << info.deviceName();

        QAudioFormat format;
        format.setSampleRate(m_sampleRate);
        format.setChannelCount(m_channels);
        format.setSampleSize(16);                            // Qt5：位深单独设，不是 setSampleFormat
        format.setCodec(QStringLiteral("audio/pcm"));       // Qt5 必填，Qt6 已移除
        format.setByteOrder(QAudioFormat::LittleEndian);    // x86 / WASAPI
        format.setSampleType(QAudioFormat::SignedInt);      // 16bit 必须有符号！UnSignedInt 会无声

        QAudioOutput audioOutput(info, format);
        audioOutput.setBufferSize(32000);       // 必须在 start() 之前；之后调用会被直接忽略

        QIODevice *pDevice = audioOutput.start();
        audioOutput.setVolume(0.99);

        while (true)
        {
            if(m_bset)
            {
                qDebug() << "Set QAudioFormat";
                break;
            }

            if(m_ABufs.isEmpty())
            {
                QThread::msleep(1);
                continue;
            }

            QByteArray data;
            {
                QMutexLocker locker(&m_Mutex);
                data = m_ABufs.takeFirst();
            }

            // 背压：按 bytesFree() 分批写，处理部分写入
            while (!data.isEmpty()) {
                const QAudio::State st = audioOutput.state();

                if (st == QAudio::SuspendedState || st == QAudio::InterruptedState) {
                    audioOutput.resume();
                    QThread::msleep(1);
                    continue;
                }

                const int freeBytes = audioOutput.bytesFree();   // Qt5 返回 int
                if (freeBytes <= 0) { QThread::msleep(1); continue; }

                const qint64 n = qMin<qint64>(data.size(), freeBytes);
                const qint64 written = pDevice->write(data.constData(), n);
                if (written <= 0) break;
                data.remove(0, static_cast<int>(written));
            }
        }

        if(pDevice) pDevice->close();

        audioOutput.stop();     // 之后 pDevice 立即失效：不要 close()、不要 delete、不要再碰

#endif

    }
}
