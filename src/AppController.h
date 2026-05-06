#pragma once

#include "ProcessingSettings.h"

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QStringList>

class FileWorker;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString defaultOutputFolder READ defaultOutputFolder CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool running() const;
    int progress() const;
    QString statusText() const;
    QString defaultOutputFolder() const;

    Q_INVOKABLE QString urlToLocalPath(const QUrl &url) const;
    Q_INVOKABLE void startProcessing(const QString &inputFolder,
                                     const QString &fileMask,
                                     const QString &outputFolder,
                                     bool deleteInputFiles,
                                     int conflictMode,
                                     bool timerEnabled,
                                     int pollIntervalSeconds,
                                     const QString &xorKeyText);
    Q_INVOKABLE void stopProcessing();

signals:
    void runningChanged();
    void progressChanged();
    void statusTextChanged();

private slots:
    void runPass();
    void onWorkerFinished(bool canceled, int processedFiles);

private:
    bool buildSettings(const QString &inputFolder,
                       const QString &fileMask,
                       const QString &outputFolder,
                       bool deleteInputFiles,
                       int conflictMode,
                       bool timerEnabled,
                       int pollIntervalSeconds,
                       const QString &xorKeyText,
                       ProcessingSettings *settings);
    QString cleanPath(const QString &path) const;
    void setRunning(bool running);
    void setProgress(int progress);
    void appendStatus(const QString &message);
    void clearWorker();

    ProcessingSettings m_settings;
    QStringList m_processedInputFiles;
    QStringList m_statusLines;
    QTimer m_timer;
    QThread *m_thread = nullptr;
    FileWorker *m_worker = nullptr;
    bool m_running = false;
    bool m_processing = false;
    bool m_passHadError = false;
    int m_progress = 0;
    QString m_statusText;
};
