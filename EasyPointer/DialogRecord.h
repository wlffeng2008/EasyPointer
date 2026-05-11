#ifndef DIALOGRECORD_H
#define DIALOGRECORD_H
#include "xfwsvoicewrite.h"
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

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::DialogRecord *ui;

    XFWSVoiceWrite *m_XFV = nullptr;
};

#endif // DIALOGRECORD_H
