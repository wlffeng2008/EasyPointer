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

static QSettings Set(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/NMYPointSetting.ini",QSettings::IniFormat) ;

static QList<FrameAppTemplate *>g_APPs;
static QStringList g_Checks;
static QJsonArray g_Commands;


DialogCloudCmd::DialogCloudCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCloudCmd)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    setStyleSheet("QLabel{color:black;font-weight:600;}");

    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(QString("类型,指令词,执行路径,删除").split(','));
    ui->tableView->setModel(m_pModel);
    QHeaderView *pHeader = ui->tableView->horizontalHeader();
    pHeader->setSectionResizeMode(QHeaderView::Stretch);
    pHeader->setSectionResizeMode(0,QHeaderView::Fixed);
    pHeader->resizeSection(0,80);
    pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
    pHeader->resizeSection(1,240);
    pHeader->setSectionResizeMode(3,QHeaderView::Fixed);
    pHeader->resizeSection(3,60);
    ui->tableView->setFocusPolicy(Qt::NoFocus);

    saveLoadCommand(false);

    connect(ui->tableView,&QTableView::clicked,this,[=](const QModelIndex &index){
        if(index.column() == 3)
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
        item0->setEditable(false);

        QStandardItem *item1 = new QStandardItem(strWord);
        QStandardItem *item2 = new QStandardItem(strData);
        QStandardItem *item3 = new QStandardItem("删除");
        item3->setTextAlignment(Qt::AlignCenter);

        m_pModel->appendRow({item0,item1,item2,item3});

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
        //pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
        //pHeader->resizeSection(1,240);
        pHeader->setSectionResizeMode(2,QHeaderView::Fixed);
        pHeader->resizeSection(2,100);
        ui->tableView1->setFocusPolicy(Qt::NoFocus);


        // "返回桌面","窗口最小","
        //                 电脑关机","
        //                     运行","
        //                         电脑锁屏","
        //                             电脑静音","
        //                                 音量10","
        //                                     音量20","
        //                                         音量30","
        //                                             音量50","
        //                                                 音量60","
        //                                                     音量70","
        //                                                         音量80","
        //                                                             音量90","
        //                                                                 音量100","
        //                                                                     屏幕亮度30%","
        //         屏幕亮度50%","
        //         屏幕亮度70%","
        //         屏幕亮度80%","
        //         屏幕亮度100%","
        //         打开控制面板","屏幕黑屏","播放","打开放大镜","关闭放大镜","Ctrl+c","Ctrl+v","截屏"};
        QString strFile = QApplication::applicationDirPath() + "/defalutCommand.xlsx";
        Document xlsx(strFile);
        if (!xlsx.load())
        {
            QMessageBox::critical(this, "提示", "文件无法加载！\n" + strFile);
        }
        else
        {
            for(int i=0; i<200; i++)
            {
                QString strCell0 = xlsx.read(QString::asprintf("A%d", i+2)).toString().trimmed();
                QString strCell1 = xlsx.read(QString::asprintf("B%d", i+2)).toString().trimmed();
                QString strCell2 = xlsx.read(QString::asprintf("C%d", i+2)).toString().trimmed();
                if(strCell0.isEmpty())
                    continue;

                QStandardItem *pItem0 = new QStandardItem(strCell0);
                QStandardItem *pItem1 = new QStandardItem(strCell2);
                QStandardItem *pItem2 = new QStandardItem("启用");
                pItem2->setCheckable(true);
                pItem2->setCheckState(Qt::Checked);

                pItem0->setEditable(false);
                pItem1->setEditable(false);
                pItem2->setEditable(false);

                m_pModel1->appendRow({pItem0,pItem1,pItem2});
            }
        }
    }

    {
        g_Checks = Set.value("checkedApps").toStringList();

        QVBoxLayout *pVLayout = (QVBoxLayout *)ui->scrollAreaWidgetContents->layout() ;
        pVLayout->setContentsMargins(5, 0, 5, 0);
        pVLayout->setAlignment(Qt::AlignLeft|Qt::AlignTop);

        CoInitialize(NULL);

        int nCount = 0;

        QStringList Dirs =  QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        Dirs.append("C:\\Users\\Public\\Desktop") ;
        Dirs.append(QDir::homePath() + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs") ;
        for(const QString&strPath:Dirs)
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

                nCount ++ ;

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

                    FrameAppTemplate *pAppItem = new FrameAppTemplate(this) ;

                    QString strUuid = QString("%1").arg(qHash(targetPath));
                    QString strCmd = Set.value(strUuid).toString();
                    pAppItem->setInfo(getShortcutPixmap(targetPath, iconIndex),strName,targetPath,strShort,strCmd);
                    pVLayout->addWidget(pAppItem,nCount);

                    if(g_Checks.contains(strShort))
                        pAppItem->setCheck();

                    g_APPs.push_back(pAppItem);

                    connect(pAppItem,&FrameAppTemplate::checkChanged,this,[=]{
                        g_Checks.clear();
                        for(FrameAppTemplate*app:g_APPs){
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

                QJsonObject jData;
                jData["type"]=item0->data().toInt();
                jData["word"]=item1->text();
                jData["data"]=item2->text();
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
                    QStandardItem *item3 = new QStandardItem("删除");
                    item3->setTextAlignment(Qt::AlignCenter);

                    m_pModel->appendRow({item0,item1,item2,item3});
                }
            }
            file.close();
        }
    }
}

DialogCloudCmd::~DialogCloudCmd()
{
    delete ui;
}

bool DialogCloudCmd::startupApp(const QString&command)
{
    if(command.isEmpty())
        return false;
    QString strText = command;
    strText.replace(".","");
    strText.replace("。","");
    strText.replace("，","");
    strText.replace("嗯","");

    int count = m_pModel->rowCount();
    for(int i=0; i<count; i++)
    {
        QStandardItem *item0 = m_pModel->item(i,1);
        QStandardItem *item1 = m_pModel->item(i,2);

        QString strKey = item0->text().trimmed();
        QString strPath = item1->text().trimmed();

        if(strKey == strText)
        {
            //QProcess::startDetached(strPath);

            HINSTANCE result = ::ShellExecute(NULL, L"open", (LPCWSTR)strPath.toStdU16String().c_str(), NULL, NULL, SW_SHOWNORMAL);
            return true;
        }
    }

    for(FrameAppTemplate*app:g_APPs)
    {
        if(app->isChecked() && app->m_strCommand.trimmed() == strText.trimmed())
        {
            app->startup();
            return true;
        }
    }
    return false;
}
