#ifndef DIALOGRECORD_H
#define DIALOGRECORD_H

#include "DialogCloudCmd.h"
#include "DialogTypeWord.h"

#include <QDialog>

namespace Ui {
class DialogRecord;
}


typedef int CGKeyCode;
typedef int CGEventFlags;

void simulateKeyDown(CGKeyCode keyCode);
void simulateKeyUp(CGKeyCode keyCode);
void simulateKeyPress(CGKeyCode keyCode);

class DialogRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRecord(QWidget *parent = nullptr);
    ~DialogRecord();

    void writePCM(char *data,int len);
    void setFunc(int nFunc=1);
    void setRelate(DialogCloudCmd *pCmdDlg,DialogTypeWord *pSetDlg);
    void setOutsizeText(const QString&text,int state=0);
    void setPaintText(bool set=true);
    void setType(int type=0);
    void DoFlush(bool emitback=false);

signals:
    void onFlushText(const QString&text);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::DialogRecord *ui;
    quint8 m_nFunc = 0;
    DialogCloudCmd *m_pCmdDlg = nullptr;
    DialogTypeWord *m_pSetDlg = nullptr;
    bool m_bCanPaint = true;
};

#endif // DIALOGRECORD_H
