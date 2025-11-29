import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    property string targetType
    property string targetId

    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Result Details"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestResultDetail(targetType, targetId)
    }

    property string resultTitle: ""
    property string resultScore: ""
    property string resultFeedback: ""
    
    ListModel {
        id: questionModel
    }

    Connections {
        target: networkManager
        function onResultDetailReceived(detailData) {
            // Format: targetId|targetType|title|score|feedback|q1~q2~q3...
            // q format: text^user^correct^status
            
            var parts = detailData.split('|')
            if (parts.length >= 5) {
                resultTitle = parts[2]
                resultScore = parts[3]
                resultFeedback = parts[4]
                
                questionModel.clear()
                
                if (parts.length >= 6) {
                    var qStr = parts[5]
                    var questions = qStr.split('~')
                    for (var i = 0; i < questions.length; i++) {
                        var qParts = questions[i].split('^')
                        if (qParts.length >= 4) {
                            questionModel.append({
                                "questionText": qParts[0],
                                "userAnswer": qParts[1],
                                "correctAnswer": qParts[2],
                                "status": qParts[3]
                            })
                        }
                    }
                }
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Style.margin
        contentWidth: parent.width - 2 * Style.margin

        ColumnLayout {
            width: parent.width
            spacing: 20

            // Summary Card
            Rectangle {
                Layout.fillWidth: true
                height: 150
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text {
                        text: resultTitle
                        font.pixelSize: Style.headerSize
                        font.bold: true
                        color: Style.textColor
                    }

                    RowLayout {
                        spacing: 20
                        Text {
                            text: "Score: " + (resultScore ? parseFloat(resultScore).toFixed(1) : "0.0") + "%"
                            font.pixelSize: Style.subHeaderSize
                            color: Style.primaryColor
                            font.bold: true
                        }
                        
                        Text {
                            text: "Type: " + (targetType === "exam" ? "Exam" : "Exercise")
                            font.pixelSize: Style.bodySize
                            color: "gray"
                        }
                    }

                    Text {
                        text: "Feedback: " + (resultFeedback ? resultFeedback : "No feedback")
                        font.pixelSize: Style.bodySize
                        color: Style.textColor
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }
                }
            }

            Text {
                text: "Questions Review"
                font.pixelSize: Style.subHeaderSize
                font.bold: true
                color: Style.textColor
            }

            // Questions List
            Repeater {
                model: questionModel
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: col.height + 30
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: model.status === "correct" ? "green" : "red"
                    border.width: 1

                    ColumnLayout {
                        id: col
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 15
                        spacing: 8

                        Text {
                            text: "Q" + (index + 1) + ": " + model.questionText
                            font.pixelSize: Style.bodySize
                            font.bold: true
                            color: Style.textColor
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "Your Answer: " + model.userAnswer
                            font.pixelSize: Style.bodySize
                            color: model.status === "correct" ? "green" : "red"
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "Correct Answer: " + model.correctAnswer
                            font.pixelSize: Style.bodySize
                            color: "green"
                            visible: model.status !== "correct"
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
