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

    QString getMontherLang(bool bIsTyping=true) ;
    QString getMontherAccent(bool bIsTyping=true) ;
    QString getTransLang1() ;
    QString getTransLang2() ;
    bool autoPressReturn() ;

    void ShowVoiceText(const QString&strText,bool bAppend=true) ;
    void showLog(const QString&strLog,bool bEnd) ;
    QString getLastText() ;
protected:
    void changeEvent(QEvent *pEvt);
private:

    Ui::DialogTypeWord *ui;
    QJsonArray m_langs;
    QJsonArray m_trans;
};

#endif // DIALOGTYPEWORD_H
