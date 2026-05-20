#ifndef FRAMEAPPTEMPLATE_H
#define FRAMEAPPTEMPLATE_H

#include <QFrame>

namespace Ui {
class FrameAppTemplate;
}

class FrameAppTemplate : public QFrame
{
    Q_OBJECT

public:
    explicit FrameAppTemplate(QWidget *parent = nullptr);
    ~FrameAppTemplate();

    void setInfo(const QPixmap&appIcon, const QString&appName, const QString&strExePath, const QString&shortcut, const QString &command);
    void setCheck(bool setChecked=true);
    bool isChecked();
    bool startup();
    QString m_strExePath;
    QString m_strAppName;
    QString m_strShortCut;
    QString m_strCommand;

private slots:
    void on_checkBox_clicked();

signals:
    void checkChanged();
    void commandChanged(const QString&command, const QString&strExePath);

private:
    Ui::FrameAppTemplate *ui;
};

#endif // FRAMEAPPTEMPLATE_H
