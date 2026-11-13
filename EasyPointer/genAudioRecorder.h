#ifndef GENAUDIORECORDER_H
#define GENAUDIORECORDER_H

#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    #include <QAudioSource>

#else

    #include <QAudioInput>

#endif

bool PCMFile2WAVFile(const QString&strPCMFile,const QString&strWAVFile);
bool WAVFile2MP3File(const QString&strWAVFile,const QString&strMP3File);

class genAudioRecorder : public QObject
{
    Q_OBJECT
public:
    explicit genAudioRecorder(QObject *parent = nullptr);

    void startRecord();
    void stopRecord();

signals:
    void audioDataIn(const QByteArray&data);

private:

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    QIODevice    *m_audioDevice = nullptr;
    QAudioSource *m_audioSource = nullptr;

#else

    QIODevice    *m_audioDevice = nullptr;
    QAudioInput  *m_audioSource = nullptr;

#endif
};

#endif // GENAUDIORECORDER_H
