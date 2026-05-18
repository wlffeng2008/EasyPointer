#ifndef DIALOGRECORD_H
#define DIALOGRECORD_H
#include "xfwsvoicewrite.h"
#include "xftexttranslate.h"
#include "DialogCloudCmd.h"
#include "DialogTypeWord.h"

#include <QDialog>

namespace Ui {
class DialogRecord;
}

class DialogRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRecord(QWidget *parent = nullptr);
    ~DialogRecord();

    void writePCM(char *data,int len);
    void setFunc(int nFunc=1);
    void setRelate(DialogCloudCmd *pCmdDlg,DialogTypeWord *pSetDlg);
protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::DialogRecord *ui;
    quint8 m_nFunc = 0;
    XFWSVoiceWrite *m_XFV = nullptr;
    FlyTextTranslate *m_XFT = nullptr;
    DialogCloudCmd *m_pCmdDlg = nullptr;
    DialogTypeWord *m_pSetDlg = nullptr;
};

#endif // DIALOGRECORD_H
