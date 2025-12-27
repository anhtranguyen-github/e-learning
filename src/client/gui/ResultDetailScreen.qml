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
    ListModel {
        id: attemptModel
    }

    Connections {
        target: networkManager
        function onResultDetailReceived(detailData) {
            // Format: targetId|targetType|title|resultId|score|feedback|questions;resultId|score|feedback;...
            // question format: text^user^correct^status^score^comment
            questionModel.clear()
            attemptModel.clear()

            var chunks = detailData.split(';')
            if (chunks.length < 1) {
                return
            }

            var headerParts = chunks[0].split('|')
            if (headerParts.length >= 6) {
                resultTitle = headerParts[2]
                resultScore = headerParts[4]
                resultFeedback = headerParts[5]

                attemptModel.append({
                    "resultId": headerParts[3],
                    "score": headerParts[4],
                    "feedback": headerParts[5]
                })

                if (headerParts.length >= 7 && headerParts[6] !== "") {
                    var questions = headerParts[6].split('~')
                    for (var i = 0; i < questions.length; i++) {
                        var qParts = questions[i].split('^')
                        if (qParts.length >= 4) {
                            questionModel.append({
                                "questionText": qParts[0],
                                "userAnswer": qParts[1],
                                "correctAnswer": qParts[2],
                                "status": qParts[3],
                                "questionScore": qParts.length >= 5 ? qParts[4] : "",
                                "teacherComment": qParts.length >= 6 ? qParts[5] : ""
                            })
                        }
                    }
                }
            }

            for (var j = 1; j < chunks.length; j++) {
                if (chunks[j] === "") continue
                var attemptParts = chunks[j].split('|')
                if (attemptParts.length >= 3) {
                    attemptModel.append({
                        "resultId": attemptParts[0],
                        "score": attemptParts[1],
                        "feedback": attemptParts[2]
                    })
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
                        font.pixelSize: Style.headingSize
                        font.bold: true
                        color: Style.textColor
                    }

                    RowLayout {
                        spacing: 20
                        Text {
                            text: "Score: " + (resultScore ? parseFloat(resultScore).toFixed(1) : "0.0") + "%"
                            font.pixelSize: Style.subHeadingSize
                            color: Style.primaryColor
                            font.bold: true
                        }
                        
                        Text {
                            text: "Type: " + (targetType === "exam" ? "Exam" : "Exercise")
                            font.pixelSize: Style.smallSize
                            color: Style.secondaryTextColor
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
                text: "All Attempts"
                font.pixelSize: Style.subHeadingSize
                font.bold: true
                color: Style.textColor
                visible: attemptModel.count > 0
            }

            Repeater {
                model: attemptModel
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: attemptCol.height + 24
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: "#e0e0e0"
                    border.width: 1

                    ColumnLayout {
                        id: attemptCol
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 6

                        Text {
                            text: "Result ID: " + model.resultId
                            font.pixelSize: Style.smallSize
                            color: Style.secondaryTextColor
                        }

                        Text {
                            text: "Score: " + (model.score ? parseFloat(model.score).toFixed(1) : "0.0") + "%"
                            font.pixelSize: Style.bodySize
                            color: Style.primaryColor
                            font.bold: true
                        }

                        Text {
                            text: "Feedback: " + (model.feedback ? model.feedback : "No feedback")
                            font.pixelSize: Style.bodySize
                            color: Style.textColor
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            Text {
                text: "Questions Review"
                font.pixelSize: Style.subHeadingSize
                font.bold: true
                color: Style.textColor
                visible: questionModel.count > 0
            }

            // Questions List
            Repeater {
                model: questionModel
                delegate: Rectangle {
                    Layout.fillWidth: true
                    height: col.height + 30
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: model.status === "correct" ? Style.successColor : Style.errorColor
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
                            color: model.status === "correct" ? Style.successColor : Style.errorColor
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "Correct Answer: " + model.correctAnswer
                            font.pixelSize: Style.bodySize
                            color: Style.successColor
                            visible: model.status !== "correct"
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }

                        // Teacher Score (if available)
                        Text {
                            text: "Score: " + model.questionScore + "/10"
                            font.pixelSize: Style.bodySize
                            color: Style.primaryColor
                            font.bold: true
                            visible: model.questionScore !== ""
                        }

                        // Teacher Comment (if available)
                        Text {
                            text: "Teacher Comment: " + model.teacherComment
                            font.pixelSize: Style.smallSize
                            font.italic: true
                            color: Style.secondaryTextColor
                            visible: model.teacherComment !== ""
                            wrapMode: Text.Wrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
