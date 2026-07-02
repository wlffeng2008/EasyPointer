#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QDialog>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QStringListModel>
#include <QStandardPaths>
#include <QDirIterator>
#include <QCheckBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>


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
    QMediaPlayer *m_pPlayer = nullptr;
    QAudioOutput *m_pAudOut = nullptr;
};

#endif // DIALOGMANAGER_H
