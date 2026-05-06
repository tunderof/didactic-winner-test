#include "FileWorker.h"

#include "XorAlgorithm.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSet>

namespace {
// Размер блока 1МЬ
constexpr qint64 BlockSize = 1024 * 1024;
}

FileWorker::FileWorker(const ProcessingSettings &settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

void FileWorker::requestStop()
{
    m_stopRequested = true;
}

void FileWorker::process()
{
    int processedFiles = 0;

    if (!validateFolders()) {
        emit finished(false, processedFiles);
        return;
    }

    const QList<QFileInfo> files = inputFiles();
    if (files.isEmpty()) {
        emit messageChanged(tr("Файлы по маске не найдены."));
        emit progressChanged(0, QString());
        emit finished(false, processedFiles);
        return;
    }

    quint64 totalBytes = 0;
    for (const QFileInfo &fileInfo : files) {
        totalBytes += static_cast<quint64>(qMax<qint64>(fileInfo.size(), 0));
    }

    quint64 doneBytes = 0;
    emitProgress(doneBytes, totalBytes, files.first().fileName());

    for (const QFileInfo &fileInfo : files) {
        if (m_stopRequested) {
            emit finished(true, processedFiles);
            return;
        }

        const QString inputPath = fileInfo.absoluteFilePath();
        const QString outputPath = outputPathFor(inputPath);

        emit messageChanged(tr("Обработка: %1").arg(fileInfo.fileName()));
        if (processFile(inputPath, outputPath, doneBytes, totalBytes)) {
            ++processedFiles;
            emit fileProcessed(inputPath);

            if (m_settings.deleteInputFiles && QFileInfo(inputPath).absoluteFilePath() != QFileInfo(outputPath).absoluteFilePath()) {
                if (!QFile::remove(inputPath)) {
                    emit errorOccurred(tr("Не удалось удалить входной файл: %1").arg(inputPath));
                }
            }
        } else if (m_stopRequested) {
            emit finished(true, processedFiles);
            return;
        }

        doneBytes += static_cast<quint64>(qMax<qint64>(fileInfo.size(), 0));
        emitProgress(doneBytes, totalBytes, fileInfo.fileName());
    }

    emit messageChanged(tr("Готово. Обработано файлов: %1").arg(processedFiles));
    emit finished(false, processedFiles);
}

QString FileWorker::normalizedMask() const
{
    QString mask = m_settings.fileMask.trimmed();
    if (mask.isEmpty()) {
        return "*";
    }
    if (mask.startsWith('.')) {
        return "*" + mask;
    }
    return mask;
}

QList<QFileInfo> FileWorker::inputFiles() const
{
    // Поиск файлов по маске
    QDir inputDir(m_settings.inputFolder);
    const QFileInfoList allFiles = inputDir.entryInfoList(
        QStringList() << normalizedMask(),
        QDir::Files | QDir::Readable,
        QDir::Name);

    QSet<QString> skipped;
    for (const QString &path : m_settings.skippedInputFiles) {
        skipped.insert(QFileInfo(path).absoluteFilePath());
    }

    QList<QFileInfo> result;
    for (const QFileInfo &fileInfo : allFiles) {
        if (!skipped.contains(fileInfo.absoluteFilePath())) {
            result.append(fileInfo);
        }
    }
    return result;
}

QString FileWorker::outputPathFor(const QString &inputPath) const
{
    // Вывод файлов
    const QFileInfo inputInfo(inputPath);
    QDir outputDir(m_settings.outputFolder);
    QString result = outputDir.filePath(inputInfo.fileName());

    if (m_settings.conflictMode == ProcessingSettings::Overwrite) {
        return result;
    }

    if (!QFileInfo::exists(result)) {
        return result;
    }

    const QString baseName = inputInfo.completeBaseName();
    const QString suffix = inputInfo.suffix();
    int counter = 1;

    while (true) {
        const QString fileName = suffix.isEmpty()
            ? QString("%1_%2").arg(baseName).arg(counter)
            : QString("%1_%2.%3").arg(baseName).arg(counter).arg(suffix);
        result = outputDir.filePath(fileName);

        if (!QFileInfo::exists(result)) {
            return result;
        }
        ++counter;
    }
}

bool FileWorker::validateFolders()
{
    const QFileInfo inputInfo(m_settings.inputFolder);
    if (!inputInfo.exists() || !inputInfo.isDir()) {
        emit errorOccurred(tr("Входная папка не существует: %1").arg(m_settings.inputFolder));
        return false;
    }

    QDir outputDir(m_settings.outputFolder);
    if (!outputDir.exists() && !QDir().mkpath(m_settings.outputFolder)) {
        emit errorOccurred(tr("Не удалось создать папку результата: %1").arg(m_settings.outputFolder));
        return false;
    }

    return true;
}

bool FileWorker::processFile(const QString &inputPath, const QString &outputPath, quint64 alreadyDone, quint64 totalBytes)
{
    QFile input(inputPath);
    if (!input.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Не удалось открыть входной файл: %1").arg(inputPath));
        return false;
    }

    QSaveFile output(outputPath);
    if (!output.open(QIODevice::WriteOnly)) {
        emit errorOccurred(tr("Не удалось открыть выходной файл: %1").arg(outputPath));
        return false;
    }

    quint64 fileOffset = 0;

    while (!input.atEnd()) {
        if (m_stopRequested) {
            output.cancelWriting();
            return false;
        }

        QByteArray buffer = input.read(BlockSize);
        if (buffer.isEmpty() && input.error() != QFileDevice::NoError) {
            output.cancelWriting();
            emit errorOccurred(tr("Ошибка чтения файла: %1").arg(inputPath));
            return false;
        }

        XorAlgorithm::apply(buffer, m_settings.xorKey, fileOffset);

        if (output.write(buffer) != buffer.size()) {
            output.cancelWriting();
            emit errorOccurred(tr("Ошибка записи файла: %1").arg(outputPath));
            return false;
        }

        fileOffset += static_cast<quint64>(buffer.size());
        emitProgress(alreadyDone + fileOffset, totalBytes, QFileInfo(inputPath).fileName());
    }

    input.close();

    if (!output.commit()) {
        emit errorOccurred(tr("Не удалось сохранить файл: %1").arg(outputPath));
        return false;
    }

    return true;
}

void FileWorker::emitProgress(quint64 doneBytes, quint64 totalBytes, const QString &fileName)
{
    // Прогресс
    const int percent = totalBytes == 0
        ? 100
        : static_cast<int>((doneBytes * 100) / totalBytes);
    emit progressChanged(qBound(0, percent, 100), fileName);
}
