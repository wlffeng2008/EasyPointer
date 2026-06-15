#ifndef DIALOGTYPEWORD_H
#define DIALOGTYPEWORD_H

#include <QDialog>
#include <QJsonArray>

namespace Ui {
class DialogTypeWord;
}

class DialogTypeWord : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTypeWord(QWidget *parent = nullptr);
    ~DialogTypeWord();

    QString getMontherLang(bool bIsTyping=true);
    QString getMontherAccent(bool bIsTyping=true);
    QString getTransLang1();
    QString getTransLang2();

protected:
    void changeEvent(QEvent *pEvt);
private slots:
    void on_pushButtonOK_clicked();

private:

    Ui::DialogTypeWord *ui;
    QJsonArray m_langs;
    QJsonArray m_trans;
};

#endif // DIALOGTYPEWORD_H
