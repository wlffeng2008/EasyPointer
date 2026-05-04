#ifndef DIALOGMKEYSET_H
#define DIALOGMKEYSET_H

#include <QDialog>

namespace Ui {
class DialogMKeySet;
}

class DialogMKeySet : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMKeySet(QWidget *parent = nullptr);
    ~DialogMKeySet();

private:
    Ui::DialogMKeySet *ui;
};

#endif // DIALOGMKEYSET_H
