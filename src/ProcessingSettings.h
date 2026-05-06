#pragma once

#include <QString>
#include <QStringList>
#include <QtGlobal>

#include <array>

struct ProcessingSettings
{
    enum ConflictMode {
        Overwrite = 0,
        AddCounter = 1
    };

    QString inputFolder;
    QString fileMask;
    QString outputFolder;
    QStringList skippedInputFiles;
    bool deleteInputFiles = false;
    ConflictMode conflictMode = Overwrite;
    bool timerEnabled = false;
    int pollIntervalSeconds = 5;
    std::array<quint8, 8> xorKey = {0, 0, 0, 0, 0, 0, 0, 0};
};
