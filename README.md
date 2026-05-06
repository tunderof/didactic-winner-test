## Структура проекта

```text
test_file_xor/
  CMakeLists.txt
  main.cpp
  Main.qml
  generate_test_file.py

  src/
    AppController.h/.cpp
    ProcessingSettings.h/.cpp
    FileWorker.h/.cpp
    XorAlgorithm.h/.cpp

  tests/
    app_unit_tests.cpp
    test_generate_test_file.py
```

Основные роли классов:

- `ProcessingSettings` хранит настройки обработки
- `XorAlgorithm` выполняет только XOR над блоком байтов
- `FileWorker` отвечает за файловую работу: поиск, проверку папок, чтение, запись, удаление исходника
- `AppController` связывает интерфейс с обработкой, управляет потоком и таймером

## Сборка Release через Qt Creator

1. Открыть `CMakeLists.txt` в Qt Creator.
2. Выбрать kit с Qt 6 и MinGW.
3. В конфигурации сборки выбрать `Release`.
4. Нажать `Configure Project`.
5. Собрать проект через `Build`.
6. Исполняемый файл будет в release build-папке, имя файла: `apptest_file_xor.exe`.

## Запуск тестов

После сборки:

```powershell
ctest --test-dir build-release --output-on-failure
```

В текущем проекте C++ тесты проверяют:

- корректность XOR при обработке файла по блокам;
- поиск файлов по маске;
- пропуск уже обработанных файлов в режиме таймера;
- режим добавления счетчика к имени;
- удаление входного файла после успешной обработки;
- проверку XOR-ключа;
- ограничение размера лога статуса.

Python-тесты генератора можно запустить так:

```powershell
python -m unittest tests\test_generate_test_file.py
```

## Использование приложения

1. Указать входную папку.
2. Указать маску файлов:
   - `.txt` будет преобразовано в `*.txt`;
   - `*.bin` найдет все `.bin`;
   - `testFile.bin` найдет конкретный файл.
3. Указать папку результата.
4. Выбрать, удалять ли входные файлы после успешной обработки.
5. Выбрать поведение при совпадении имени:
   - `Перезаписать`;
   - `Добавить счетчик`, например `file_1.bin`.
6. Выбрать режим запуска:
   - `Разовый запуск`;
   - `По таймеру`.
7. Ввести период опроса в секундах, если выбран таймер.
8. Ввести XOR-ключ: ровно 8 hex-байт, например:

```text
01 02 03 04 05 06 07 08
```

## Генератор тестовых файлов

Скрипт `generate_test_file.py` создает случайный `.bin` или `.txt` файл в текущей папке запуска. Имя формируется автоматически:

```text
YYYYMMDD_HHMMSS_SIZE.ext
```

Примеры:

```powershell
py generate_test_file.py -s 100MB -t bin
py generate_test_file.py --size 10KB --type txt
py generate_test_file.py -s 1GB -t bin
py generate_test_file.py --help
```

Поддерживаемые единицы размера:

```text
B, KB, MB, GB
```