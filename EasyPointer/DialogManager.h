#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QDialog>

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QStandardItemModel>
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


class CHidWorker ;

class DialogManager : public QDialog
{
    Q_OBJECT

public:
    explicit DialogManager(QWidget *parent = nullptr);
    ~DialogManager();

    QString getRecordPath();
    QString getCapturePath();
    QString StartRecord(bool record);

signals:
    void onRecord(bool record);

private:
    Ui::DialogManager *ui;
    QMediaPlayer *m_pPlayer = nullptr;
    QAudioOutput *m_pAudOut = nullptr;

    QString m_strMP3;
    quint32 m_recordCount = 0;
    bool m_bRecording = false;
    QTimer *m_pTMRec = nullptr;

    qint64 m_nDur = 0 ;
    int m_nSelRow = 0 ;
    int m_nPlayRow = -1 ;
    int m_nItemCount = 0 ;

    bool m_bPlaying = false;
    bool m_bPaused = false;

    void playNext(int nOffset);

    void SaveLoadRecord(bool save=true);
    void AddRecord(const QString&strFile,quint32 duration,bool newOne=true);
    QStandardItemModel *m_model=nullptr;
};

#endif // DIALOGMANAGER_H
