import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }

    header: Header {
        title: "Settings"
        showBack: true
        onBackClicked: stackView.pop()
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Style.margin
        clip: true

        ColumnLayout {
            width: Math.min(600, parent.width)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            // User Level Section
            GroupBox {
                title: "User Level"
                Layout.fillWidth: true
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: "#e0e0e0"
                    border.width: 1
                }
                label: Text {
                    text: parent.title
                    font.family: Style.fontFamily
                    font.pixelSize: Style.bodySize
                    font.bold: true
                    color: Style.primaryColor
                    padding: 10
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    Text {
                        text: "Select your proficiency level to adjust lesson difficulity:"
                        font.family: Style.fontFamily
                        font.pixelSize: Style.smallSize
                        color: Style.secondaryTextColor
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        id: levelCombo
                        Layout.fillWidth: true
                        model: ["Beginner", "Intermediate", "Advanced"]
                        font.pixelSize: Style.bodySize
                        
                        // Two-way binding with UserPreferences singleton
                        currentIndex: UserPreferences.userLevel
                        onCurrentIndexChanged: UserPreferences.userLevel = currentIndex
                    }
                }
            }

            // Voice Folder Section
            GroupBox {
                title: "Voice Message Storage"
                Layout.fillWidth: true
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: "#e0e0e0"
                    border.width: 1
                }
                label: Text {
                    text: parent.title
                    font.family: Style.fontFamily
                    font.pixelSize: Style.bodySize
                    font.bold: true
                    color: Style.primaryColor
                    padding: 10
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    Text {
                        text: "Location to save and retrieve voice messages (enter manually):"
                        font.family: Style.fontFamily
                        font.pixelSize: Style.smallSize
                        color: Style.secondaryTextColor
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: pathField
                            Layout.fillWidth: true
                            placeholderText: "/path/to/folder"
                            readOnly: false
                            selectByMouse: true
                            font.pixelSize: Style.bodySize
                            background: Rectangle {
                                color: "#f0f2f5"
                                radius: Style.cornerRadius
                                border.color: pathField.activeFocus ? Style.primaryColor : "transparent"
                                border.width: 2
                            }
                            // Two-way binding with UserPreferences singleton
                            text: UserPreferences.voiceFolderPath
                            onTextChanged: UserPreferences.voiceFolderPath = text
                        }
                    }
                }
            }
        }
    }
}
