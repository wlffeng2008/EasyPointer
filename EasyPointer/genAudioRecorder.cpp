#include "genAudioRecorder.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    #include <QAudioFormat>
    #include <QAudioDevice>
    #include <QMediaDevices>
    #include <QAudioInput>
    #include <QAudioSource>
    #include <QAudioSink>
    #include <QIODevice>

#else

    #include <QAudioDeviceInfo>

#endif


#include "lame.h"
#pragma comment(lib,"libmp3lame-static.lib")
#pragma comment(lib,"libmpghip-static.lib")

class WaveUntils
{
public:
    static bool WavToMp3(const QString& wavPath, const QString& mp3Path)
    {
        QFile wavFile(wavPath);
        if (!wavFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "打开 WAV 文件失败：" << wavPath;
            return false;
        }

        QFile mp3File(mp3Path);
        if (!mp3File.open(QIODevice::WriteOnly))
        {
            qDebug() << "创建 MP3 文件失败：" << mp3Path;
            wavFile.close();
            return false;
        }

        char wavHeader[44] = {0};
        if (wavFile.read(wavHeader, 44) != 44)
        {
            qDebug() << "WAV 文件格式错误！";
            return false;
        }

        int sampleRate = ( unsigned char)wavHeader[24] |
                         ((unsigned char)wavHeader[25] << 8) |
                         ((unsigned char)wavHeader[26] << 16) |
                         ((unsigned char)wavHeader[27] << 24);

        short channels = (unsigned char)wavHeader[22] | ((unsigned char)wavHeader[23] << 8);
        short bits     = (unsigned char)wavHeader[34] | ((unsigned char)wavHeader[35] << 8);

        qDebug() << "WAV信息：采样率=" << sampleRate << " 通道数=" << channels << " 位深=" << bits;

        lame_global_flags* lame = lame_init();
        if (!lame)
        {
            qDebug() << "LAME 初始化失败！";
            return false;
        }

        lame_set_in_samplerate(lame, sampleRate);    // 输入采样率
        lame_set_num_channels(lame, channels);       // 通道数
        lame_set_mode(lame, channels == 1 ? MONO : STEREO); // 单声道/立体声
        lame_set_brate(lame, 192);                  // MP3 码率（128/192/320）
        lame_set_quality(lame, 2);                   // 质量 0~9，2=高品质

        if (lame_init_params(lame) < 0)
        {
            qDebug() << "LAME 参数设置失败！";
            lame_close(lame);
            return false;
        }

        const int BUFFER_SIZE = 8192;
        char readBuffer[BUFFER_SIZE];
        qint64 readLen=0;

        QDateTime startTime = QDateTime::currentDateTime();

        while ((readLen = wavFile.read(readBuffer, BUFFER_SIZE)) > 0)
        {
            // 16bit WAV 数据转成 LAME 需要的格式
            short* pcmData = (short*)readBuffer;
            int pcmSamples = readLen / 2;

            unsigned char mp3Buffer[BUFFER_SIZE * 5 / 4 + 7200];
            int encodedLen;

            if (channels == 1)
            {
                encodedLen = lame_encode_buffer(lame, pcmData, nullptr, pcmSamples, mp3Buffer, sizeof(mp3Buffer));
            }
            else
            {
                encodedLen = lame_encode_buffer_interleaved(lame, pcmData, pcmSamples / 2, mp3Buffer, sizeof(mp3Buffer));
            }

            if (encodedLen > 0) {
                mp3File.write((char*)mp3Buffer, encodedLen);
            }
        }

        unsigned char finalBuffer[7200];
        int finalLen = lame_encode_flush(lame, finalBuffer, sizeof(finalBuffer));
        if (finalLen > 0) {
            mp3File.write((char*)finalBuffer, finalLen);
        }

        lame_close(lame);
        wavFile.close();
        mp3File.close();

        qDebug() << "转换完成！耗时：" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
        return true;
    }

    static bool Pcm2Wav(const QString&strPCMFile,const QString&strWAVFile)
    {
        struct WAVFILEHEADER
        {
            char RiffName[4];
            unsigned long nRiffLength;
            char WavName[4];
            char FmtName[4];
            unsigned long nFmtLength;
            unsigned short nAudioFormat;
            unsigned short nChannleNumber;
            unsigned long nSampleRate;
            unsigned long nBytesPerSecond;
            unsigned short nBytesPerSample;
            unsigned short nBitsPerSample;
            char    DATANAME[4];
            unsigned long   nDataLength;
        };

        WAVFILEHEADER WavFileHeader;
        qstrcpy(WavFileHeader.RiffName,"RIFF");
        qstrcpy(WavFileHeader.WavName, "WAVE");
        qstrcpy(WavFileHeader.FmtName, "fmt ");
        qstrcpy(WavFileHeader.DATANAME,"data");

        WavFileHeader.nFmtLength = 16;
        WavFileHeader.nAudioFormat = 1;
        WavFileHeader.nChannleNumber = 1;
        WavFileHeader.nSampleRate = 16000;
        WavFileHeader.nBytesPerSample = 2;
        WavFileHeader.nBytesPerSecond = 32000;
        WavFileHeader.nBitsPerSample = 16;

        QFile pcmFile(strPCMFile);
        QFile wavFile(strWAVFile);
        if (!pcmFile.open(QIODevice::ReadOnly))
            return false;
        if (!wavFile.open(QIODevice::WriteOnly))
            return false;

        int nSize = sizeof(WavFileHeader);
        qint64 nFileLen = pcmFile.bytesAvailable();
        WavFileHeader.nRiffLength = static_cast<unsigned long>(nFileLen - 8 + nSize);
        WavFileHeader.nDataLength = static_cast<unsigned long>(nFileLen);

        wavFile.write((const char *)&WavFileHeader,nSize);
        wavFile.write(pcmFile.readAll());

        pcmFile.close();
        wavFile.close();
        return true;
    }
};

bool WAVFile2MP3File(const QString&strWAVFile,const QString&strMP3File)
{
    return WaveUntils::WavToMp3(strWAVFile,strMP3File);
}

bool PCMFile2WAVFile(const QString&strPCMFile,const QString&strWAVFile)
{
    return WaveUntils::Pcm2Wav(strPCMFile,strWAVFile);
}



genAudioRecorder::genAudioRecorder(QObject *parent)
    : QObject{parent}
{

}

void genAudioRecorder::startRecord()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    QAudioFormat fmt = inputDevice.preferredFormat();
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);
    m_audioSource = new QAudioSource(inputDevice,fmt);
    m_audioSource->setBufferSize(1280);  // 建议每 40ms 读一次 → 16000 * 2 * 0.04 = 1280 bytes

    m_audioDevice = m_audioSource->start();

#else

    const QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (info.isNull()) {
        qCritical() << QStringLiteral("无可用音频输入设备");
        return;
    }
    qDebug() << QStringLiteral("Audio input:") << info.deviceName();

    QAudioFormat fmt;
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
    fmt.setSampleSize(16);                            // Qt5：位深单独设，不是 setSampleFormat
    fmt.setCodec(QStringLiteral("audio/pcm"));       // Qt5 必填，Qt6 已移除
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::SignedInt);      // 16bit 必须有符号，否则开流失败/无声

    m_audioSource = new QAudioInput(info, fmt, this);
    m_audioSource->setBufferSize(1280);   // ★ 必须在 start() 之前，之后调用被忽略
    m_audioSource->setNotifyInterval(40);         // 备用心跳：部分后端 readyRead 不发/很稀疏

    m_audioDevice = m_audioSource->start();      // push 模式：返回内部 QIODevice，已 open

#endif

    connect(m_audioDevice, &QIODevice::readyRead, this, [=]{
        while (m_audioDevice->bytesAvailable() >= 1280)
        {
            QByteArray pcm = m_audioDevice->read(1280);
            emit audioDataIn(pcm);
            qDebug() << pcm.size();
        }
    });
}

void genAudioRecorder::stopRecord()
{
    if(m_audioDevice) m_audioDevice->close();
    if(m_audioSource) m_audioSource->stop();
    m_audioSource = nullptr;
    m_audioDevice = nullptr;
}
