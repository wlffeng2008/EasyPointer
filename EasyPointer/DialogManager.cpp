#include "DialogManager.h"
#include "ui_DialogManager.h"

#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <windows.h>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>
#include <mainwindow.h>

QString formatTimeLen(int nLen, bool bHour = false)
{
    QString strInfo ;
    char szOut[1024]={0} ;
    int nRuns = nLen ;
    int nH1 = nRuns/3600 ;
    int nM1 = (nRuns%3600)/60;
    int nS1 = nRuns%60;

    if(nRuns >= 3600 || bHour)
    {
        sprintf(szOut,"%02d:%02d:%02d",nH1,nM1,nS1);
    }
    else
    {
        sprintf(szOut,"%02d:%02d",nM1,nS1);
    }
    strInfo = szOut;
    return strInfo;
}

DialogManager::DialogManager(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogManager)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    setStyleSheet("color:black; font-size:12px;");
    QSettings *pSet = ::getUserSetting();


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

        QString strInfo = ::formatTimeLen(m_recordCount);
        ui->labelRecordInfo->setText(strInfo);
    });

    connect(ui->checkBoxRecord,&QCheckBox::clicked,this,[=](bool checked){
        emit onRecord(checked);
    });

    connect(ui->lineEditRecord,&QLineEdit::textChanged,this,[=](const QString&text){
        QDir MD(text.trimmed());
        if(!MD.exists()) MD.mkdir(text.trimmed());
    });

    connect(ui->lineEditCapture,&QLineEdit::textChanged,this,[=](const QString&text){
        QDir MD(text.trimmed());
        if(!MD.exists()) MD.mkdir(text.trimmed());
    });

    ui->lineEditRecord->setText(pSet->value("RecordPath",::getUserDataPath() +"/RecordFile").toString());
    ui->lineEditCapture->setText(pSet->value("CapturePath",::getUserDataPath() +"/CaptureFile").toString());

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
        float volume = pSet->value("Volume",60).toInt();
        ui->horizontalSliderVol->setValue(volume);
        m_pPlayer = new QMediaPlayer(this);
        m_pAudOut = new QAudioOutput(this);
        m_pPlayer->setAudioOutput(m_pAudOut);
        m_pAudOut->setVolume(volume/100);

        connect(ui->horizontalSliderVol,&QSlider::valueChanged,[=](int value){
            m_pAudOut->setVolume(value/100.0);
        });

        connect(ui->pushButtonPlay,&QPushButton::clicked,[=](bool checked){
            if(m_bPlaying)
            {
                if(m_pPlayer->isPlaying())
                    m_pPlayer->pause();
                else
                    m_pPlayer->play();
            }
            else
            {
                m_nPlayRow = m_nSelRow;
                playNext(0);
            }
        });

        connect(m_pPlayer,&QMediaPlayer::durationChanged,[=](qint64 nDur){
            ui->horizontalSliderPrg->setRange(0,nDur);
            m_bPlaying = true ;
            m_bPaused = false ;
            m_nDur = nDur ;
        });
        connect(m_pPlayer,&QMediaPlayer::positionChanged,[=](qint64 pos){
            ui->horizontalSliderPrg->setValue(pos);
            m_bPlaying = true ;
            m_bPaused = false ;
        });

        connect(m_pPlayer,&QMediaPlayer::playbackStateChanged,[=](QMediaPlayer::PlaybackState newState){
            qDebug()<< newState ;
            if(newState == QMediaPlayer::StoppedState)
            {
                m_bPaused = false ;
                m_bPlaying = false ;
                ui->horizontalSliderPrg->setValue(0);
            }
        });

        connect(m_pPlayer,&QMediaPlayer::mediaStatusChanged,[=](QMediaPlayer::MediaStatus status){
            qDebug()<< status ;
            if(QMediaPlayer::EndOfMedia == status)
            {
                m_bPaused = false ;
                m_bPlaying = false ;
                ui->horizontalSliderPrg->setValue(0);
                //if(ui->checkBoxLoopPlay->isChecked())
                //playNext(1);
            }
        });

        connect(ui->pushButtonStop,&QPushButton::clicked,[=](){
            m_pPlayer->stop();
            m_bPlaying = false ;
            //ui->labelDuration->setText("00:00 / 00:00");
            //ui->labelVoiceFile->setText("");
        });

        connect(ui->horizontalSliderPrg,&QSlider::sliderMoved,[=](int nPos){
            m_pPlayer->setPosition(nPos);
        });

        connect(ui->horizontalSliderPrg,&QSlider::sliderPressed,[=](){
            m_pPlayer->setPosition(ui->horizontalSliderPrg->sliderPosition());
        });

        connect(ui->tableView,&QTableView::clicked,this,[=](const QModelIndex &index){
            if(m_bPlaying) return;
            if(index.column() == 3)
            {
                QFile::remove(getRecordPath() + "/" + m_model->item(index.row(),0)->text());
                m_model->removeRow(index.row());

                SaveLoadRecord();
                m_nSelRow = -1;
                return;
            }
            m_nSelRow = index.row();
            ui->labelPlayFile->setText(m_model->item(index.row(),0)->text());
            ui->labelFileTime->setText(m_model->item(index.row(),1)->text());
        });
        connect(ui->tableView,&QTableView::doubleClicked,this,[=](const QModelIndex &index){
            if(m_bPlaying) return;
            m_nSelRow = index.row();
            m_nPlayRow= index.row();
            ui->labelPlayFile->setText(m_model->item(index.row(),0)->text());
            ui->labelFileTime->setText(m_model->item(index.row(),1)->text());
            playNext(0);
        });
    }

    {
        m_model = new QStandardItemModel(this);
        m_model->setHorizontalHeaderLabels({"文件名","时长","","管理"});
        ui->tableView->setModel(m_model);

        QHeaderView *pHeader = ui->tableView->horizontalHeader();
        pHeader->setSectionResizeMode(QHeaderView::Stretch);
        pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
        pHeader->resizeSection(1,120);
        pHeader->setSectionResizeMode(2,QHeaderView::Fixed);
        pHeader->resizeSection(2,0);
        pHeader->hideSection(2);
        pHeader->setSectionResizeMode(3,QHeaderView::Fixed);
        pHeader->resizeSection(3,80);

        SaveLoadRecord(false);
    }

}

void DialogManager::playNext(int nOffset)
{
    int count = m_model->rowCount();
    if(count == 0)
        return ;
    int nRow = m_nPlayRow + nOffset;

    if(nRow >= count)
        nRow = 0 ;
    if(nRow<0)
        nRow = m_nItemCount - 1;

    m_nPlayRow = nRow ;

    QString strText = m_model->item(nRow,0)->text();
    ui->labelPlayFile->setText(strText) ;
    ui->labelFileTime->setText(m_model->item(nRow,1)->text());
    QString strFile = getRecordPath() + QString("/") + strText ;
    m_bPaused = false;
    m_pPlayer->stop();
    m_pPlayer->setSource(QUrl::fromLocalFile(strFile));
    m_pPlayer->play();
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

void DialogManager::AddRecord(const QString&strFile, quint32 duration, bool newOne)
{
    QString strInfo = ::formatTimeLen(duration);

    QStandardItem *item0 = new QStandardItem(strFile);
    QStandardItem *item1 = new QStandardItem(strInfo);
    QStandardItem *item2 = new QStandardItem(QString("%1").arg(duration));
    QStandardItem *item3 = new QStandardItem("删除");
    item0->setEditable(false);
    item1->setEditable(false);
    item3->setEditable(false);
    item1->setTextAlignment(Qt::AlignCenter);
    item3->setTextAlignment(Qt::AlignCenter);
    m_model->appendRow({item0,item1,item2,item3});

    if(newOne)
    {
        SaveLoadRecord();
    }
}

void DialogManager::SaveLoadRecord(bool save)
{
    QString strDBFile = getRecordPath() + "/record.json";
    QFile RecF(strDBFile);
    if(save)
    {
        int count = m_model->rowCount();
        QJsonArray jArr;
        for(int i=0; i<count; i++)
        {
            QJsonObject jObj;
            jObj["file"]= m_model->item(i,0)->text();
            jObj["time"]= m_model->item(i,2)->text().toInt();
            jArr.append(jObj);
        }

        QJsonDocument jDoc(jArr);
        if(RecF.open(QIODevice::WriteOnly))
        {
            RecF.write(jDoc.toJson());
            RecF.close();
        }
    }
    else
    {
        if(RecF.open(QIODevice::ReadOnly))
        {
            QString text = RecF.readAll();
            QJsonDocument jDoc = QJsonDocument::fromJson(text.toUtf8());
            if(jDoc.isArray())
            {
                QJsonArray jArr = jDoc.array();
                int count = jArr.count();
                for(int i=0; i<count; i++)
                {
                    QJsonObject jObj = jArr[i].toObject();
                    AddRecord(jObj["file"].toString(),jObj["time"].toInt(),false);
                }
            }
            RecF.close();
        }
    }
}

QString DialogManager::StartRecord(bool record)
{
    QString strFile;
    if(record)
    {
        QDateTime NOW = QDateTime::currentDateTime();
        m_strMP3 = NOW.toString("yyyy-MM-dd_HH-mm-ss") + QString(".mp3") ;
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