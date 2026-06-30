#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QDialog>

namespace Ui {
class DialogManager;
}

class DialogManager : public QDialog
{
    Q_OBJECT

public:
    explicit DialogManager(QWidget *parent = nullptr);
    ~DialogManager();

private:
    Ui::DialogManager *ui;
};

#endif // DIALOGMANAGER_H
