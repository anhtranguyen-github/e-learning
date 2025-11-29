import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    property string resultId
    property string userName
    property string targetType
    property string targetTitle
    property string userAnswer

    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Grading: " + userName
        onBackClicked: stackView.pop()
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Style.margin
        contentWidth: parent.width - 2 * Style.margin

        ColumnLayout {
            width: parent.width
            spacing: 20

            // Submission Info
            Rectangle {
                Layout.fillWidth: true
                height: infoCol.height + 20
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    id: infoCol
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Text {
                        text: "Task: " + targetTitle
                        font.bold: true
                        font.pixelSize: Style.subHeaderSize
                    }
                    Text {
                        text: "Type: " + targetType
                        font.pixelSize: Style.bodySize
                    }
                }
            }

            // User Answer
            Rectangle {
                Layout.fillWidth: true
                height: answerCol.height + 20
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    id: answerCol
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Text {
                        text: "User Answer:"
                        font.bold: true
                        font.pixelSize: Style.subHeaderSize
                    }
                    Text {
                        text: userAnswer
                        font.pixelSize: Style.bodySize
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }

            // Grading Inputs
            Rectangle {
                Layout.fillWidth: true
                height: gradingCol.height + 20
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    id: gradingCol
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Text {
                        text: "Score (0-100):"
                        font.bold: true
                    }
                    TextField {
                        id: scoreInput
                        Layout.fillWidth: true
                        placeholderText: "Enter score"
                        validator: DoubleValidator { bottom: 0; top: 100; decimals: 1 }
                    }

                    Text {
                        text: "Feedback:"
                        font.bold: true
                    }
                    TextArea {
                        id: feedbackInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        placeholderText: "Enter feedback"
                        background: Rectangle {
                            border.color: "#bdbdbd"
                            radius: 4
                        }
                    }
                }
            }

            Button {
                text: "Submit Grade"
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                background: Rectangle {
                    color: parent.down ? "#2e59d9" : "#4e73df"
                    radius: Style.cornerRadius
                }

                onClicked: {
                    if (scoreInput.text === "") {
                        // Show error
                        return
                    }
                    networkManager.submitGrade(resultId, scoreInput.text, feedbackInput.text)
                    stackView.pop() // Return to list
                }
            }
        }
    }
}
