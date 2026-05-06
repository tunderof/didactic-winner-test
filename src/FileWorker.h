#pragma once

#include "ProcessingSettings.h"

#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QString>

#include <atomic>

class FileWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileWorker(const ProcessingSettings &settings, QObject *parent = nullptr);

    void requestStop();

public slots:
    void process();

signals:
    void progressChanged(int percent, const QString &currentFile);
    void messageChanged(const QString &message);
    void errorOccurred(const QString &message);
    void fileProcessed(const QString &inputPath);
    void finished(bool canceled, int processedFiles);

private:
    QString normalizedMask() const;
    QList<QFileInfo> inputFiles() const;
    QString outputPathFor(const QString &inputPath) const;
    bool validateFolders();
    bool processFile(const QString &inputPath, const QString &outputPath, quint64 alreadyDone, quint64 totalBytes);
    void emitProgress(quint64 doneBytes, quint64 totalBytes, const QString &fileName);

    ProcessingSettings m_settings;
    std::atomic_bool m_stopRequested = false;
};
