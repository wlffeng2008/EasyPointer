#include "DialogMKeySet.h"
#include "ui_DialogMKeySet.h"
#include <QListView>
#include <QSettings>
#include <QStandardPaths>
DialogMKeySet::DialogMKeySet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMKeySet)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog|Qt::MSWindowsFixedSizeDialogHint);

    ui->comboBoxEngine->addItem(tr("百度"));
    ui->comboBoxEngine->addItem(tr("谷歌"));
    ui->comboBoxEngine->addItem(tr("雅虎"));

    static QSettings Set(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/NMYPointSetting.ini",QSettings::IniFormat) ;
    connect(ui->comboBoxEngine,&QComboBox::activated,this,[](int index){ Set.setValue("Engine",index) ; });

    int nIndex = Set.value("Engine").toInt();
    ui->comboBoxEngine->setCurrentIndex(nIndex);

    ui->comboBoxEngine->setView(new QListView());
}

DialogMKeySet::~DialogMKeySet()
{
    delete ui;
}


int DialogMKeySet::getEngine()
{
    return ui->comboBoxEngine->currentIndex() ;
}

void DialogMKeySet::changeEvent(QEvent *pEvt)
{
    if(pEvt->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        ui->comboBoxEngine->setItemText(0,tr("百度"));
        ui->comboBoxEngine->setItemText(1,tr("谷歌"));
        ui->comboBoxEngine->setItemText(2,tr("雅虎"));
    }

    QWidget::changeEvent(pEvt);
}
