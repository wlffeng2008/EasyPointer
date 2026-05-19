#include "DialogCloudCmd.h"
#include "ui_DialogCloudCmd.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

DialogCloudCmd::DialogCloudCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCloudCmd)
{
    ui->setupUi(this);

    //setWindowFlags(windowFlags() | Qt::Dialog |Qt::Tool);

    m_pModel = new QStandardItemModel(this);
    m_pModel->setHorizontalHeaderLabels(QString("类型,指令词,执行路径,删除").split(','));
    ui->tableView->setModel(m_pModel);
    QHeaderView *pHeader = ui->tableView->horizontalHeader();
    pHeader->setSectionResizeMode(QHeaderView::Stretch);
    pHeader->setSectionResizeMode(0,QHeaderView::Fixed);
    pHeader->resizeSection(0,80);
    pHeader->setSectionResizeMode(1,QHeaderView::Fixed);
    pHeader->resizeSection(1,240);
    ui->tableView->setFocusPolicy(Qt::NoFocus);
    pHeader->resizeSection(3,60);
    pHeader->setSectionResizeMode(3,QHeaderView::Fixed);

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
