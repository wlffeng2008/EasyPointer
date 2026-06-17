
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QMutexLocker>
#include <QDir>

#include "hidapi.h"
#include "hidworker.h"
#include "lame.h"
#include "qjsonobject.h"
#pragma comment(lib,"libmp3lame.lib")

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


namespace MicLink {

unsigned char BKEY[100]={
    0x31, 0xec, 0x06, 0xa2, 0x0c, 0x90, 0x12, 0x9d, 0x20, 0xa3,
    0x6b, 0x41, 0xbb, 0x40, 0x68, 0x5a, 0x4b, 0x12, 0xf4, 0x11,
    0x03, 0x9a, 0x4b, 0x12, 0xf6, 0x11, 0x03, 0x76, 0x0a, 0x12,
    0xd4, 0x48, 0x10, 0x49, 0x20, 0xf2, 0x20, 0x03, 0x54, 0x49,
    0x24, 0xf4, 0x20, 0x03, 0x92, 0x12, 0xf6, 0x02, 0x03, 0x20,
    0xa4, 0x91, 0x02, 0xdb, 0xc1, 0xda, 0x49, 0x19, 0x4a, 0x09,
    0x6e, 0xed, 0x6b, 0x49, 0xf8, 0xab, 0x94, 0xc1, 0x77, 0x48,
    0x01, 0xaf, 0x28, 0xc0, 0x2a, 0x39, 0x9f, 0x08, 0x90, 0x01,
    0x9e, 0x01, 0xa0, 0x09, 0x90, 0x74, 0x9a, 0x81, 0x87, 0x2b,
    0x4d, 0xab, 0x20, 0xc0, 0x75, 0x87, 0xbe, 0xec, 0x30, 0xec};

void encrypt(unsigned char *indata, unsigned char *outdata, unsigned int len)
{
    for (int i = 0; i < len; i++) {
        outdata[i] = indata[i] ^ BKEY[i];
    }
}

static const signed char idxtbl[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
static const unsigned short steptbl[] = {
    7,  8,  9,  10,  11,  12,  13,  14,  16,  17,
    19,  21,  23,  25,  28,  31,  34,  37,  41,  45,
    50,  55,  60,  66,  73,  80,  88,  97,  107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767   };


#define	 NUM_OF_ORIG_SAMPLE	2

void adpcm_to_pcm (signed short *ps, signed short *pd, int len)
{
    int i;

    //byte2,byte1: predict;  byte3: predict_idx; byte4:adpcm data len
    int predict = ps[0];
    int predict_idx = ps[1] & 0xff;
    //	int adpcm_len = (ps[1]>>8) & 0xff;

    unsigned char *pcode = (unsigned char *) (ps + NUM_OF_ORIG_SAMPLE);

    unsigned char code;
    code = *pcode ++;

    //byte5- byte128: 124 byte(62 sample) adpcm data
    for (i=0; i<len; i++)
    {
        int step = steptbl[predict_idx];

        int diffq = step >> 3;

        if (code & 4) {
            diffq = diffq + step;
        }
        step = step >> 1;
        if (code & 2) {
            diffq = diffq + step;
        }
        step = step >> 1;
        if (code & 1) {
            diffq = diffq + step;
        }

        if (code & 8) {
            predict = predict - diffq;
        }
        else {
            predict = predict + diffq;
        }

        if (predict > 32767) {
            predict = 32767;
        }
        else if (predict < -32768) {
            predict = -32768;
        }

        predict_idx = predict_idx + idxtbl[code & 15];

        if(predict_idx < 0) {
            predict_idx = 0;
        }
        else if(predict_idx > 88) {
            predict_idx = 88;
        }

        if (i&1) {
            code = *pcode ++;
        }
        else {
            code = code >> 4;
        }

        *pd++ = predict;
    }
}

}

static QString g_strWork;

CHidWorker::CHidWorker()
{
    m_pDev = nullptr;
    QTimer::singleShot(200,this,[=]{
        m_pAPlayer = new CAudioPlayer(this);
        start();
    });
    QTimer *tmReadSN = new QTimer();
    //tmReadSN->start(5000);
    connect(tmReadSN,&QTimer::timeout,this,[=]{
        if(!m_bRecord)
        {
            readSN();
            QThread::msleep(20);
            askBattery();
        }
    });

    g_strWork = QApplication::applicationDirPath() + "/recordfile";
    QDir D(g_strWork);
    if(!D.exists()) D.mkdir(g_strWork);
}

CHidWorker::~CHidWorker()
{
    m_pAPlayer->forceExit();
    m_bEndWork = true;
    close();
}

void CHidWorker::close()
{
    if(m_pDev)
        hid_close(m_pDev);
    m_pDev = nullptr;
}

void CHidWorker::setHidVIDPID(quint16 VID, quint16 PID)
{
    m_VID = VID;
    m_PID = PID;
}

void CHidWorker::run()
{
    while (!m_bEndWork)
    {
        int mode = 1;
        hid_device_info *pEDev = nullptr;
        while (!m_bEndWork)
        {
            pEDev = hid_enumerate(m_VID,m_PID);
            if(pEDev)
            {
                mode = 1;
                qDebug() << "hid_enumerate 2.4G" << Qt::hex << m_VID << m_PID;
                break;
            }

            pEDev = hid_enumerate(m_VID,0x61AB);
            if(pEDev)
            {
                mode = 2;
                qDebug() << "hid_enumerate BLE" << Qt::hex << m_VID << 0x61AB;
                break;
            }

            QThread::msleep(500);
        }

        hid_device_info *pTDev = pEDev; // trace
        while(pTDev)
        {
            //qDebug() << pTDev->path << Qt::hex << pTDev->usage_page << pTDev->usage;
            pTDev = pTDev->next;
        }
        pTDev = pEDev;

        hid_device *pDev = nullptr; // to open
        while(pTDev)
        {
            if(pTDev->usage_page == 0xFF91)
            {
                pDev = hid_open_path(pTDev->path);
                if(pDev)
                {
                    m_pDev = pDev;
                    m_strDevPath = pTDev->path;
                    qDebug() << "Open HID:"<< pTDev->path;
                    break;
                }
            }

            pTDev = pTDev->next;
        }
        hid_free_enumeration(pEDev);

        if(!m_pDev) continue;

        QThread::msleep(20);
        m_pAPlayer->setAudioInfo(16000/mode);

        emit onConnect(mode,true);

        readSN();
        QThread::msleep(20);
        setMouse(false);
        QThread::msleep(20);
        setLaser(false);
        QThread::msleep(20);
        stopRecord();
        QThread::msleep(20);
        setTime();
        QThread::msleep(20);
        askBattery();

        quint8 szBufs[200][1024] = {{0}};

        int nUesed = 0;
        int nRead = 33;
        if(mode == 2)
            nRead = 1001;
        while(true)
        {
            quint8 *szBuf = szBufs[nUesed++ % 200];
            int nRet = hid_read_timeout(pDev,szBuf,nRead,0xFFFF);

            if(nRet <  0) break;
            if(nRet == 0) continue;

            if(szBuf[0] == 0x1b)
            {
                quint8 eBuf[1024] = {0};
                MicLink::encrypt(szBuf + 1, eBuf, nRet-1);

                int count = (nRet - 1 - 4) * 2;
                short aBuf[1024] = {0};
                MicLink::adpcm_to_pcm((short *)(eBuf), aBuf, count*2);

                QByteArray data((char *)aBuf, count*2);
                WritePCMData(data);

                if(m_bOutPlay)
                {
                    m_pAPlayer->pushBuf(data);
                }
                else
                {
                    emit onPCMData(data);
                }
            }
            else
            {
                //QByteArray Log((const char *)(szBuf),nRet);
                //qDebug().noquote()<<"USB <=: "<< Log.toHex(' ') << "Len: "<< nRet;
                emit onDataIn(szBuf,32);
            }
        }

        hid_close(pDev);
        m_pDev = nullptr;

        emit onConnect(mode,false);
    }
}

void CHidWorker::sendCmd(quint8 *pCmd)
{
    if(!m_pDev)
        return;

    //QByteArray Log((char *)pCmd,32);
    //qDebug().noquote()<<"USB =>: "<< Log.toHex(' ');
    hid_write(m_pDev,pCmd,32);
}

void CHidWorker::hearBeat()
{
    quint8 szCmd[33]={0x0C,0x4D,0x05,0x10,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::readSN()
{
    quint8 szCmd[33]={0x0C,0x4D,0x01,0x61,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setMouse(bool on)
{
    quint8 nSet = on ? 0xB0 : 0xB1;
    quint8 szCmd[33]={0x0C,0x4D,0x05,nSet,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setLaser(bool on)
{
    quint8 nSet = on ? 0xB4 : 0xB5;
    quint8 szCmd[33]={0x0C,0x4D,0x05,nSet,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setDPI(quint8 index) //1-3
{
    quint8 szCmd[33]={0x0C,0x4D,0x05,0x68,index,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setURL(bool on)
{
    quint8 nSet = on ? 0x66 : 0x67;
    quint8 szCmd[33]={0x0C,0x4D,0x05,nSet,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setSample(quint8 index)
{
    quint8 szCmd[33]={0x0C,0x4D,0x05,0x6D,index,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::sendKey(quint8 key1,quint8 key0)
{
    quint8 szCmd[33]={0x0C,0x4D,0x06,0x20,key0,key1,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::sendKey(quint16 key)
{
    quint8 key1 = (key&0xFF);
    quint8 key0 = (key&0xFF00)>>8;
    quint8 szCmd[33]={0x0C,0x4D,0x06,0x20,key0,key1,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setOnline(bool on)
{
    quint8 nSet = on ? 0x66 : 0x67;
    quint8 szCmd[33]={0x0C,0x4D,0x05,nSet,0,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::askBattery()
{
    quint8 szCmd[33]={0x0C,0x4D,0x02,0x90,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::askStatus()
{
    quint8 szCmd[33]={0x0C,0x4D,0x02,0x69,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::startRecord()
{
    m_bRecord=true;
    quint8 szCmd[33]={0x0C,0x4D,0x05,0xB2,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::stopRecord()
{
    m_bRecord=false;
    quint8 szCmd[33]={0x0C,0x4D,0x05,0xB3,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setTime()
{
    quint32 now = time(nullptr);
    quint8 szCmd[33]={0x0C,0x4F,0x05,0xB3,00,00};
    memcpy(szCmd+2,&now,4);
    sendCmd(szCmd);
}

void CHidWorker::changeRecord()
{
    if(m_bRecord)
        startRecord();
    else
        stopRecord();
}

/*
16. 空鼠键
①　单击:  0xC0 //消除
②　双击:  0xC1 //物理激光
           0xC2 //数码激光
           0xC3 //聚光灯
           0xC4 //放大镜
           0xC5 //划线
③　长按:  0xC6 //物理激光按下
           0xC7 //数码激光按下
           0xC8 //聚光灯按下
           0xC9 //放大镜按下
           0xCA //划线按下
④　抬起:  0xCB //物理激光抬起
           0xCC //数码激光抬起
           0xCD //聚光灯抬起
           0xCE //放大镜抬起
           0xCF //划线抬起
   //上翻页
    UP_KEY_SINGLE_CLICK = 0xD0,
    UP_KEY_DOUBLE_CLICK = 0xD1,
    UP_KEY_LONG_DOWN = 0xD2,
    UP_KEY_LONG_UP = 0xD3,

    //下翻页
    DOWN_KEY_SINGLE_CLICK = 0xD4,
    DOWN_KEY_DOUBLE_CLICK = 0xD5,
    DOWN_KEY_LONG_DOWN = 0xD6,
    DOWN_KEY_LONG_UP = 0xD7,

    //语音按键
    AUDIO_PLAY_SINGLE_CLICK = 0xD8,
    AUDIO_WRITE_SINGLE_CLICK = 0xD9,
    AUDIO_SEARCH_SINGLE_CLICK = 0xDA,
    AUDIO_TRANSLATE_SINGLE_CLICK = 0xDB,

    AUDIO_PLAY_DOUBLE_CLICK = 0xDC,
    AUDIO_WRITE_DOUBLE_CLICK = 0xDD,
    AUDIO_SEARCH_DOUBLE_CLICK = 0xDE,
    AUDIO_TRANSLATE_DOUBLE_CLICK = 0xDF,

    AUDIO_PLAY_LONG_DOWN = 0xE0,
    AUDIO_WRITE_LONG_DOWN = 0xE1,
    AUDIO_SEARCH_LONG_DOWN = 0xE2,
    AUDIO_TRANSLATE_LONG_DOWN = 0xE3,

    AUDIO_PLAY_LONG_UP = 0xE4,
    AUDIO_WRITE_LONG_UP = 0xE5,
    AUDIO_SEARCH_LONG_UP = 0xE6,
    AUDIO_TRANSLATE_LONG_UP = 0xE7,

    AUDIO_AI_SINGLE_CLICK = 0xE8,
    AUDIO_AI_DOUBLE_CLICK = 0xE9,
    AUDIO_AI_LONG_DOWN = 0xEA,
    AUDIO_AI_LONG_UP = 0xEB,

*/
void CHidWorker::setMouseBtn(quint8 func)
{
    quint8 szCmd[33]={0x0C,0x4D,0x04,func,00,0x4C};
    sendCmd(szCmd);
}

void CHidWorker::setRecordPlay(bool set)
{
    m_bOutPlay = set;
}

void CHidWorker::StarRecorFile(const QString&strFile)
{
    if(!m_pDev) return;

    m_strTemp = g_strWork + "/temp.pcm";
    m_RecFile.setFileName(m_strTemp);

    m_strFile = strFile;
    if(!strFile.contains(":"))
        m_strFile = g_strWork + QString("/") + strFile;
    if(m_RecFile.open(QIODevice::WriteOnly))
    {
        m_recordTime = time(nullptr);
        startRecord();
    }
}

bool CHidWorker::WritePCMData(const QByteArray&data)
{
    if(m_RecFile.isOpen())
    {
        quint32 now = time(nullptr);
        onRecordFile(now - m_recordTime);
        m_RecFile.write(data);
        return true;
    }

    return false;
}

void CHidWorker::StopRecorFile()
{
    if(m_RecFile.isOpen())
    {
        onRecordFile(-1);
        stopRecord();
        m_RecFile.close();

        PCMFile2WAVFile(m_strTemp,m_strFile);
        QString strMP3 = m_strFile;
        strMP3.replace(".wav",".mp3");
        WAVFile2MP3File(m_strFile,strMP3);

        QFile::remove(m_strFile);
    }
}
