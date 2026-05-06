#include "AppController.h"

#include "FileWorker.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace {
constexpr int MaxStatusLines = 300;
}

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &AppController::runPass);
}

AppController::~AppController()
{
    stopProcessing();
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(3000);
    }
}

bool AppController::running() const
{
    return m_running;
}

int AppController::progress() const
{
    return m_progress;
}

QString AppController::statusText() const
{
    return m_statusText;
}

QString AppController::defaultOutputFolder() const
{
    return QDir::toNativeSeparators(QDir::currentPath());
}

QString AppController::urlToLocalPath(const QUrl &url) const
{
    return QDir::toNativeSeparators(url.toLocalFile());
}

void AppController::startProcessing(const QString &inputFolder,
                                    const QString &fileMask,
                                    const QString &outputFolder,
                                    bool deleteInputFiles,
                                    int conflictMode,
                                    bool timerEnabled,
                                    int pollIntervalSeconds,
                                    const QString &xorKeyText)
{
    if (m_running) {
        return;
    }

    ProcessingSettings settings;
    if (!buildSettings(inputFolder, fileMask, outputFolder, deleteInputFiles, conflictMode,
                       timerEnabled, pollIntervalSeconds, xorKeyText, &settings)) {
        return;
    }

    m_settings = settings;
    m_processedInputFiles.clear();
    m_statusLines.clear();
    m_statusText.clear();
    emit statusTextChanged();
    setProgress(0);
    setRunning(true);

    runPass();
}

void AppController::stopProcessing()
{
    if (!m_running && !m_processing) {
        return;
    }

    m_timer.stop();
    setRunning(false);

    if (m_worker) {
        m_worker->requestStop();
        appendStatus(tr("Остановка обработки..."));
    } else {
        appendStatus(tr("Остановлено."));
    }
}

void AppController::runPass()
{
    if (!m_running || m_processing) {
        return;
    }

    m_processing = true;
    m_passHadError = false;
    m_settings.skippedInputFiles = m_settings.deleteInputFiles ? m_processedInputFiles : QStringList();

    // Отдельный поток для работы
    m_thread = new QThread(this);
    m_worker = new FileWorker(m_settings);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &FileWorker::process);
    connect(m_worker, &FileWorker::progressChanged, this, [this](int percent, const QString &fileName) {
        Q_UNUSED(fileName);
        setProgress(percent);
    });
    connect(m_worker, &FileWorker::messageChanged, this, &AppController::appendStatus);
    connect(m_worker, &FileWorker::fileProcessed, this, [this](const QString &inputPath) {
        const QString normalizedPath = QFileInfo(inputPath).absoluteFilePath();
        if (!m_processedInputFiles.contains(normalizedPath)) {
            m_processedInputFiles.append(normalizedPath);
        }
    });
    connect(m_worker, &FileWorker::errorOccurred, this, [this](const QString &message) {
        m_passHadError = true;
        appendStatus(tr("Ошибка: %1").arg(message));
    });
    connect(m_worker, &FileWorker::finished, this, &AppController::onWorkerFinished);
    connect(m_worker, &FileWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    appendStatus(tr("Запуск обработки: %1").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss")));
    m_thread->start();
}

void AppController::onWorkerFinished(bool canceled, int processedFiles)
{
    m_processing = false;
    clearWorker();

    if (canceled) {
        appendStatus(tr("Обработка остановлена."));
        setRunning(false);
        return;
    }

    if (m_settings.timerEnabled && m_running && !m_passHadError) {
        appendStatus(tr("Проход завершен. Обработано файлов: %1. Следующий опрос через %2 сек.")
                         .arg(processedFiles)
                         .arg(m_settings.pollIntervalSeconds));
        m_timer.start(m_settings.pollIntervalSeconds * 1000);
        return;
    }

    setRunning(false);
    appendStatus(tr("Работа завершена. Обработано файлов: %1.").arg(processedFiles));
}

bool AppController::buildSettings(const QString &inputFolder,
                                  const QString &fileMask,
                                  const QString &outputFolder,
                                  bool deleteInputFiles,
                                  int conflictMode,
                                  bool timerEnabled,
                                  int pollIntervalSeconds,
                                  const QString &xorKeyText,
                                  ProcessingSettings *settings)
{
    if (!settings) {
        return false;
    }
    // Переводим строку XOR в 8 байт
    QString keyText = xorKeyText;
    keyText.remove(QRegularExpression(QStringLiteral("\\s")));
    static const QRegularExpression keyRegex(QStringLiteral("^[0-9a-fA-F]{16}$"));

    if (!keyRegex.match(keyText).hasMatch()) {
        appendStatus(tr("Ошибка: XOR-ключ должен содержать ровно 16 hex-символов."));
        return false;
    }

    settings->inputFolder = cleanPath(inputFolder);
    settings->fileMask = fileMask.trimmed();
    settings->outputFolder = cleanPath(outputFolder);
    settings->deleteInputFiles = deleteInputFiles;
    settings->conflictMode = conflictMode == ProcessingSettings::AddCounter
        ? ProcessingSettings::AddCounter
        : ProcessingSettings::Overwrite;
    settings->timerEnabled = timerEnabled;
    settings->pollIntervalSeconds = qMax(1, pollIntervalSeconds);

    if (settings->inputFolder.isEmpty() || settings->outputFolder.isEmpty()) {
        appendStatus(tr("Ошибка: укажите входную папку и папку результата."));
        return false;
    }

    for (int i = 0; i < 8; ++i) {
        bool ok = false;
        const int value = keyText.mid(i * 2, 2).toInt(&ok, 16);
        if (!ok || value < 0 || value > 255) {
            appendStatus(tr("Ошибка: XOR-ключ должен содержать только символы 0-9 и A-F."));
            return false;
        }
        settings->xorKey[static_cast<size_t>(i)] = static_cast<quint8>(value);
    }

    return true;
}

QString AppController::cleanPath(const QString &path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.startsWith("file:", Qt::CaseInsensitive)) {
        return QUrl(trimmed).toLocalFile();
    }
    return QDir::fromNativeSeparators(trimmed);
}

void AppController::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }
    m_running = running;
    emit runningChanged();
}

void AppController::setProgress(int progress)
{
    progress = qBound(0, progress, 100);
    if (m_progress == progress) {
        return;
    }
    m_progress = progress;
    emit progressChanged();
}

void AppController::appendStatus(const QString &message)
{

    if (message.isEmpty()) {
        return;
    }

    const QStringList newLines = message.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : newLines) {
        m_statusLines.append(line);
    }
    // Храним лимит логов
    while (m_statusLines.size() > MaxStatusLines) {
        m_statusLines.removeFirst();
    }

    m_statusText = m_statusLines.join('\n');
    emit statusTextChanged();
}

void AppController::clearWorker()
{
    m_worker = nullptr;
    m_thread = nullptr;
}
