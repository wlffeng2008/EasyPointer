
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>

#include "hidapi.h"
#include "hidworker.h"
#include "caudioplayer.h"

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
    QTimer::singleShot(200,this,[=]{ start(); });
}

CHidWorker::~CHidWorker()
{
    m_bEndWork = true;
    close();
}

void CHidWorker::close()
{
    if(m_pDev)
        hid_close(m_pDev);
    m_pDev = nullptr;
}

void CHidWorker::setHidVIDPID(unsigned short VID,unsigned short PID)
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
                qDebug() << "hid_enumerate" << Qt::hex << m_VID <<m_PID;
                break;
            }

            QThread::msleep(500);
        }

        hid_device_info *pTDev = pEDev;
        while(pTDev)
        {
            qDebug() << pTDev->path << Qt::hex << pTDev->usage_page << pTDev->usage;
            pTDev = pTDev->next;
        }

        hid_device *pDev = nullptr;
        pTDev = pEDev;
        while(pTDev)
        {
            m_strDevPath = pTDev->path;

            if(strstr(pTDev->path,"KBD"))
            {
            }

            if(pTDev->usage_page == 0xFF91)
            {
                pDev = hid_open_path(pTDev->path);
                if(pDev) break;
            }

            pTDev = pTDev->next;
        }
        hid_free_enumeration(pEDev);

        if(!pDev) continue;

        QThread::msleep(200);

        qDebug() << "Open HID:"<< m_strDevPath;

        m_pDev = pDev ;
        readSN();

        unsigned char szBufs[20][64] = {{0}};
        int nUesed = 0;
        while(true)
        {
            unsigned char *szBuf = szBufs[nUesed++%20] ;
            int nRet = hid_read_timeout(pDev,szBuf,64,100);

            if(nRet <= 0)
            {
                QThread::msleep(2);
                continue;
            }

            QMutexLocker Lock(&m_mutex);
            QByteArray Log((const char *)szBuf,32);
            if(szBuf[0] != 0x1b)
            {
                qDebug().noquote()<<"USB <=: "<< Log.toHex(' ');
            }
            else
            {
                static CAudioPlayer A;
                static short aBuf[1024]={0};
                adpcm_to_pcm((short *)szBuf,aBuf,nRet);
                A.pushBuf(QByteArray((char *)aBuf,nRet));
            }

            emit onDataIn(szBuf,32);
        }

        hid_close(pDev);
        m_pDev = nullptr;
    }
}

void CHidWorker::sendCmd(unsigned char nCmd,unsigned char nVal)
{
    if(!m_pDev)
        return;

    unsigned char nLen = 0x01;
    unsigned char nSet = nVal;
    if(nVal != 0xFF)
        nLen = 0x02;
    else
        nSet = 0x00;

    unsigned char szCmd[33]={0x0C,0x4D,0x04,0x61,00,0x4C} ;
    int nRet = hid_write(m_pDev,szCmd,33) ;
    Q_UNUSED(nRet) ;
    QByteArray Log((const char *)szCmd,32);
    qDebug().noquote()<<"USB =>: "<< Log.toHex(' ') ;

}

void CHidWorker::readSN()
{
    sendCmd(0x05);
}

void CHidWorker::setDPI(int nIndex)
{
    sendCmd(0x01,(unsigned char)nIndex);
}

void CHidWorker::setURL(int nOpen)
{
    sendCmd((unsigned char)(nOpen > 0 ? 0x06 : 0x07));
}

void CHidWorker::setSnap(int nOpen)
{
    sendCmd(0x08,(unsigned char)nOpen);
}

void CHidWorker::startRecord()
{
    sendCmd(0x03);
}

void CHidWorker::stopRecord()
{
    sendCmd(0x04);
}
