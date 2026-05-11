#ifndef DIALOGRECORD_H
#define DIALOGRECORD_H

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
protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::DialogRecord *ui;
};

#endif // DIALOGRECORD_H
