#include "DialogRecord.h"
#include "ui_DialogRecord.h"

#include <Windows.h>
#include <QScreen>
#include <QClipboard>
#include <QTimer>

void SetTextToWinWithUnicode(const QString&strText)
{
    QClipboard *pClip = QApplication::clipboard() ;
    pClip->setText(strText);
    keybd_event(VK_CONTROL,0x45, KEYEVENTF_EXTENDEDKEY , 0);
    keybd_event('V',0x45, KEYEVENTF_EXTENDEDKEY , 0);
    Sleep(10) ;
    keybd_event('V',0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    keybd_event(VK_CONTROL,0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    return ;

    std::u16string text = strText.toStdU16String();

    for (size_t i = 0; i < text.size(); i++){
        INPUT inputs[2] = {{0}};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = 0;
        inputs[0].ki.wScan = 0;
        inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = 0;
        inputs[1].ki.wScan = 0;
        inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs[0].ki.wScan = text[i];
        inputs[1].ki.wScan = text[i];
        SendInput(2, inputs, sizeof(INPUT));
    }
}

// typedef int CGKeyCode;
// typedef int CGEventFlags;

void simulateKeyDown(CGKeyCode keyCode)
{
    keybd_event(keyCode,0x45, KEYEVENTF_EXTENDEDKEY , 0);
}

void simulateKeyUp(CGKeyCode keyCode)
{
    keybd_event(keyCode,0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

void simulateKeyPress(CGKeyCode keyCode)
{
    qDebug()<<"simulateKeyPress()" << keyCode;

    simulateKeyDown(keyCode);
    simulateKeyUp(keyCode);
}

DialogRecord::DialogRecord(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRecord)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::MSWindowsFixedSizeDialogHint|Qt::WindowStaysOnTopHint);

    // m_XFV = new XFWSVoiceWrite();
    // connect(m_XFV,&XFWSVoiceWrite::send_voice_text,this,[=](const QString &strText,const QString &strpgs, int nTextSN, bool bTeminate){
    //     qDebug() << strText;
    //     ui->textEdit->setText(strText);
    //     if(bTeminate)
    //     {
    //         if(m_nFunc == 2)
    //         {
    //             m_XFT->translateText(strText,m_pSetDlg->getTransLang1(),m_pSetDlg->getTransLang2());
    //         }
    //         else
    //         {
    //             SetTextToWinWithUnicode(strText);
    //             if(ui->checkBox->isChecked())
    //                 simulateKeyPress(VK_RETURN);
    //         }
    //     }
    // });

    // m_XFT = new FlyTextTranslate(this);
    // connect(m_XFT,&FlyTextTranslate::finish_translate,this,[=](const QString&text){
    //     ui->textEdit->setText(text);
    //     SetTextToWinWithUnicode(text);
    //     if(ui->checkBox->isChecked())
    //         simulateKeyPress(VK_RETURN);
    // });

    // QString m_strTypeLang="zh_cn";
    // QString m_strTypeAccent="mandarin";
    // m_XFV->setMontherLanguage(m_strTypeLang,m_strTypeAccent);
}

void DialogRecord::setPaintText(bool set)
{
    m_bCanPaint = set;
}

void DialogRecord::setType(int type)
{
    m_bCanPaint = (type>0);
    QStringList types={tr("云指令"),tr("语音打字"),tr("语音翻译")};
    ui->labelType->setText(types[type]);
}

void DialogRecord::setOutsizeText(const QString&text,int state)
{
    ui->textEdit->setText(text);
    if(state == 2 && m_bCanPaint)
    {
        DoFlush();
    }
}

void DialogRecord::DoFlush(bool emitback)
{
    QString text = ui->textEdit->toPlainText().trimmed();
    if(text.isEmpty())
        return;

    if(emitback)
    {
        emit onFlushText(text);
    }
    else
    {
        SetTextToWinWithUnicode(text);
        if(ui->checkBox->isChecked())
            simulateKeyPress(VK_RETURN);
    }

    QTimer::singleShot(10000,this,[=]{
        //ui->textEdit->setText("");
    });
}

DialogRecord::~DialogRecord()
{
    delete ui;
}

void DialogRecord::setRelate(DialogCloudCmd *pCmdDlg,DialogTypeWord *pSetDlg)
{
    m_pCmdDlg = pCmdDlg;
    m_pSetDlg = pSetDlg;
}

void DialogRecord::showEvent(QShowEvent *event)
{
    int nPosX = QApplication::screens().at(0)->geometry().width() - size().width() - 20;
    int nPosY = QApplication::screens().at(0)->geometry().height() - size().height() - 30;
    QRect rcTip(nPosX,nPosY,size().width(),size().height());

    setGeometry(rcTip);

    QDialog::showEvent(event);
}

void DialogRecord::setFunc(int nFunc)
{
    ui->labelType->setText(nFunc == 1 ? tr("语音打字"): tr("翻译打字"));

    m_nFunc = nFunc;
}

#define MAX(a,b) ( ((a) > (b)) ? (a):(b) )
#define MIN(a,b) ( ((a) < (b)) ? (a):(b) )

void DialogRecord::writePCM(char *data,int len)
{
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