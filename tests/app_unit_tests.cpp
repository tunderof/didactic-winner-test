#include <QtTest/QtTest>

#include "AppController.h"
#include "FileWorker.h"
#include "XorAlgorithm.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

class AppUnitTests : public QObject
{
    Q_OBJECT

private slots:
    void xorKeepsOffsetBetweenBlocks();
    void fileWorkerProcessesFilesByMask();
    void fileWorkerSkipsAlreadyProcessedFiles();
    void fileWorkerAddsCounterOnNameConflict();
    void fileWorkerDeletesInputAfterSuccess();
    void appControllerRejectsInvalidXorKey();
    void appControllerAcceptsSpacedXorKey();
    void appControllerTimerReprocessesKeptInputFiles();
    void appControllerKeepsStatusLogBounded();
};

void AppUnitTests::xorKeepsOffsetBetweenBlocks()
{
    const std::array<quint8, 8> key = {1, 2, 3, 4, 5, 6, 7, 8};
    const QByteArray source = QByteArray::fromHex("00010203040506070809");

    QByteArray allAtOnce = source;
    XorAlgorithm::apply(allAtOnce, key, 0);

    QByteArray first = source.left(5);
    QByteArray second = source.mid(5);
    XorAlgorithm::apply(first, key, 0);
    XorAlgorithm::apply(second, key, 5);

    QCOMPARE(first + second, allAtOnce);
}

void AppUnitTests::fileWorkerProcessesFilesByMask()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile txt(inputDir.filePath("a.txt"));
    QVERIFY(txt.open(QIODevice::WriteOnly));
    QVERIFY(txt.write(QByteArray::fromHex("00010203")) == 4);
    txt.close();

    QFile bin(inputDir.filePath("b.bin"));
    QVERIFY(bin.open(QIODevice::WriteOnly));
    QVERIFY(bin.write(QByteArray::fromHex("10111213")) == 4);
    bin.close();

    ProcessingSettings settings;
    settings.inputFolder = inputDir.path();
    settings.fileMask = ".txt";
    settings.outputFolder = outputDir.path();
    settings.conflictMode = ProcessingSettings::AddCounter;
    settings.xorKey = {0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0};

    FileWorker worker(settings);
    QSignalSpy finishedSpy(&worker, &FileWorker::finished);
    worker.process();

    QCOMPARE(finishedSpy.count(), 1);
    QFile result(outputDir.filePath("a.txt"));
    QVERIFY(result.open(QIODevice::ReadOnly));
    QCOMPARE(result.readAll(), QByteArray::fromHex("fffefdfc"));
    QVERIFY(!QFile::exists(outputDir.filePath("b.bin")));
    QVERIFY(QFile::exists(inputDir.filePath("a.txt")));
}

void AppUnitTests::fileWorkerSkipsAlreadyProcessedFiles()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile skipped(inputDir.filePath("skipped.txt"));
    QVERIFY(skipped.open(QIODevice::WriteOnly));
    QVERIFY(skipped.write(QByteArray::fromHex("01020304")) == 4);
    skipped.close();

    QFile fresh(inputDir.filePath("fresh.txt"));
    QVERIFY(fresh.open(QIODevice::WriteOnly));
    QVERIFY(fresh.write(QByteArray::fromHex("05060708")) == 4);
    fresh.close();

    ProcessingSettings settings;
    settings.inputFolder = inputDir.path();
    settings.fileMask = "*.txt";
    settings.outputFolder = outputDir.path();
    settings.skippedInputFiles = QStringList() << QFileInfo(skipped).absoluteFilePath();
    settings.xorKey = {0, 0, 0, 0, 0, 0, 0, 0};

    FileWorker worker(settings);
    worker.process();

    QVERIFY(!QFile::exists(outputDir.filePath("skipped.txt")));
    QVERIFY(QFile::exists(outputDir.filePath("fresh.txt")));
}

void AppUnitTests::fileWorkerAddsCounterOnNameConflict()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile input(inputDir.filePath("same.txt"));
    QVERIFY(input.open(QIODevice::WriteOnly));
    QVERIFY(input.write(QByteArray::fromHex("01020304")) == 4);
    input.close();

    QFile oldOutput(outputDir.filePath("same.txt"));
    QVERIFY(oldOutput.open(QIODevice::WriteOnly));
    QVERIFY(oldOutput.write("old") == 3);
    oldOutput.close();

    ProcessingSettings settings;
    settings.inputFolder = inputDir.path();
    settings.fileMask = "same.txt";
    settings.outputFolder = outputDir.path();
    settings.conflictMode = ProcessingSettings::AddCounter;
    settings.xorKey = {0, 0, 0, 0, 0, 0, 0, 0};

    FileWorker worker(settings);
    worker.process();

    QFile untouched(outputDir.filePath("same.txt"));
    QVERIFY(untouched.open(QIODevice::ReadOnly));
    QCOMPARE(untouched.readAll(), QByteArray("old"));

    QFile result(outputDir.filePath("same_1.txt"));
    QVERIFY(result.open(QIODevice::ReadOnly));
    QCOMPARE(result.readAll(), QByteArray::fromHex("01020304"));
}

void AppUnitTests::fileWorkerDeletesInputAfterSuccess()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile input(inputDir.filePath("remove.txt"));
    QVERIFY(input.open(QIODevice::WriteOnly));
    QVERIFY(input.write(QByteArray::fromHex("0001")) == 2);
    input.close();

    ProcessingSettings settings;
    settings.inputFolder = inputDir.path();
    settings.fileMask = "remove.txt";
    settings.outputFolder = outputDir.path();
    settings.deleteInputFiles = true;
    settings.xorKey = {1, 1, 0, 0, 0, 0, 0, 0};

    FileWorker worker(settings);
    worker.process();

    QVERIFY(QFile::exists(outputDir.filePath("remove.txt")));
    QVERIFY(!QFile::exists(inputDir.filePath("remove.txt")));
}

void AppUnitTests::appControllerRejectsInvalidXorKey()
{
    AppController controller;

    controller.startProcessing("input", ".bin", "output", false, 0, false, 5, "ABC");

    QVERIFY(!controller.running());
    QVERIFY(controller.statusText().contains("16"));
}

void AppUnitTests::appControllerAcceptsSpacedXorKey()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile input(inputDir.filePath("spaced.bin"));
    QVERIFY(input.open(QIODevice::WriteOnly));
    QVERIFY(input.write(QByteArray::fromHex("00010203")) == 4);
    input.close();

    AppController controller;
    controller.startProcessing(inputDir.path(),
                               "spaced.bin",
                               outputDir.path(),
                               false,
                               ProcessingSettings::Overwrite,
                               false,
                               5,
                               "FF FF FF FF 00 00 00 00");

    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(outputDir.filePath("spaced.bin")), 3000);

    QFile result(outputDir.filePath("spaced.bin"));
    QVERIFY(result.open(QIODevice::ReadOnly));
    QCOMPARE(result.readAll(), QByteArray::fromHex("fffefdfc"));
}

void AppUnitTests::appControllerTimerReprocessesKeptInputFiles()
{
    QTemporaryDir inputDir;
    QTemporaryDir outputDir;
    QVERIFY(inputDir.isValid());
    QVERIFY(outputDir.isValid());

    QFile input(inputDir.filePath("repeat.bin"));
    QVERIFY(input.open(QIODevice::WriteOnly));
    QVERIFY(input.write(QByteArray::fromHex("01020304")) == 4);
    input.close();

    AppController controller;
    controller.startProcessing(inputDir.path(),
                               "repeat.bin",
                               outputDir.path(),
                               false,
                               ProcessingSettings::AddCounter,
                               true,
                               1,
                               "00 00 00 00 00 00 00 00");

    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(outputDir.filePath("repeat.bin")), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(outputDir.filePath("repeat_1.bin")), 4000);

    controller.stopProcessing();
}

void AppUnitTests::appControllerKeepsStatusLogBounded()
{
    AppController controller;

    for (int i = 0; i < 350; ++i) {
        controller.startProcessing("input", ".bin", "output", false, 0, false, 5, "ABC");
    }

    QVERIFY(controller.statusText().split('\n').size() <= 300);
}

QTEST_MAIN(AppUnitTests)

#include "app_unit_tests.moc"
