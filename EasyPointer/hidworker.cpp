
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>

#include "hidapi.h"
#include "hidworker.h"

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

#define	 NUM_OF_ORIG_SAMPLE				2

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
    for (i=0; i<len; i++) {

        if (1) {
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
        }

        if (0 && i < NUM_OF_ORIG_SAMPLE) {
            *pd++ = ps[i];
        }
        else {
            *pd++ = predict;
        }
    }
}


CHidWorker::CHidWorker()
{
    m_pDev = nullptr;
    m_pAPlayer = new CAudioPlayer(this);
    QTimer::singleShot(200,this,[=]{ start(); });
    QTimer *tmReadSN = new QTimer();
    tmReadSN->start(5000);
    connect(tmReadSN,&QTimer::timeout,this,[=]{
        readSN();
    });
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
        hid_device_info *pEDev = nullptr;
        while (!m_bEndWork)
        {
            pEDev = hid_enumerate(m_VID,m_PID);
            if(pEDev)
            {
                qDebug() << "hid_enumerate 2.4G" << Qt::hex << m_VID <<m_PID;
                break;
            }

            pEDev = hid_enumerate(m_VID,0x61AB);
            if(pEDev)
            {
                qDebug() << "hid_enumerate BLE" << Qt::hex << m_VID <<m_PID;
                break;
            }

            QThread::msleep(500);
        }

        hid_device_info *pTDev = pEDev; // trace
        while(pTDev)
        {
            qDebug() << pTDev->path << Qt::hex << pTDev->usage_page << pTDev->usage;
            pTDev = pTDev->next;
        }

        hid_device *pDev = nullptr; // to open
        pTDev = pEDev;
        while(pTDev)
        {
            if(pTDev->usage_page == 0xFF91)
            {
                pDev = hid_open_path(pTDev->path);
                if(pDev)
                {
                    m_strDevPath = pTDev->path;
                    break;
                }
            }

            pTDev = pTDev->next;
        }
        hid_free_enumeration(pEDev);

        if(!pDev) continue;

        QThread::msleep(200);

        qDebug() << "Open HID:"<< m_strDevPath;

        m_pDev = pDev;
        readSN();
        setMouse(false);
        setLaser(false);
        stopRecord();

        quint8 szBufs[20][64] = {{0}};
        int nUesed = 0;
        while(true)
        {
            quint8 *szBuf = szBufs[nUesed++ % 20];
            int nRet = hid_read_timeout(pDev,szBuf,65,0xFFFF);

            if(nRet <= 0)
                break;

            QMutexLocker Lock(&m_mutex);
            QByteArray Log((const char *)(szBuf),nRet);
            if(szBuf[0] != 0x1b)
            {
                qDebug().noquote()<<"USB <=: "<< Log.toHex(' ') << "Len: "<< nRet;
            }
            else
            {
                static quint8 eBuf[1024] = {0};
                encrypt(szBuf+1,eBuf,32);

                static short aBuf[1024] = {0};
                adpcm_to_pcm((short *)eBuf,aBuf,56);

                if(m_bOutPlay)
                {
                    m_pAPlayer->pushBuf(QByteArray((char *)aBuf,112));
                }
                else
                {
                    emit onPCMData((quint8 *)aBuf,112);
                }
            }

            emit onDataIn(szBuf,32);
        }

        hid_close(pDev);
        m_pDev = nullptr;
    }
}

void CHidWorker::sendCmd(quint8 *pCmd)
{
    if(!m_pDev)
        return;

    QByteArray Log((char *)pCmd,32);
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

void CHidWorker::setOnline(bool on)
{
    quint8 nSet = on ? 0x66 : 0x67;
    quint8 szCmd[33]={0x0C,0x4D,0x05,nSet,0,0x4C};
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

void CHidWorker::changeRecord()
{
    if(m_bRecord)
        stopRecord();
    else
        stopRecord();
}

void CHidWorker::setRecordPlay(bool set)
{
    m_bOutPlay = set ;
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
*/
void CHidWorker::setMouseBtn(quint8 func)
{
    quint8 szCmd[33]={0x0C,0x4D,0x04,func,00,0x4C};
    sendCmd(szCmd);
}
