#include "frameapptemplate.h"
#include "ui_frameapptemplate.h"

#include<Windows.h>

FrameAppTemplate::FrameAppTemplate(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::FrameAppTemplate)
{
    ui->setupUi(this);
    connect(ui->lineEditCmd,&QLineEdit::editingFinished,this,[=]{
        m_strCommand = ui->lineEditCmd->text().trimmed();
        emit commandChanged(m_strCommand,m_strExePath);
        qDebug() << QString("%1").arg(qHash(m_strExePath+m_strCommand));
    });
}

FrameAppTemplate::~FrameAppTemplate()
{
    delete ui;
}

void FrameAppTemplate::setInfo(const QPixmap&appIcon,const QString&appName,const QString&strExePath,const QString&shortcut,const QString&command)
{
    static int nIndex = 0;
    ui->labelIcon->setPixmap(appIcon);
    ui->labelName->setText(appName);
    ui->labelIndex->setText(QString::number(++nIndex));
    ui->lineEditCmd->setText(command);
    //ui->checkBox->setChecked(true);

    m_strAppName = appName;
    m_strExePath = strExePath;
    m_strShortCut = shortcut;
    m_strCommand = command;
}

bool FrameAppTemplate::isChecked()
{
    return ui->checkBox->isChecked();
}

void FrameAppTemplate::setCheck(bool setChecked)
{
    ui->checkBox->setChecked(setChecked);
}

void FrameAppTemplate::on_checkBox_clicked()
{
    emit checkChanged();
}

bool FrameAppTemplate::startup()
{
    LPCWSTR lnkPath = (const wchar_t*)m_strShortCut.utf16();
    HINSTANCE result = ShellExecute(NULL, L"open", lnkPath, NULL, NULL, SW_SHOWNORMAL);
    return true ;
}

