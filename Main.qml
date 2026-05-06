import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: root

    property color bgColor: "#f3f5f7"
    property color panelColor: "#ffffff"
    property color textColor: "#18212f"
    property color mutedTextColor: "#5f6b7a"
    property color disabledTextColor: "#8b95a1"
    property color borderColor: "#cfd6df"

    width: 820
    height: 620
    minimumWidth: 720
    minimumHeight: 560
    visible: true
    title: qsTr("XOR Processor")

    palette.window: bgColor
    palette.windowText: textColor
    palette.text: textColor
    palette.buttonText: textColor
    palette.base: panelColor
    palette.placeholderText: mutedTextColor
    palette.highlight: "#8b95a1"
    palette.highlightedText: "#ffffff"
    palette.disabled.text: disabledTextColor
    palette.disabled.buttonText: disabledTextColor
    palette.disabled.windowText: disabledTextColor
    palette.disabled.base: panelColor
    palette.disabled.button: panelColor
    palette.disabled.highlight: "#b4bcc6"

    AppController {
        id: controller
    }

    FolderDialog {
        id: inputDialog
        title: qsTr("Выберите входную папку")
        onAccepted: inputFolder.text = controller.urlToLocalPath(selectedFolder)
    }

    FolderDialog {
        id: outputDialog
        title: qsTr("Выберите папку для результата")
        onAccepted: outputFolder.text = controller.urlToLocalPath(selectedFolder)
    }

    Rectangle {
        anchors.fill: parent
        color: root.bgColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 14

            GridLayout {
                Layout.fillWidth: true
                columns: 3
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: qsTr("Входная папка")
                    color: root.textColor
                }
                TextField {
                    id: inputFolder
                    Layout.fillWidth: true
                    color: enabled ? root.textColor : root.disabledTextColor
                    placeholderText: qsTr("C:/")
                    enabled: !controller.running
                    selectByMouse: true
                    selectionColor: "#c8d0da"
                    selectedTextColor: root.textColor
                    background: Rectangle {
                        color: root.panelColor
                        border.color: root.borderColor
                        radius: 3
                    }
                }
                Button {
                    id: inputBrowseButton
                    text: qsTr("Обзор")
                    enabled: !controller.running
                    onClicked: inputDialog.open()
                    contentItem: Text {
                        text: inputBrowseButton.text
                        color: inputBrowseButton.enabled ? root.textColor : root.disabledTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#eef1f5"
                        border.color: root.borderColor
                        radius: 3
                    }
                }

                Label {
                    text: qsTr("Маска файлов")
                    color: root.textColor
                }
                TextField {
                    id: fileMask
                    Layout.fillWidth: true
                    text: ".txt"
                    color: enabled ? root.textColor : root.disabledTextColor
                    placeholderText: qsTr(".txt, *.bin или testFile.bin")
                    enabled: !controller.running
                    selectByMouse: true
                    selectionColor: "#c8d0da"
                    selectedTextColor: root.textColor
                    background: Rectangle {
                        color: root.panelColor
                        border.color: root.borderColor
                        radius: 3
                    }
                }
                Item { width: 1; height: 1 }

                Label {
                    text: qsTr("Папка результата")
                    color: root.textColor
                }
                TextField {
                    id: outputFolder
                    Layout.fillWidth: true
                    text: controller.defaultOutputFolder
                    color: enabled ? root.textColor : root.disabledTextColor
                    placeholderText: qsTr("C:/")
                    enabled: !controller.running
                    selectByMouse: true
                    selectionColor: "#c8d0da"
                    selectedTextColor: root.textColor
                    background: Rectangle {
                        color: root.panelColor
                        border.color: root.borderColor
                        radius: 3
                    }
                }
                Button {
                    id: outputBrowseButton
                    text: qsTr("Обзор")
                    enabled: !controller.running
                    onClicked: outputDialog.open()
                    contentItem: Text {
                        text: outputBrowseButton.text
                        color: outputBrowseButton.enabled ? root.textColor : root.disabledTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#eef1f5"
                        border.color: root.borderColor
                        radius: 3
                    }
                }
            }

            GroupBox {
                title: qsTr("Настройки")
                Layout.fillWidth: true

                label: Label {
                    text: parent.title
                    color: root.textColor
                    font.bold: true
                }

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 18
                    rowSpacing: 10

                    CheckBox {
                        id: deleteInputs
                        text: qsTr("Удалять входные файлы после обработки")
                        enabled: !controller.running
                        Layout.columnSpan: 2
                    }

                    Label {
                        text: qsTr("Если имя уже есть")
                        color: root.textColor
                    }
                    ComboBox {
                        id: conflictMode
                        enabled: !controller.running
                        model: [qsTr("Перезаписать"), qsTr("Добавить счетчик")]
                        Layout.preferredWidth: 220
                    }

                    Label {
                        text: qsTr("Режим запуска")
                        color: root.textColor
                    }
                    ComboBox {
                        id: runMode
                        enabled: !controller.running
                        model: [qsTr("Разовый запуск"), qsTr("По таймеру")]
                        Layout.preferredWidth: 220
                    }

                    Label {
                        text: qsTr("Период опроса, сек")
                        color: root.textColor
                    }
                    TextField {
                        id: pollSeconds
                        enabled: !controller.running && runMode.currentIndex === 1
                        text: "5"
                        leftPadding: 8
                        rightPadding: 8
                        Layout.preferredWidth: 120
                        color: pollSeconds.enabled ? root.textColor : root.disabledTextColor
                        horizontalAlignment: Qt.AlignHCenter
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: 1
                            top: 3600
                        }
                        inputMethodHints: Qt.ImhFormattedNumbersOnly

                        background: Rectangle {
                            color: root.panelColor
                            border.color: root.borderColor
                            radius: 3
                        }
                    }
                }
            }

            GroupBox {
                title: qsTr("XOR ключ")
                Layout.fillWidth: true

                label: Label {
                    text: parent.title
                    color: root.textColor
                    font.bold: true
                }

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 6

                    Label {
                        text: qsTr("8 байт")
                        color: root.textColor
                    }
                    TextField {
                        id: xorKey
                        Layout.fillWidth: true
                        text: "00 00 00 00 00 00 00 00"
                        inputMask: "HH HH HH HH HH HH HH HH;_"
                        color: !enabled ? root.disabledTextColor : acceptableInput ? root.textColor : "#a33b3b"
                        placeholderText: qsTr("Например: 01 02 03 04 05 06 07 08")
                        enabled: !controller.running
                        selectByMouse: true
                        selectionColor: "#c8d0da"
                        selectedTextColor: root.textColor
                        inputMethodHints: Qt.ImhPreferUppercase | Qt.ImhNoPredictiveText
                        background: Rectangle {
                            color: root.panelColor
                            border.color: root.borderColor
                            radius: 3
                        }

                        onTextEdited: text = text.toUpperCase()
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    id: startButton
                    text: qsTr("Старт")
                    enabled: !controller.running
                             && inputFolder.text.length > 0
                             && fileMask.text.length > 0
                             && outputFolder.text.length > 0
                             && xorKey.acceptableInput
                             && (runMode.currentIndex === 0 || pollSeconds.acceptableInput)
                    Layout.preferredWidth: 120
                    contentItem: Text {
                        text: startButton.text
                        color: startButton.enabled ? root.textColor : root.disabledTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#eef1f5"
                        border.color: root.borderColor
                        radius: 3
                    }
                    onClicked: controller.startProcessing(
                        inputFolder.text,
                        fileMask.text,
                        outputFolder.text,
                        deleteInputs.checked,
                        conflictMode.currentIndex,
                        runMode.currentIndex === 1,
                        parseInt(pollSeconds.text, 10),
                        xorKey.text
                    )
                }

                Button {
                    id: stopButton
                    text: qsTr("Стоп")
                    enabled: controller.running
                    Layout.preferredWidth: 120
                    contentItem: Text {
                        text: stopButton.text
                        color: stopButton.enabled ? root.textColor : root.disabledTextColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#eef1f5"
                        border.color: root.borderColor
                        radius: 3
                    }
                    onClicked: controller.stopProcessing()
                }

                ProgressBar {
                    from: 0
                    to: 100
                    value: controller.progress
                    Layout.fillWidth: true
                }

                Label {
                    text: controller.progress + "%"
                    color: root.textColor
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 52
                }
            }

            Frame {
                Layout.fillWidth: true
                Layout.fillHeight: true

                background: Rectangle {
                    color: root.panelColor
                    border.color: root.borderColor
                    radius: 4
                }

                ScrollView {
                    anchors.fill: parent
                    clip: true

                    TextArea {
                        id: statusArea
                        text: controller.statusText
                        color: root.textColor
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                        selectByMouse: true
                        onTextChanged: cursorPosition = length
                        background: Rectangle {
                            color: root.panelColor
                        }
                    }
                }
            }
        }
    }
}
