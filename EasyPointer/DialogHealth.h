#ifndef DIALOGHEALTH_H
#define DIALOGHEALTH_H

#include <QDialog>
class BleWorker;

namespace Ui {
class DialogHealth;
}

class DialogHealth : public QDialog
{
    Q_OBJECT

public:
    explicit DialogHealth(QWidget *parent = nullptr);
    ~DialogHealth();

    BleWorker *m_Reader = nullptr;

private:
    Ui::DialogHealth *ui;
};

#endif // DIALOGHEALTH_H
