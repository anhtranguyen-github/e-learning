import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: targetType === "exam" ? "Do Exam" : "Do Exercise"
        onBackClicked: stackView.pop()
    }

    property int exerciseId
    property string exerciseTypeStr
    property string targetType: "exercise"

    function getTypeCode(typeStr) {
        switch(typeStr) {
            case "MULTIPLE_CHOICE": return 170;
            case "FILL_IN_BLANK": return 180;
            case "SENTENCE_ORDER": return 190;
            case "REWRITE_SENTENCE": return 200;
            case "WRITE_PARAGRAPH": return 210;
            case "SPEAKING_TOPIC": return 220;
            default: return 170;
        }
    }

    Component.onCompleted: {
        var code = 170;
        if (exerciseTypeStr.indexOf("MULTIPLE") !== -1) code = 170;
        else if (exerciseTypeStr.indexOf("FILL") !== -1) code = 180;
        else if (exerciseTypeStr.indexOf("ORDER") !== -1) code = 190;
        else if (exerciseTypeStr.indexOf("REWRITE") !== -1) code = 200;
        else if (exerciseTypeStr.indexOf("PARAGRAPH") !== -1) code = 210;
        else if (exerciseTypeStr.indexOf("SPEAKING") !== -1) code = 220;
        
        networkManager.requestExercise(code, exerciseId)
    }

    Connections {
        target: networkManager
        function onExerciseContentReceived(content) {
            questionText.text = content
        }
        function onAnswerSubmissionResult(response) {
            resultText.text = response
            resultText.color = response.indexOf("Correct") !== -1 ? Style.successColor : Style.errorColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: Style.margin

        // Question Area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: Style.margin
                
                TextArea {
                    id: questionText
                    readOnly: true
                    wrapMode: Text.WordWrap
                    text: "Loading question..."
                    font.family: Style.fontFamily
                    font.pixelSize: Style.bodySize
                    color: Style.textColor
                    background: null
                }
            }
        }

        // Answer Area
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 10

            TextField {
                id: answerField
                placeholderText: "Type your answer here..."
                Layout.fillWidth: true
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: answerField.activeFocus ? Style.primaryColor : "#e0e0e0"
                    border.width: 1
                }
                padding: 12
            }

            Button {
                text: "Submit Answer"
                Layout.fillWidth: true
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.primaryColor
                    radius: Style.cornerRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    networkManager.submitAnswer(targetType, exerciseId, answerField.text)
                }
            }

            Text {
                id: resultText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font.pixelSize: Style.bodySize
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
