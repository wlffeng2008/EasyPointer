#ifndef TYPEWORD_H
#define TYPEWORD_H

#include <QWidget>
#include <QJsonArray>

namespace Ui {
class typeword;
}

class typeword : public QWidget
{
    Q_OBJECT

public:
    explicit typeword(QWidget *parent = nullptr);
    ~typeword();

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
    Ui::typeword *ui;

    QJsonArray m_langs;
    QJsonArray m_trans;
};

#endif // TYPEWORD_H
