#include "DialogCloudCmd.h"
#include "ui_DialogCloudCmd.h"
#include "FrameAppTemplate.h"

#include <windows.h>
#include <winuser.h>

#include <QXlsx>
using namespace QXlsx;

#include <QJsonArray>
#include <QProcess>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>


#include <QSettings>
#include <QListView>
#include <QStandardPaths>

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QScreen>
#include <QFileInfo>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <shlobj.h>
#include <shlguid.h>
#include <objbase.h>
#include <atlbase.h>
#include <atlconv.h>

// 获取快捷方式目标路径
QString getShortcutTargetPath(const QString& shortcutPath)
{
    CComPtr<IShellLinkW> pShellLink;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&pShellLink))) {
        CComPtr<IPersistFile> pPersistFile;
        if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile))) {
            if (SUCCEEDED(pPersistFile->Load(shortcutPath.toStdWString().c_str(), STGM_READ))) {
                wchar_t targetPath[MAX_PATH];
                if (SUCCEEDED(pShellLink->GetPath(targetPath, MAX_PATH, NULL, SLGP_UNCPRIORITY))) {
                    return QString::fromWCharArray(targetPath);
                }
            }
        }
    }
    return "";
}

// 获取快捷方式图标
QPixmap getShortcutPixmap(const QString& targetPath, int iconIndex)
{
    HICON hIcon;
    SHFILEINFO shfi;
    ZeroMemory(&shfi, sizeof(SHFILEINFO));
    DWORD_PTR result = SHGetFileInfo(targetPath.toStdWString().c_str(), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES | SHGFI_ICONLOCATION);
    if (result)
    {
        hIcon = shfi.hIcon;
        return QPixmap::fromImage(QImage::fromHICON(hIcon));
    }
    return QPixmap();
}

static QSettings Set(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/NMYPointSetting.ini",QSettings::IniFormat);

static QList<FrameAppTemplate *>g_APPs;
static QStringList g_Checks;
static QJsonArray g_Commands;

DialogCloudCmd::DialogCloudCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCloudCmd)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    setStyleSheet("QLabel{color:black;font-weight:600;} QPushButton{color:black;font-weight:600;}");

    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(QString("类型,指令词,执行路径,启用,管理").split(','));
    ui->tableView->setModel(m_pModel);
    QHeaderView *pHeader = ui->tableView->horizontalHeader();
    pHeader->setSectionResizeMode(QHeaderView::Stretch);
    pHeader->setSectionResizeMode(0,QHeaderView::Fixed);
    pHeader->resizeSection(0,80);
    pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
    pHeader->resizeSection(1,240);
    pHeader->setSectionResizeMode(3,QHeaderView::Fixed);
    pHeader->resizeSection(3,80);
    pHeader->setSectionResizeMode(4,QHeaderView::Fixed);
    pHeader->resizeSection(4,60);
    ui->tableView->setFocusPolicy(Qt::NoFocus);

    saveLoadCommand(false);
    connect(m_pModel,&QStandardItemModel::itemChanged,this,[=](QStandardItem *item){
        saveLoadCommand();
    });

    connect(ui->tableView,&QTableView::clicked,this,[=](const QModelIndex &index){
        if(index.column() == 4)
        {
            if( QMessageBox::question(this,"提示","确定要删除此条指令词吗？") == QMessageBox::Yes)
            {
                m_pModel->removeRows(index.row(),1);
                saveLoadCommand();
            }
        }
    });

    connect(ui->pushButtonAdd,&QPushButton::clicked,this,[=]{
        QString strType = ui->comboBoxFunc->currentText();
        QString strWord = ui->lineEditWord->text().trimmed();
        QString strData = ui->lineEditData->text().trimmed();
        if(strWord.isEmpty() || strData.isEmpty())
        {
            QMessageBox::warning(this,"提示","指令词或者执行数据不能为空值！");
            return;
        }

        int count = m_pModel->rowCount();
        for(int i=0; i<count; i++)
        {
            QStandardItem *item1 = m_pModel->item(i,1);
            if(item1->text().trimmed() == strWord)
            {
                QMessageBox::warning(this,"提示","指令词已存在，请重试！");
                return;
            }
        }

        QStandardItem *item0 = new QStandardItem(strType);
        item0->setData(ui->comboBoxFunc->currentIndex());

        QStandardItem *item1 = new QStandardItem(strWord);
        QStandardItem *item2 = new QStandardItem(strData);
        QStandardItem *item3 = new QStandardItem("启用");
        QStandardItem *item4 = new QStandardItem("删除");
        item4->setTextAlignment(Qt::AlignCenter);

        item0->setEditable(false);
        item4->setEditable(false);
        item3->setEditable(false);
        item3->setCheckable(true);
        item3->setCheckState(Qt::Checked);
        m_pModel->appendRow({item0,item1,item2,item3,item4});

        saveLoadCommand();
    });

    connect(ui->pushButtonSet,&QPushButton::clicked,this,[=]{
        int type = ui->comboBoxFunc->currentIndex();
        QString strFile;
        if(type == 0)
        {
            strFile = QFileDialog::getOpenFileName(this,"打开文件", QDir::homePath(), tr("所有文件 (*.*)"));
        }
        if(type == 1)
        {
            strFile = QFileDialog::getExistingDirectory(this);
        }

        if(strFile.isEmpty())
            return;

        ui->lineEditData->setText(strFile);
    });

    {
        m_pModel1 = new QStandardItemModel(this);
        m_pModel1->setHorizontalHeaderLabels(QString("指令词,备注,启用").split(','));
        ui->tableView1->setModel(m_pModel1);
        QHeaderView *pHeader = ui->tableView1->horizontalHeader();
        pHeader->setSectionResizeMode(QHeaderView::Stretch);
        pHeader->setSectionResizeMode(0,QHeaderView::Fixed);
        pHeader->resizeSection(0,160);
        pHeader->setSectionResizeMode(2,QHeaderView::Fixed);
        pHeader->resizeSection(2,80);
        ui->tableView1->setFocusPolicy(Qt::NoFocus);

        saveLoadDefault(false);
        connect(m_pModel1,&QStandardItemModel::itemChanged,this,[=](QStandardItem *item){
            saveLoadDefault();
        });
    }

    {
        g_Checks = Set.value("checkedApps").toStringList();

        QVBoxLayout *pVLayout = (QVBoxLayout *)ui->scrollAreaWidgetContents->layout();
        pVLayout->setContentsMargins(5, 0, 5, 0);
        pVLayout->setAlignment(Qt::AlignLeft|Qt::AlignTop);

        CoInitialize(NULL);

        int nCount = 0;

        QStringList Dirs =  QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        Dirs.append("C:\\Users\\Public\\Desktop");
        Dirs.append(QDir::homePath() + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs") ;
        for(const QString&strPath:std::as_const(Dirs))
        {
            if (strPath.isEmpty())
                continue;

            QDir desktopDir(strPath);
            QStringList filters;
            filters << "*.lnk";
            QFileInfoList fileList = desktopDir.entryInfoList(filters, QDir::AllEntries,QDir::SortFlag::Name);
            for (const QFileInfo& fileInfo : fileList)
            {
                QString shortcutPath = fileInfo.filePath();
                QString targetPath = getShortcutTargetPath(shortcutPath);
                if (targetPath.isEmpty())
                    continue;

                nCount ++;

                CComPtr<IShellLinkW> pShellLink;
                if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&pShellLink)))
                    continue;
                wchar_t iconPath[MAX_PATH]={0};
                int iconIndex=-1;
                if (SUCCEEDED(pShellLink->GetIconLocation(iconPath, MAX_PATH, &iconIndex)))
                {
                    QString strName = fileInfo.fileName();
                    strName.replace(".lnk","");
                    QString strShort = fileInfo.filePath();

                    FrameAppTemplate *pAppItem = new FrameAppTemplate(this);

                    QString strUuid = QString("%1").arg(qHash(targetPath));
                    QString strCmd = Set.value(strUuid).toString();
                    pAppItem->setInfo(getShortcutPixmap(targetPath, iconIndex),strName,targetPath,strShort,strCmd);
                    pVLayout->addWidget(pAppItem,nCount);

                    if(g_Checks.contains(strShort))
                        pAppItem->setCheck();

                    g_APPs.push_back(pAppItem);

                    connect(pAppItem,&FrameAppTemplate::checkChanged,this,[=]{
                        g_Checks.clear();
                        for(FrameAppTemplate*app:std::as_const(g_APPs)){
                            if(app->isChecked())
                                g_Checks.append(app->m_strShortCut);
                        }
                        Set.setValue("checkedApps",g_Checks);
                    }) ;

                    connect(pAppItem,&FrameAppTemplate::commandChanged,this,[=](const QString&command, const QString&strExePath){
                        QString strUuid = QString("%1").arg(qHash(strExePath));
                        Set.setValue(strUuid,command);
                    }) ;
                }
            }
        }

        CoUninitialize();

        connect(ui->pushButtonAllOn,&QPushButton::clicked,this,[=]{

            static bool bCheckAll = true;
            for(FrameAppTemplate *app: g_APPs){
                app->setCheck(bCheckAll);
            }
            bCheckAll = !bCheckAll;

            g_Checks.clear();
            for(FrameAppTemplate*app:g_APPs){
                if(app->isChecked())
                    g_Checks.append(app->m_strShortCut);
            }
            Set.setValue("checkedApps",g_Checks);
        });
    }
}

void DialogCloudCmd::saveLoadCommand(bool save)
{
    QString strCmdFile = QApplication::applicationDirPath() + "/command.json";

    QFile file(strCmdFile);
    if(save)
    {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QJsonArray all;
            int count = m_pModel->rowCount();
            for(int i=0; i<count; i++)
            {
                QStandardItem *item0 = m_pModel->item(i,0);
                QStandardItem *item1 = m_pModel->item(i,1);
                QStandardItem *item2 = m_pModel->item(i,2);
                QStandardItem *item3 = m_pModel->item(i,3);

                QJsonObject jData;
                jData["type"]=item0->data().toInt();
                jData["word"]=item1->text();
                jData["data"]=item2->text();
                jData["enable"]=item3->checkState() == Qt::Checked;
                all.append(jData);
            }
            QJsonDocument jDoc(all);
            QByteArray jText = jDoc.toJson();
            file.write(jText);
            file.close();
        }
    }
    else
    {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString strText = file.readAll();
            file.close();
            QJsonDocument jDoc = QJsonDocument::fromJson(strText.toUtf8());
            if(jDoc.isArray())
            {
                QJsonArray all = jDoc.array();
                int count = all.count();
                for(int i=0; i<count; i++)
                {
                    QJsonObject jData = all[i].toObject();
                    int type = jData["type"].toInt();
                    QStandardItem *item0 = new QStandardItem(ui->comboBoxFunc->itemText(type));
                    QStandardItem *item1 = new QStandardItem(jData["word"].toString());
                    QStandardItem *item2 = new QStandardItem(jData["data"].toString());
                    QStandardItem *item3 = new QStandardItem("启用");
                    QStandardItem *item4 = new QStandardItem("删除");

                    item0->setEditable(false);
                    item4->setEditable(false);
                    item3->setEditable(false);
                    item3->setCheckable(true);
                    if(jData["enable"].toBool())
                        item3->setCheckState(Qt::Checked);

                    item4->setTextAlignment(Qt::AlignCenter);

                    m_pModel->appendRow({item0,item1,item2,item3,item4});
                }
            }
        }
    }
}

void DialogCloudCmd::saveLoadDefault(bool save)
{
    QString strCmdFile = QApplication::applicationDirPath() + "/default.json";

    QFile file(strCmdFile);
    if(save)
    {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QJsonArray all;
            int count = m_pModel1->rowCount();
            for(int i=0; i<count; i++)
            {
                QStandardItem *item0 = m_pModel1->item(i,0);
                QStandardItem *item1 = m_pModel1->item(i,1);
                QStandardItem *item2 = m_pModel1->item(i,2);

                QJsonObject jData;
                jData["word"]=item0->text();
                jData["text"]=item1->text();
                jData["enable"]=item2->checkState() == Qt::Checked;
                all.append(jData);
            }
            QJsonDocument jDoc(all);
            QByteArray jText = jDoc.toJson();
            file.write(jText);
            file.close();
        }
    }
    else
    {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString strText = file.readAll();
            file.close();
            QJsonDocument jDoc = QJsonDocument::fromJson(strText.toUtf8());
            if(jDoc.isArray())
            {
                QJsonArray all = jDoc.array();
                int count = all.count();
                for(int i=0; i<count; i++)
                {
                    QJsonObject jData = all[i].toObject();
                    QStandardItem *item0 = new QStandardItem(jData["word"].toString());
                    QStandardItem *item1 = new QStandardItem(jData["text"].toString());
                    QStandardItem *item2 = new QStandardItem("启用");

                    item0->setEditable(false);
                    item2->setEditable(false);
                    if(jData["enable"].toBool())
                        item2->setCheckState(Qt::Checked);

                    item2->setTextAlignment(Qt::AlignCenter);

                    m_pModel1->appendRow({item0,item1,item2});
                }
            }
        }
    }
}

DialogCloudCmd::~DialogCloudCmd()
{
    delete ui;
}

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "mmdevapi.lib")
// 设置系统音量 0.0~1.0
bool setMasterVolume(float fVol)
{

    CoInitialize(NULL);
    IMMDeviceEnumerator *pEnum = nullptr;
    if (FAILED(CoCreateInstance(
            __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator), (void**)&pEnum)))
    {
        qDebug() << "setMasterVolume failed 0";
        CoUninitialize();
        return false;
    }

    IMMDevice *pDev = nullptr;
    if (FAILED(pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDev)))
    {
        qDebug() << "setMasterVolume failed 1";
        pEnum->Release();
        CoUninitialize();
        return false;
    }

    IAudioEndpointVolume *pVol = nullptr;
    if (FAILED(pDev->Activate(
            __uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pVol)))
    {
        qDebug() << "setMasterVolume failed 2";
        pDev->Release();
        pEnum->Release();
        CoUninitialize();
        return false;
    }

    qDebug() << pVol->SetMasterVolumeLevelScalar(fVol, NULL);

    pVol->Release();
    pDev->Release();
    pEnum->Release();
    CoUninitialize();
    return true;
}

bool DialogCloudCmd::startupApp(const QString&command)
{
    QString strText = command;
    strText.replace(".","");
    strText.replace("。","");
    strText.replace("，","");
    strText.replace("嗯","");
    if(strText.isEmpty())
        return false;
    {
        int count = m_pModel1->rowCount();
        for(int i=0; i<count; i++)
        {
            QStandardItem *item0 = m_pModel1->item(i,0);
            QStandardItem *item1 = m_pModel1->item(i,1);
            QStandardItem *item2 = m_pModel1->item(i,2);
            if(item2->checkState() != Qt::Checked)
                continue;

            QString strKey = item0->text().trimmed();
            QString strPath = item1->text().trimmed();

            if(strKey == strText)
            {
                if(strKey == tr("返回桌面"))
                {
                    ShellExecuteA(NULL, "open", "C:\\Windows\\explorer.exe", "/n,/select,C:\\Users\\Default\\Desktop", NULL, SW_HIDE);
                }

                if(strKey == tr("窗口最小"))
                {
                }

                if(strKey == tr("运行"))
                {
                }

                if(strKey == tr("电脑锁屏"))
                {
                    LockWorkStation();
                }

                if(strKey == tr("电脑静音"))
                {
                    keybd_event(VK_VOLUME_MUTE, 0, 0, 0);
                    keybd_event(VK_VOLUME_MUTE, 0, KEYEVENTF_KEYUP, 0);
                }

                if(strKey == tr("音量10"))
                {
                    setMasterVolume(0.1);
                }
                if(strKey == tr("音量20"))
                {
                    setMasterVolume(0.2);
                }
                if(strKey == tr("音量30"))
                {
                    setMasterVolume(0.3);
                }
                if(strKey == tr("音量40"))
                {
                    setMasterVolume(0.4);
                }
                if(strKey == tr("音量50"))
                {
                    setMasterVolume(0.5);
                }
                if(strKey == tr("音量60"))
                {
                    setMasterVolume(0.6);
                }

                if(strKey == tr("音量70"))
                {
                    setMasterVolume(0.7);
                }
                if(strKey == tr("音量80"))
                {
                    setMasterVolume(0.8);
                }

                if(strKey == tr("音量90"))
                {
                    setMasterVolume(0.9);
                }

                if(strKey == tr("音量100"))
                {
                    setMasterVolume(1.0);
                }


                if(strKey == tr("屏幕亮度"))
                {
                    setMasterVolume(0.6);
                }

                if(strKey == tr("打开控制面板"))
                {
                    QProcess::execute("control.exe", QStringList{});
                }
                if(strKey == tr("屏幕黑屏"))
                {
                    m_pPad->showBlack();
                }

                if(strKey == tr("播放"))
                {
                }

                if(strKey == tr("打开放大镜"))
                {
                    //m_pWork->sendKey(0x08,0x2E);

                    QProcess::execute("magnify.exe", QStringList{});

                    // // 打开放大镜 Win + =
                    // INPUT input[4] = { 0 };

                    // // 按下 Win
                    // input[0].type = INPUT_KEYBOARD;
                    // input[0].ki.wVk = VK_LWIN;

                    // // 按下 =
                    // input[1].type = INPUT_KEYBOARD;
                    // input[1].ki.wVk = VK_OEM_PLUS;

                    // // 松开 =
                    // input[2].type = INPUT_KEYBOARD;
                    // input[2].ki.wVk = VK_OEM_PLUS;
                    // input[2].ki.dwFlags = KEYEVENTF_KEYUP;

                    // // 松开 Win
                    // input[3].type = INPUT_KEYBOARD;
                    // input[3].ki.wVk = VK_LWIN;
                    // input[3].ki.dwFlags = KEYEVENTF_KEYUP;

                    // SendInput(4, input, sizeof(INPUT));
                }

                if(strKey == tr("关闭放大镜"))
                {
                    QProcess::execute("taskkill", QStringList() << "/f" << "/im" << "magnify.exe");
                }
                if(strKey == tr("复制"))
                {
                }

                if(strKey == tr("粘贴"))
                {
                }

                if(strKey == tr("截屏"))
                {
                    QScreen* primaryScreen = QGuiApplication::primaryScreen();
                    QString strPath = QApplication::applicationDirPath() + "/CaptureFile";
                    QDir CD(strPath);
                    if(!CD.exists()) CD.mkdir(strPath);
                    QString strTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
                    primaryScreen->grabWindow(0).save(strPath + QString("/%1.png").arg(strTime));
                }

                return true;
            }
        }
    }

    {
        int count = m_pModel->rowCount();
        for(int i=0; i<count; i++)
        {
            QStandardItem *item0 = m_pModel->item(i,1);
            QStandardItem *item1 = m_pModel->item(i,2);
            QStandardItem *item2 = m_pModel->item(i,3);
            if(item2->checkState() != Qt::Checked)
                continue;

            QString strKey = item0->text().trimmed();
            QString strPath = item1->text().trimmed();

            if(strKey == strText)
            {
                ::ShellExecute(NULL, L"open", (LPCWSTR)strPath.toStdU16String().c_str(), NULL, NULL, SW_SHOWNORMAL);
                return true;
            }
        }
    }

    {
        for(FrameAppTemplate*app:std::as_const(g_APPs))
        {
            if(app->isChecked() && app->m_strCommand.trimmed() == strText.trimmed())
            {
                app->startup();
                return true;
            }
        }
    }
    return false;
}
