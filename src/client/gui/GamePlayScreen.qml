import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }
    
    property string gameId: ""
    property var gameData: null
    
    // Game State
    property int currentQuestionIndex: 0
    property int score: 0
    property var userAnswers: []
    property bool isGameFinished: false

    Component.onCompleted: {
        networkManager.requestGameData(gameId)
    }

    Connections {
        target: networkManager
        
        function onGameDataReceived(data) {
            // Format: id|type|level|questionJson
            var parts = data.split("|")
            if (parts.length >= 4) {
                // Reassemble JSON if it contained pipes (unlikely but possible in content)
                // Actually utility join uses |, so splitting strictly might break json.
                // But DTO serialize uses |, assuming fields don't contain it.
                // JSON definitely contains special chars.
                // Wait, `questionJson` is the last field. So we can join the rest.
                var jsonStr = parts.slice(3).join("|")
                try {
                    var parsedData = JSON.parse(jsonStr)
                    gameData = {
                        id: parts[0],
                        type: parts[1],
                        level: parts[2],
                        questions: parsedData
                    }
                    console.log("Game Data Loaded:", gameData.type, gameData.questions.length)
                } catch (e) {
                    console.error("Failed to parse game JSON:", e)
                }
            }
        }
        
        function onGameSubmitSuccess(message) {
            console.log("Game submitted successfully")
            // Show success dialog or navigate back
            finishDialog.open()
        }
        
        function onGameSubmitFailure(message) {
            console.error("Game submission failed:", message)
        }
    }
    
    // --- UI Helper Components ---
    
    Dialog {
        id: finishDialog
        width: 300
        anchors.centerIn: parent
        title: "Game Completed"
        modal: true
        standardButtons: Dialog.Ok
        
        Text {
            text: "Congratulations! You scored " + score + " points."
        }
        
        onAccepted: {
            stackView.pop()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        
        visible: gameData !== null && !isGameFinished

        // Header (Score & Progress)
        RowLayout {
            Layout.fillWidth: true
            
            Text {
                text: gameData ? "Level: " + gameData.level : ""
                font.bold: true
                color: Style.secondaryColor
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: "Score: " + score
                font.bold: true
                font.pixelSize: Style.subHeadingSize
                color: Style.primaryColor
            }
        }

        // Progress Bar
        ProgressBar {
            Layout.fillWidth: true
            from: 0
            to: gameData ? gameData.questions.length : 1
            value: currentQuestionIndex
        }

        // Game Content Area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"

            // Loader to switch between game types
            Loader {
                anchors.fill: parent
                anchors.margins: 20
                sourceComponent: {
                    if (!gameData) return null
                    if (gameData.type === "sentence_match") return sentenceMatchComponent
                    if (gameData.type === "word_match") return wordMatchComponent
                    if (gameData.type === "image_match") return imageMatchComponent
                    return null
                }
            }
        }

        // Navigation Buttons
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: "Quit"
                onClicked: stackView.pop()
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: currentQuestionIndex < (gameData ? gameData.questions.length - 1 : 0) ? "Next" : "Finish"
                highlighted: true
                enabled: canProceed()
                onClicked: {
                    if (currentQuestionIndex < gameData.questions.length - 1) {
                        nextQuestion()
                    } else {
                        submitGame()
                    }
                }
            }
        }
    }
    
    // --- Game Logic ---
    
    function canProceed() {
        // Validation logic based on game type
        return true // detailed validation later
    }
    
    function nextQuestion() {
        // Save current answer logic
        currentQuestionIndex++
    }
    
    function submitGame() {
        isGameFinished = true
        var details = JSON.stringify(userAnswers)
        networkManager.submitGameResult(gameId, score.toString(), details)
    }

    // --- Game Components ---

    Component {
        id: sentenceMatchComponent
        ColumnLayout {
            property var question: gameData ? gameData.questions[currentQuestionIndex] : null
            
            Text {
                text: "Arrange the words to form a correct sentence:"
                font.pixelSize: Style.subHeadingSize
                Layout.alignment: Qt.AlignHCenter
                font.bold: true
                color: Style.primaryColor
            }
            
            // Display shuffled fragments or just "Words: ..."
            // Ideally we'd have draggable items, but for simplicity in QML GridView/Flow without drag-drop lib:
            // We can show clickable words to build sentence.
            
            property string currentInput: ""
            
            Flow {
                Layout.fillWidth: true
                spacing: 10
                
                Repeater {
                   model: question ? question.sentence_parts : [] 
                   Button {
                       text: modelData
                       onClicked: {
                           if (currentInput.length > 0) currentInput += " "
                           currentInput += modelData
                       }
                   }
                }
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 100
                color: "#f8f9fa"
                border.color: "#333"
                radius: 5
                
                Text {
                    anchors.centerIn: parent
                    text: currentInput.length > 0 ? currentInput : "Click words above to build sentence"
                    font.italic: currentInput.length === 0
                    width: parent.width - 20
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignRight
                Button {
                    text: "Clear"
                    onClicked: currentInput = ""
                }
                Button {
                    text: "Submit Answer"
                    enabled: currentInput.length > 0
                    highlighted: true
                    onClicked: {
                        // Validate
                        var isCorrect = (currentInput === question.correct_sentence)
                        if (isCorrect) {
                            score += 10
                        }
                        userAnswers.push({
                            question: question.correct_sentence,
                            userResponse: currentInput,
                            correct: isCorrect
                        })
                        
                        // Move to next
                        if (currentQuestionIndex < gameData.questions.length - 1) {
                            nextQuestion()
                            currentInput = ""
                        } else {
                            submitGame()
                        }
                    }
                }
            }
            
            // Answer Key (hidden/debug)
            // Text { text: "Debug: " + (question ? question.correct_sentence : "") }
        }
    }

    Component {
        id: wordMatchComponent
        ColumnLayout {
            property var questionPairs: gameData ? gameData.questions : []
            // Actually word_match question structure in seed is:
            // [ {"word_pair": ["A", "B"]}, ... ]
            // This is one huge list of pairs? NO, seed inserts a LIST of questions.
            // Each question item in the array is ONE pair? 
            // Seed: `[ {"word_pair": ["A", "B"]}, ... ]`.
            // So currentQuestionIndex iterates over PAIRS?
            // "Word Match" usually implies matching ALL pairs on screen.
            // But if `currentQuestionIndex` logic is used, we do one pair at a time?
            // That's "Flashcard" style. 
            // "Word Match" game usually means a grid of mixed words.
            // Let's implement it as "Mix and Match" for the WHOLE set if possible.
            // But `gameData` is parsed as `questions` array.
            // Let's treat the whole `questions` array as ONE game level content if it's small.
            // BUT `currentQuestionIndex` logic suggests step-by-step.
            // Let's adapt: "Select the specific definition for this word".
            // Since the seed data is `word_pair: [Word, Meaning]`.
            // We can show "Word" and 4 options (1 correct, 3 random from other pairs).
            
            property var currentPair: questionPairs[currentQuestionIndex]
            property var options: []
            
            Component.onCompleted: generateOptions()
            
            Connections {
                target: root
                function onCurrentQuestionIndexChanged() {
                    generateOptions()
                }
            }

            function generateOptions() {
                if (!currentPair) return
                var correct = currentPair.word_pair[1]
                var opts = [correct]
                // Pick 3 random wrong answers from other pairs
                for (var i = 0; i < 3; i++) {
                    var rnd = Math.floor(Math.random() * questionPairs.length)
                    var wrong = questionPairs[rnd].word_pair[1]
                    if (opts.indexOf(wrong) === -1 && wrong !== correct) {
                        opts.push(wrong)
                    }
                }
                // Shuffle
                for (let i = opts.length - 1; i > 0; i--) {
                    const j = Math.floor(Math.random() * (i + 1));
                    [opts[i], opts[j]] = [opts[j], opts[i]];
                }
                options = opts
            }

            Text {
                text: "Select the correct meaning for:"
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: Style.subHeadingSize
            }
            
            Text {
                text: currentPair ? currentPair.word_pair[0] : ""
                font.bold: true
                font.pixelSize: Style.h1Size
                color: Style.primaryColor
                Layout.alignment: Qt.AlignHCenter
            }
            
            GridView {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                cellWidth: width / 2
                cellHeight: 60
                model: options
                delegate: Button {
                    text: modelData
                    width: GridView.view.cellWidth - 10
                    height: 50
                    onClicked: {
                        var correct = currentPair.word_pair[1]
                        var isCorrect = (modelData === correct)
                        if (isCorrect) score += 10
                        
                        userAnswers.push({
                            word: currentPair.word_pair[0],
                            userResponse: modelData,
                            correct: isCorrect
                        })
                        
                        if (currentQuestionIndex < questionPairs.length - 1) {
                            nextQuestion()
                        } else {
                            submitGame()
                        }
                    }
                }
            }
        }
    }
    
    Component {
        id: imageMatchComponent
        ColumnLayout {
            property var question: gameData ? gameData.questions[currentQuestionIndex] : null
            
            Text {
                text: "Identify the image:"
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: Style.subHeadingSize
            }

            Image {
                source: question ? question.image_url : "" 
                Layout.maximumHeight: 250
                Layout.maximumWidth: 400
                Layout.preferredHeight: 250
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                fillMode: Image.PreserveAspectFit
                
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: "#ccc"
                    visible: parent.status !== Image.Ready
                    Text { anchors.centerIn: parent; text: "Loading Image..." }
                }
            }
             
            GridView {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
                cellWidth: width / 2
                cellHeight: 60
                model: question ? question.options : []
                delegate: Button {
                    text: modelData
                    width: GridView.view.cellWidth - 10
                    height: 50
                    onClicked: {
                         var isCorrect = (modelData === question.word)
                         if (isCorrect) {
                             score += 10
                         }
                         userAnswers.push({
                            image: question.word, // Using word as ID
                            userResponse: modelData,
                            correct: isCorrect
                         })
                         
                         if (currentQuestionIndex < gameData.questions.length - 1) {
                            nextQuestion()
                         } else {
                            submitGame()
                         }
                    }
                }
            }
        }
    }
}
