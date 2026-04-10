
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>

#include "hidapi.h"
#include "hidworker.h"

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
            //int nRet = hid_get_feature_report(pDev,szBuf,13);
            //int nRet = hid_get_input_report(pDev,szBuf,13);

            if(nRet <= 0)
            {
                QThread::msleep(2);
                continue ;
            }

            QMutexLocker Lock(&m_mutex);
            QByteArray Log((const char *)szBuf,32);
            qDebug().noquote()<<"USB <=: "<< Log.toHex(' ');

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
