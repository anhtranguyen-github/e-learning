import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: targetType === "exam" ? "Do Exam" : "Do Exercise"
        onBackClicked: stackView.pop()
    }

    property int exerciseId
    property string exerciseTypeStr
    property string targetType: "exercise"
    property var exerciseData: ({ questions: [] })
    property var examData: ({ questions: [] })
    property var preloadedExamData: null
    property int currentQuestionIndex: 0
    
    readonly property int questionCount: {
        if (root.examData && root.examData.questions && Array.isArray(root.examData.questions)) {
            return root.examData.questions.length;
        }
        return 0;
    }

    Component.onCompleted: {
        if (targetType === "exam") {
            if (preloadedExamData) {
                console.log("DoExerciseScreen: Using preloaded exam data")
                root.examData = preloadedExamData
                currentQuestionIndex = 0
                loadQuestion(0)
            } else {
                networkManager.requestExam(exerciseId)
            }
        } else {
            var code = 170;
            var typeUpper = exerciseTypeStr.toUpperCase();
            
            if (typeUpper.indexOf("MULTIPLE") !== -1) code = 170;
            else if (typeUpper.indexOf("FILL") !== -1) code = 180;
            else if (typeUpper.indexOf("ORDER") !== -1) code = 190;
            else if (typeUpper.indexOf("REWRITE") !== -1) code = 200;
            else if (typeUpper.indexOf("PARAGRAPH") !== -1 || typeUpper.indexOf("ESSAY") !== -1) code = 210;
            else if (typeUpper.indexOf("SPEAKING") !== -1) code = 220;
            
            networkManager.requestExercise(code, exerciseId)
        }
    }

    Connections {
        target: networkManager
        function onExerciseContentReceived(content) {
            console.log("DoExerciseScreen: Received exercise content: " + content)
            // Parse DTO: id|lessonId|title|type|level|questions(caret separated json strings)
            var parts = content.split('|');
            if (parts.length >= 6) {
                var questionsStr = parts.slice(5).join('|'); 
                var questions = questionsStr.split('^');
                
                root.exerciseData = {
                    id: parts[0],
                    lessonId: parts[1],
                    title: parts[2],
                    type: parts[3],
                    level: parts[4],
                    questions: questions
                };
                
                // Use same data structure for both
                root.examData = root.exerciseData;
                
                currentQuestionIndex = 0;
                loadQuestion(0);
            }
        }
        
        function onExamContentReceived(content) {
            console.log("DoExerciseScreen: Received exam content: " + content)
            // Parse ExamDTO: id|lessonId|title|type|level|questions(caret separated json strings)
            var parts = content.split('|');
            if (parts.length >= 6) {
                var questionsStr = parts.slice(5).join('|'); 
                var questions = questionsStr.split('^');
                
                root.examData = {
                    id: parts[0],
                    lessonId: parts[1],
                    title: parts[2],
                    type: parts[3],
                    level: parts[4],
                    questions: questions
                };
                
                currentQuestionIndex = 0;
                loadQuestion(0);
            }
        }

        function onAnswerSubmissionSuccess(tType, tId) {
            // Redirect to ResultDetailScreen
            // Note: tType and tId might be empty if we didn't pass them back from C++
            // But we have them in local properties: targetType and exerciseId (which is targetId)
            stackView.replace("ResultDetailScreen.qml", {
                "targetType": targetType,
                "targetId": exerciseId
            })
        }

        function onAnswerSubmissionFailure(message) {
            resultText.text = message
            resultText.color = Style.errorColor
        }

        function onErrorOccurred(message) {
            console.log("DoExerciseScreen: Error occurred: " + message)
            resultText.text = message
            resultText.color = Style.errorColor
            // Optionally disable interaction or go back
        }
    }
    
    property var userAnswers: []

    function saveCurrentAnswer() {
         userAnswers[currentQuestionIndex] = answerField.text
    }

    function checkAllAnswered() {
        if (!examData.questions) return true
        for (var i = 0; i < examData.questions.length; i++) {
            var ans = userAnswers[i]
            if (ans === undefined || ans === null || ans.toString().trim() === "") {
                return false
            }
        }
        return true
    }

    function loadQuestion(index) {
        if (!examData.questions || index >= examData.questions.length) return;
        currentQuestionIndex = index;
        
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
        
        // Restore answer
        if (userAnswers[index] !== undefined) {
            answerField.text = userAnswers[index]
        } else {
            answerField.text = ""
        }
    }

    ListModel { id: mcModel }

        // Question Overview Panel (Drawer)
        Drawer {
            id: overviewDrawer
            width: Math.min(parent.width * 0.8, 300)
            height: parent.height
            edge: Qt.RightEdge
            background: Rectangle { color: Style.backgroundColor }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Style.margin
                spacing: Style.margin

                Label {
                    text: "Questions Overview"
                    font.pixelSize: Style.h3Size
                    font.bold: true
                    color: Style.primaryColor
                    Layout.alignment: Qt.AlignHCenter
                }

                GridView {
                    id: questionGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cellWidth: 60
                    cellHeight: 60
                    clip: true
                    model: root.questionCount
                    
                    delegate: Item {
                        width: questionGrid.cellWidth
                        height: questionGrid.cellHeight
                        
                        Rectangle {
                            anchors.centerIn: parent
                            width: 50
                            height: 50
                            radius: 25
                            color: {
                                if (index === currentQuestionIndex) return Style.primaryColor
                                if (userAnswers[index] !== undefined && userAnswers[index] !== "") return Style.successColor
                                return "#e0e0e0"
                            }
                            
                            Text {
                                anchors.centerIn: parent
                                text: index + 1
                                color: (index === currentQuestionIndex || (userAnswers[index] !== undefined && userAnswers[index] !== "")) ? "white" : Style.textColor
                                font.bold: true
                            }
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    saveCurrentAnswer()
                                    loadQuestion(index)
                                    overviewDrawer.close()
                                }
                            }
                        }
                    }
                }
                
                Button {
                    text: "Submit All"
                    Layout.fillWidth: true
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
                        saveCurrentAnswer()
                        if (!checkAllAnswered()) {
                            resultText.text = "Please answer all questions before submitting."
                            resultText.color = Style.errorColor
                            overviewDrawer.close()
                            return
                        }
                        var finalAnswer = userAnswers.join('^')
                        networkManager.submitAnswer(targetType, exerciseId, finalAnswer)
                        overviewDrawer.close()
                    }
                }
            }
        }

        // Main Content
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Style.margin
            spacing: Style.margin

            // Header with Overview Toggle
            RowLayout {
                Layout.fillWidth: true
                
                Text {
                    text: "Question " + (currentQuestionIndex + 1) + " of " + root.questionCount
                    font.pixelSize: Style.h3Size
                    font.bold: true
                    color: Style.primaryColor
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "Overview"
                    onClicked: overviewDrawer.open()
                    background: Rectangle {
                        color: "transparent"
                        border.color: Style.primaryColor
                        border.width: 1
                        radius: Style.cornerRadius
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Style.primaryColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

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
                spacing: 15

                // Multiple Choice UI
                ListView {
                    visible: mcModel.count > 0
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentHeight
                    model: mcModel
                    delegate: RadioButton {
                        width: ListView.view.width
                        text: model.text
                        font.pixelSize: Style.bodySize
                        onCheckedChanged: if (checked) answerField.text = text
                        // Auto-select if answer matches
                        Component.onCompleted: {
                            if (answerField.text === text) checked = true;
                        }
                        Connections {
                            target: answerField
                            function onTextChanged() {
                                if (answerField.text === text) checked = true;
                            }
                        }
                    }
                }

                // Text Input UI (Fill-in, Rewrite, Order)
                TextField {
                    id: answerField
                    visible: !mcModel.count && 
                             exerciseTypeStr.toUpperCase().indexOf("PARAGRAPH") === -1 && 
                             exerciseTypeStr.toUpperCase().indexOf("ESSAY") === -1 &&
                             exerciseTypeStr.toUpperCase().indexOf("SPEAKING") === -1
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
                    visible: exerciseTypeStr.toUpperCase().indexOf("PARAGRAPH") !== -1 || 
                             exerciseTypeStr.toUpperCase().indexOf("ESSAY") !== -1
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
                    // Sync back from answerField
                    Connections {
                        target: answerField
                        function onTextChanged() {
                            if (paragraphField.visible && paragraphField.text !== answerField.text) {
                                paragraphField.text = answerField.text
                            }
                        }
                    }
                }

                // Speaking UI (Placeholder)
                ColumnLayout {
                    visible: exerciseTypeStr.toUpperCase().indexOf("SPEAKING") !== -1
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: 70
                        height: 70
                        radius: 35
                        color: isRecording ? "#ff4444" : "#e0e0e0"
                        
                        property bool isRecording: false
                        
                        Text {
                            anchors.centerIn: parent
                            text: parent.isRecording ? "STOP" : "REC"
                            color: parent.isRecording ? "white" : "#333333"
                            font.bold: true
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                parent.isRecording = !parent.isRecording
                                if (!parent.isRecording) {
                                    answerField.text = "[Audio Recording Submitted]"
                                }
                            }
                        }
                    }
                    
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: parent.children[0].isRecording ? "Recording..." : "Tap to record"
                        color: Style.secondaryTextColor
                        font.pixelSize: Style.smallSize
                    }
                }

                // Navigation Buttons (Simplified)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 20
                    
                    Button {
                        text: "Previous"
                        enabled: currentQuestionIndex > 0
                        Layout.preferredWidth: 120
                        background: Rectangle {
                            color: parent.enabled ? Style.cardBackground : "#f0f0f0"
                            border.color: parent.enabled ? Style.primaryColor : "#e0e0e0"
                            border.width: 1
                            radius: Style.cornerRadius
                        }
                        contentItem: Text {
                            text: parent.text
                            color: parent.enabled ? Style.primaryColor : "#999999"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                        onClicked: {
                            saveCurrentAnswer()
                            loadQuestion(--currentQuestionIndex)
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Button {
                        text: currentQuestionIndex < (examData.questions ? examData.questions.length - 1 : 0) ? "Next" : "Submit"
                        Layout.preferredWidth: 120
                        flat: true
                        background: Rectangle {
                            color: parent.enabled ? Style.primaryColor : "#f0f0f0"
                            radius: Style.cornerRadius
                        }
                        contentItem: Text {
                            text: parent.text
                            color: parent.enabled ? "white" : "#999999"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.bold: true
                        }
                        onClicked: {
                            saveCurrentAnswer()
                            if (currentQuestionIndex < (examData.questions ? examData.questions.length - 1 : 0)) {
                                loadQuestion(++currentQuestionIndex)
                            } else {
                                if (!checkAllAnswered()) {
                                    resultText.text = "Please answer all questions before submitting."
                                    resultText.color = Style.errorColor
                                    return
                                }
                                // Join answers with caret delimiter
                                var finalAnswer = userAnswers.join('^')
                                networkManager.submitAnswer(targetType, exerciseId, finalAnswer)
                            }
                        }
                    }
                }

                Text {
                    id: resultText
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: Style.bodySize
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    visible: text !== ""
                }
            }
        }
}
