#include "DialogManager.h"
#include "ui_DialogManager.h"

#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <windows.h>

#include <QDateTime>
#include <mainwindow.h>

DialogManager::DialogManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogManager)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    setStyleSheet("color:black; font-size:12px;");
    QSettings *pSet = ::getUserSetting();

    ui->lineEditRecord->setText(pSet->value("RecordPath",::getUserDataPath() +"/RecordFile").toString());
    ui->lineEditCapture->setText(pSet->value("CapturePath",::getUserDataPath() +"/CaptureFile").toString());
    ui->horizontalSlider->setValue(0);
    ui->horizontalSliderVol->setValue(pSet->value("Volume",60).toInt());

    m_pTMRec = new QTimer(this);
    m_pTMRec->start(990);
    connect(m_pTMRec,&QTimer::timeout,this,[=]{
        if(m_bRecording)
        {
            m_recordCount++;
            ui->labelRecordFile->setText(m_strMP3);
        }
        else
        {
            m_recordCount = 0;
            ui->labelRecordFile->setText("None");
        }

        int remain = m_recordCount;
        int hour = remain/3600;
        int min = remain%3600/60;
        int sec = remain%60;
        QString strInfo = QString::asprintf("%02d:%02d:%02d",hour,min,sec);
        ui->labelRecordInfo->setText(strInfo);
    });

    connect(ui->checkBoxRecord,&QCheckBox::clicked,this,[=](bool checked){
        emit onRecord(checked);
    });

    connect(ui->lineEditRecord,&QLineEdit::textEdited,this,[=](const QString&text){
        QDir MD(text.trimmed());
        if(!MD.exists()) MD.mkdir(text.trimmed());
    });

    connect(ui->lineEditCapture,&QLineEdit::textEdited,this,[=](const QString&text){
        QDir MD(text.trimmed());
        if(!MD.exists()) MD.mkdir(text.trimmed());
    });

    connect(ui->pushButtonRecord,&QPushButton::clicked,this,[=]{
        QString strPath = QFileDialog::getExistingDirectory(this);
        if(strPath.isEmpty()) return;
        ui->lineEditRecord->setText(strPath);
    });

    connect(ui->pushButtonCapture,&QPushButton::clicked,this,[=]{
        QString strPath = QFileDialog::getExistingDirectory(this);
        if(strPath.isEmpty()) return;
        ui->lineEditCapture->setText(strPath);
    });

    connect(ui->pushButtonGoRec,&QPushButton::clicked,this,[=]{
        QString strPath = ui->lineEditRecord->text().trimmed();
        if(strPath.isEmpty()) return;
        ::ShellExecute(NULL, L"open", (LPCWSTR)strPath.toStdU16String().c_str(), NULL, NULL, SW_SHOWNORMAL);
    });
    connect(ui->pushButtonGoCap,&QPushButton::clicked,this,[=]{
        QString strPath = ui->lineEditCapture->text().trimmed();
        if(strPath.isEmpty()) return;
        ::ShellExecute(NULL, L"open", (LPCWSTR)strPath.toStdU16String().c_str(), NULL, NULL, SW_SHOWNORMAL);
    });

    {
        m_model = new QStandardItemModel(this);
        m_model->setHorizontalHeaderLabels({"文件名","时长","管理"});
        ui->tableView->setModel(m_model);

        QHeaderView *pHeader = ui->tableView->horizontalHeader();
        pHeader->setSectionResizeMode(QHeaderView::Stretch);
        pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
        pHeader->resizeSection(1,120);
        pHeader->setSectionResizeMode(2,QHeaderView::Fixed);
        pHeader->resizeSection(2,80);
    }

}

DialogManager::~DialogManager()
{
    delete ui;
}

QString DialogManager::getRecordPath()
{
    return ui->lineEditRecord->text().trimmed();
}

QString DialogManager::getCapturePath()
{
    return ui->lineEditCapture->text().trimmed();
}

void DialogManager::AddRecord(const QString&strFile,quint32 duration)
{
    int remain = duration;
    int hour = remain/3600;
    int min = remain%3600/60;
    int sec = remain%60;
    QString strInfo = QString::asprintf("%02d:%02d:%02d",hour,min,sec);

    QStandardItem *item0 = new QStandardItem(strFile);
    QStandardItem *item1 = new QStandardItem(strInfo);
    QStandardItem *item2 = new QStandardItem("删除");
    item0->setEditable(false);
    item1->setEditable(false);
    item2->setEditable(false);
    m_model->appendRow({item0,item1,item2});
}

QString DialogManager::StartRecord(bool record)
{
    QString strFile;
    if(record)
    {
        QDateTime NOW = QDateTime::currentDateTime();
        m_strMP3 = NOW.toString("yyyy-MM-dd_HH-mm-ss") + QString(".wav") ;
        strFile = getRecordPath() + QString("/") + m_strMP3;
    }
    else
    {
        AddRecord(m_strMP3,m_recordCount);
    }

    m_recordCount = 0;
    m_bRecording = record;
    ui->checkBoxRecord->setChecked(record);
    return strFile;
}