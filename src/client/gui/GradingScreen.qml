import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 5.15
import "."

Page {
    property string resultId
    property string userName
    property string studentId
    property string targetType
    property string targetTitle
    property string userAnswer
    property string targetId
    property string lessonId

    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Grading: " + userName
        onBackClicked: stackView.pop()
    }

    property var gradingDetails: ({})

    ListModel { id: questionModel }

    Component.onCompleted: {
        console.log("GradingScreen loaded. Type:", targetType, "ID:", targetId)
        var tId = parseInt(targetId)
        if (isNaN(tId)) {
            console.warn("Invalid targetId:", targetId)
            return
        }

        if (targetType === "exercise") {
            networkManager.requestExercise(160, tId)
        } else if (targetType === "exam") {
            networkManager.requestExamReview(tId)  // Teacher uses review endpoint
        }
    }

    Connections {
        target: networkManager
        function onExerciseContentReceived(content) { parseContent(content) }
        function onExamContentReceived(content) { parseContent(content) }
        function onGradeSubmissionSuccess(msg) { stackView.pop() }
    }

    function parseContent(content) {
        questionModel.clear()
        
        var parts = content.split('|')
        if (parts.length < 6) {
            console.warn("Invalid content format received in GradingScreen")
            return
        }

        var questionsStr = parts.slice(5).join('|')
        var questionsList = questionsStr.split('^')
        var userAnswers = userAnswer.split("^")
        
        for (var i = 0; i < questionsList.length; i++) {
            var qStr = questionsList[i]
            var uAns = userAnswers[i] || "No answer"
            
            try {
                var qObj = JSON.parse(qStr)
                var qText = qObj.text || qObj.question || "Question " + (i+1)
                var qType = qObj.type || "unknown"
                var audioPath = ""
                if (qType.toLowerCase().indexOf("speaking") !== -1 && uAns !== "") {
                    audioPath = networkManager.saveAudioFromBase64(uAns)
                    if (audioPath !== "") {
                        uAns = "Audio recording"
                    }
                }
                
                questionModel.append({
                    "index": i,
                    "questionText": qText,
                    "type": qType,
                    "userAnswer": uAns,
                    "correctAnswer": qObj.answer || qObj.correctAnswer || "N/A",
                    "score": "0",
                    "comment": "",
                    "audioPath": audioPath
                })
            } catch (e) {
                console.log("Error parsing question JSON:", e)
                questionModel.append({
                    "index": i,
                    "questionText": qStr, // Fallback to raw string
                    "type": "unknown",
                    "userAnswer": uAns,
                    "score": "0",
                    "comment": "",
                    "audioPath": ""
                })
            }
        }
    }

    ListView {
        id: gradingList
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: 20
        clip: true
        model: questionModel

        delegate: Rectangle {
            width: gradingList.width
            height: contentCol.height + 30
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"

                ColumnLayout {
                    id: contentCol
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 15
                    spacing: 10

                    Text {
                        text: "Student ID: " + studentId + " | Lesson ID: " + lessonId
                        font.pixelSize: Style.smallSize
                        color: Style.secondaryTextColor
                        visible: studentId !== "" || lessonId !== ""
                    }

                // Question Header
                Text {
                    text: "Q" + (model.index + 1) + ": " + model.type
                    font.bold: true
                    color: Style.primaryColor
                }

                // Question Text
                Text {
                    text: model.questionText
                    font.pixelSize: Style.bodySize
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }

                Rectangle { height: 1; Layout.fillWidth: true; color: "#e0e0e0" }

                // User Answer
                Text {
                    text: "Student Answer:"
                    font.bold: true
                    font.pixelSize: Style.smallSize
                    color: "gray"
                }
                Text {
                    text: model.userAnswer
                    font.pixelSize: Style.bodySize
                    font.italic: true
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: Style.textColor
                }

                RowLayout {
                    visible: model.audioPath && model.audioPath !== ""
                    spacing: 10

                    Audio {
                        id: audioPlayer
                        source: model.audioPath === "" ? "" : ("file://" + model.audioPath)
                    }

                    Button {
                        text: audioPlayer.playbackState === Audio.PlayingState ? "Stop Audio" : "Play Audio"
                        onClicked: {
                            if (audioPlayer.playbackState === Audio.PlayingState) {
                                audioPlayer.stop()
                            } else {
                                audioPlayer.play()
                            }
                        }
                    }
                }

                Rectangle { height: 1; Layout.fillWidth: true; color: "#e0e0e0" }

                 // Correct Answer
                Text {
                    text: "Correct Answer:"
                    font.bold: true
                    font.pixelSize: Style.smallSize
                    color: Style.successColor // Greenish
                }
                Text {
                    text: model.correctAnswer
                    font.pixelSize: Style.bodySize
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: Style.textColor
                }

                Rectangle { height: 1; Layout.fillWidth: true; color: "#e0e0e0" }

                // Grading Inputs
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Text { text: "Score (0-10):"; verticalAlignment: Text.AlignVCenter }
                    TextField {
                        id: scoreField
                        Layout.preferredWidth: 80
                        text: model.score
                        validator: DoubleValidator { bottom: 0; top: 10; decimals: 1 }
                        onEditingFinished: {
                            model.score = text
                        }
                    }

                    Text { text: "Comment:"; verticalAlignment: Text.AlignVCenter }
                    TextField {
                        id: commentField
                        Layout.fillWidth: true
                        placeholderText: "Specific feedback..."
                        text: model.comment
                        onEditingFinished: {
                            model.comment = text
                        }
                    }
                }
            }
        }

        footer: ColumnLayout {
            width: gradingList.width
            spacing: 15
            
            // Spacer
            Item { Layout.preferredHeight: 20 }

            Rectangle {
                Layout.fillWidth: true
                height: footerCol.height + 30
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: Style.primaryColor
                
                ColumnLayout {
                    id: footerCol
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 10
                    
                    Text {
                        text: "Overall Feedback & Grade"
                        font.bold: true
                        font.pixelSize: Style.subHeadingSize
                        color: Style.primaryColor
                    }
                    
                    TextArea {
                        id: overallFeedbackInput
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        placeholderText: "Enter overall feedback for this submission..."
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
                    color: parent.down ? Style.primaryDarkColor : Style.primaryColor
                    radius: Style.cornerRadius
                }

                onClicked: submitGrading(overallFeedbackInput.text)
            }
            
            // Bottom spacer
            Item { Layout.preferredHeight: 50 }
        }
    }

    function submitGrading(overallFeedback) {
        var totalScore = 0
        var detailsArray = []
        var maxTotal = 0

        for (var i = 0; i < questionModel.count; i++) {
            var item = questionModel.get(i)
            var s = parseFloat(item.score) || 0
            totalScore += s
            maxTotal += 10 // Assuming 10 per question
            
            detailsArray.push({
                "question": item.questionText,
                "userAnswer": item.userAnswer,
                "score": s,
                "comment": item.comment
            })
        }

        // Normalize total score to 0-100 scale
        var normalizedScore = (maxTotal > 0) ? (totalScore / maxTotal) * 100 : 0
        normalizedScore = Math.round(normalizedScore * 10) / 10

        var gradingDetailsObj = {
            "items": detailsArray,
            "overallFeedback": overallFeedback
        }
        
        var detailsJson = JSON.stringify(gradingDetailsObj)
        
        console.log("Submitting:", normalizedScore, overallFeedback, detailsJson)
        
        networkManager.submitGrade(resultId, normalizedScore.toString(), overallFeedback, detailsJson)
    }
}
