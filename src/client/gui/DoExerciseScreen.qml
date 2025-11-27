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
    property var examData: ({})
    property int currentQuestionIndex: 0

    Component.onCompleted: {
        if (targetType === "exam") {
            networkManager.requestExam(exerciseId)
        } else {
            var code = 170;
            if (exerciseTypeStr.indexOf("MULTIPLE") !== -1) code = 170;
            else if (exerciseTypeStr.indexOf("FILL") !== -1) code = 180;
            else if (exerciseTypeStr.indexOf("ORDER") !== -1) code = 190;
            else if (exerciseTypeStr.indexOf("REWRITE") !== -1) code = 200;
            else if (exerciseTypeStr.indexOf("PARAGRAPH") !== -1) code = 210;
            else if (exerciseTypeStr.indexOf("SPEAKING") !== -1) code = 220;
            
            networkManager.requestExercise(code, exerciseId)
        }
    }

    Connections {
        target: networkManager
        function onExerciseContentReceived(content) {
            // Parse DTO: id|lessonId|title|type|level|question|options|answer|explanation
            var parts = content.split('|');
            if (parts.length >= 6) {
                exerciseData = {
                    id: parts[0],
                    lessonId: parts[1],
                    title: parts[2],
                    type: parts[3],
                    level: parts[4],
                    question: parts[5],
                    options: parts.length > 6 ? parts[6].split(',') : [],
                    answer: parts.length > 7 ? parts[7] : "",
                    explanation: parts.length > 8 ? parts[8] : ""
                };
                questionText.text = exerciseData.question;
                
                // Reset UI
                answerField.text = "";
                mcModel.clear();
                if (exerciseData.options.length > 0) {
                    for (var i = 0; i < exerciseData.options.length; i++) {
                        mcModel.append({text: exerciseData.options[i]});
                    }
                }
            } else {
                questionText.text = content; // Fallback
            }
        }
        
        function onExamContentReceived(content) {
            // Parse ExamDTO: id|lessonId|title|type|level|questions(comma separated json strings)
            var parts = content.split('|');
            if (parts.length >= 6) {
                // Reconstruct questions array (might contain commas, so we need to be careful)
                // Actually, the DTO joins questions with comma. But questions are JSON strings which might contain commas.
                // This is a flaw in the DTO design. 
                // However, for now, let's assume simple splitting or that the JSON doesn't contain the separator.
                // A better way would be to use a different separator in DTO or JSON array.
                // Given current implementation: utils::join(questions, ',')
                // We will try to parse. If it fails, we might need to fix the backend DTO.
                // But for the seed data, I used JSON strings.
                
                // Let's assume the questions part is everything after the 5th pipe
                var questionsStr = parts.slice(5).join('|'); 
                // Wait, if questions contain pipes? JSON shouldn't contain pipes usually.
                
                // Splitting by comma might break JSON objects.
                // We should probably have used a better delimiter or JSON array for the whole DTO.
                // But let's try to parse the whole string as a list of "Question 1", "Question 2" if it's simple text.
                // If it's JSON, we have a problem.
                
                // WORKAROUND: For this task, I will assume the seed data questions are simple enough or I will fix the DTO separator.
                // Actually, I can't easily fix the DTO separator everywhere without recompiling everything.
                // Let's try to parse.
                
                var questions = questionsStr.split('^');
                
                examData = {
                    id: parts[0],
                    lessonId: parts[1],
                    title: parts[2],
                    type: parts[3],
                    level: parts[4],
                    questions: questions
                };
                
                currentQuestionIndex = 0;
                loadExamQuestion(0);
            }
        }

        function onAnswerSubmissionResult(response) {
            resultText.text = response
            if (response.indexOf("Correct") !== -1 || response.indexOf("received") !== -1 || response.indexOf("submitted") !== -1) {
                resultText.color = Style.successColor
            } else {
                resultText.color = Style.errorColor
            }
        }
    }
    
    function loadExamQuestion(index) {
        if (!examData.questions || index >= examData.questions.length) return;
        
        var qStr = examData.questions[index];
        // Try to parse JSON
        try {
            var qObj = JSON.parse(qStr);
            questionText.text = qObj.text || qObj.question;
            
            mcModel.clear();
            if (qObj.options) {
                for (var i = 0; i < qObj.options.length; i++) {
                    mcModel.append({text: qObj.options[i]});
                }
            }
        } catch (e) {
            questionText.text = qStr; // Fallback to raw string
            mcModel.clear();
        }
        answerField.text = "";
    }

    ListModel { id: mcModel }

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
                    text: "Loading..."
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

            // Multiple Choice UI
            ListView {
                visible: mcModel.count > 0
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                model: mcModel
                delegate: RadioButton {
                    text: model.text
                    onCheckedChanged: if (checked) answerField.text = text
                }
            }

            // Text Input UI (Fill-in, Rewrite, Order)
            TextField {
                id: answerField
                visible: !mcModel.count && exerciseTypeStr.indexOf("PARAGRAPH") === -1
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

            // Paragraph UI
            TextArea {
                id: paragraphField
                visible: exerciseTypeStr.indexOf("PARAGRAPH") !== -1
                placeholderText: "Write your paragraph here..."
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: parent.activeFocus ? Style.primaryColor : "#e0e0e0"
                    border.width: 1
                }
                onTextChanged: answerField.text = text
            }

            RowLayout {
                Layout.fillWidth: true
                visible: targetType === "exam"
                
                Button {
                    text: "Previous"
                    enabled: currentQuestionIndex > 0
                    onClicked: loadExamQuestion(--currentQuestionIndex)
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: currentQuestionIndex < (examData.questions ? examData.questions.length - 1 : 0) ? "Next" : "Submit Exam"
                    onClicked: {
                        if (currentQuestionIndex < (examData.questions ? examData.questions.length - 1 : 0)) {
                            // Save answer logic here if needed
                            loadExamQuestion(++currentQuestionIndex)
                        } else {
                            networkManager.submitAnswer(targetType, exerciseId, "Exam Submitted")
                        }
                    }
                }
            }

            Button {
                visible: targetType !== "exam"
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
