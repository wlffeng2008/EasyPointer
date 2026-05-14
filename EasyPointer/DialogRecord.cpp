#include "DialogRecord.h"
#include "ui_DialogRecord.h"

#include <QScreen>

DialogRecord::DialogRecord(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRecord)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint|Qt::WindowStaysOnTopHint);

    m_XFV = new XFWSVoiceWrite();
    connect(m_XFV,&XFWSVoiceWrite::send_voice_text,this,[=](const QString &strText,const QString &strpgs, int nTextSN, bool bTeminate){
        qDebug() << strText;
        ui->textEdit->setText(strText);
    });

    QString m_strTypeLang="zh_cn" ;
    QString m_strTypeAccent="mandarin" ;
    m_XFV->setMontherLanguage(m_strTypeLang,m_strTypeAccent);
}

DialogRecord::~DialogRecord()
{
    delete ui;
}

void DialogRecord::showEvent(QShowEvent *event)
{
    int nPosX = QApplication::screens().at(0)->geometry().width() - size().width() - 20;
    int nPosY = QApplication::screens().at(0)->geometry().height() - size().height() - 30;
    QRect rcTip(nPosX,nPosY,size().width(),size().height());

    setGeometry(rcTip);

    m_XFV->ReqAuthAudio();
    QDialog::showEvent(event);
}

#define MAX(a,b) ( ((a) > (b)) ? (a):(b) )
#define MIN(a,b) ( ((a) < (b)) ? (a):(b) )


void DialogRecord::writePCM(char *data,int len)
{
    if(!data || !len)
    {
        m_XFV->WritePcmEnd();
        return ;
    }
    //qDebug() <<  "writePCM"  << len ;
    m_XFV->WritePcmData(data,len);

    short minValue = -32767;
    short maxValue = 32767;
    int   micVolume = 0;

    short *pVoice = (short *)data;
    for(int i=0;i<len/2;i++)
    {
        int value = MAX(MIN(pVoice[i],maxValue), minValue);
        micVolume = MAX(micVolume,value);
    }

    ui->progressBar->setValue(micVolume*600/maxValue);
}