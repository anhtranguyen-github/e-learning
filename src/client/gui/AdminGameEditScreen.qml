import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }

    // Properties passed from AdminGameScreen
    property string mode: "create"  // "create", "edit", "view"
    property string gameId: ""
    property string gameType: "sentence_match"
    property string gameLevel: "beginner"

    ListModel { id: itemsModel }

    Component.onCompleted: {
        // Set combo boxes
        for (var i = 0; i < typeCombo.model.length; i++) {
            if (typeCombo.model[i] === gameType) typeCombo.currentIndex = i
        }
        for (var j = 0; j < levelCombo.model.length; j++) {
            if (levelCombo.model[j].toLowerCase() === gameLevel.toLowerCase()) levelCombo.currentIndex = j
        }

        // If edit or view mode, load game data
        if (mode !== "create" && gameId !== "") {
            networkManager.requestGameData(gameId)
        }
    }

    Connections {
        target: networkManager

        function onGameDataReceived(data) {
            // Parse GameContentDTO: id|type|level|json
            var parts = data.split("|")
            if (parts.length >= 4) {
                var id = parts[0]
                var type = parts[1]
                var level = parts[2]
                var json = parts.slice(3).join("|")

                // Update combos
                for (var i = 0; i < typeCombo.model.length; i++) {
                    if (typeCombo.model[i] === type) typeCombo.currentIndex = i
                }
                for (var j = 0; j < levelCombo.model.length; j++) {
                    if (levelCombo.model[j].toLowerCase() === level.toLowerCase()) levelCombo.currentIndex = j
                }

                // Parse JSON into itemsModel
                itemsModel.clear()
                try {
                    var arr = JSON.parse(json)
                    if (Array.isArray(arr)) {
                        for (var k = 0; k < arr.length; k++) {
                            itemsModel.append({ 
                                "question": arr[k].question || arr[k].text || "", 
                                "answer": arr[k].answer || arr[k].correct || "" 
                            })
                        }
                    }
                } catch (e) {
                    console.log("Error parsing game JSON: " + e)
                }
            }
        }

        function onGameCreateSuccess(message) {
            statusMessage.text = "Game created successfully!"
            statusMessage.color = Style.successColor
            successTimer.start()
        }

        function onGameCreateFailure(message) {
            statusMessage.text = "Error: " + message
            statusMessage.color = Style.errorColor
        }

        function onGameUpdateSuccess(message) {
            statusMessage.text = "Game updated successfully!"
            statusMessage.color = Style.successColor
            successTimer.start()
        }

        function onGameUpdateFailure(message) {
            statusMessage.text = "Error: " + message
            statusMessage.color = Style.errorColor
        }
    }

    Timer {
        id: successTimer
        interval: 1500
        onTriggered: stackView.pop()
    }

    function generateJson() {
        var arr = []
        for (var i = 0; i < itemsModel.count; i++) {
            var item = itemsModel.get(i)
            arr.push({ "question": item.question, "answer": item.answer })
        }
        return JSON.stringify(arr)
    }

    header: Header {
        title: mode === "create" ? "Create Game" : (mode === "edit" ? "Edit Game" : "View Game")
        onBackClicked: stackView.pop()
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Style.margin
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: Style.margin

            // Status Message
            Text {
                id: statusMessage
                text: ""
                font.pixelSize: Style.bodySize
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                visible: text !== ""
            }

            // Main Card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: mainContent.implicitHeight + 40
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    id: mainContent
                    anchors.fill: parent
                    anchors.margins: Style.margin
                    spacing: 20

                    // Game ID (for edit/view)
                    RowLayout {
                        visible: mode !== "create"
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            text: "Game ID:"
                            font.bold: true
                            color: Style.secondaryTextColor
                        }
                        Text {
                            text: gameId
                            font.family: "monospace"
                            font.pixelSize: Style.bodySize
                            color: Style.textColor
                        }
                    }

                    // Type and Level
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: "Game Type"
                                font.bold: true
                                color: Style.secondaryTextColor
                            }
                            ComboBox {
                                id: typeCombo
                                Layout.fillWidth: true
                                model: ["sentence_match", "word_match", "image_match"]
                                enabled: mode !== "view"
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5

                            Text {
                                text: "Difficulty Level"
                                font.bold: true
                                color: Style.secondaryTextColor
                            }
                            ComboBox {
                                id: levelCombo
                                Layout.fillWidth: true
                                model: ["beginner", "intermediate", "advanced"]
                                enabled: mode !== "view"
                            }
                        }
                    }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#e0e0e0" }

                    // Items Section
                    Text {
                        text: "Game Items (" + itemsModel.count + ")"
                        font.pixelSize: Style.subHeadingSize
                        font.bold: true
                        color: Style.textColor
                    }

                    // Add Item Row (hidden in view mode)
                    RowLayout {
                        visible: mode !== "view"
                        Layout.fillWidth: true
                        spacing: 10

                        property string qLabel: {
                            if (typeCombo.currentText === "image_match") return "Image URL"
                            if (typeCombo.currentText === "sentence_match") return "Sentence Part 1"
                            return "Word"
                        }
                        property string aLabel: {
                            if (typeCombo.currentText === "image_match") return "Correct Word"
                            if (typeCombo.currentText === "sentence_match") return "Sentence Part 2"
                            return "Meaning/Match"
                        }

                        TextField {
                            id: inputQ
                            Layout.fillWidth: true
                            placeholderText: parent.qLabel
                            font.pixelSize: Style.bodySize
                            onAccepted: inputA.forceActiveFocus()
                        }

                        Text {
                            text: "→"
                            color: Style.secondaryTextColor
                            font.pixelSize: 20
                        }

                        TextField {
                            id: inputA
                            Layout.fillWidth: true
                            placeholderText: parent.aLabel
                            font.pixelSize: Style.bodySize
                            onAccepted: addItem()
                        }

                        Button {
                            text: "+ Add"
                            background: Rectangle {
                                color: Style.successColor
                                radius: Style.cornerRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: addItem()
                        }
                    }

                    function addItem() {
                        if (inputQ.text !== "" && inputA.text !== "") {
                            itemsModel.append({ "question": inputQ.text, "answer": inputA.text })
                            inputQ.text = ""
                            inputA.text = ""
                            inputQ.forceActiveFocus()
                        }
                    }

                    // Items List
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.max(200, Math.min(itemsModel.count * 50 + 20, 400))
                        color: "#f8f9fa"
                        radius: Style.cornerRadius
                        border.color: "#e0e0e0"

                        ListView {
                            id: itemsList
                            anchors.fill: parent
                            anchors.margins: 10
                            model: itemsModel
                            clip: true
                            spacing: 8

                            delegate: Rectangle {
                                width: itemsList.width
                                height: 44
                                color: Style.cardBackground
                                radius: 4
                                border.color: "#e0e0e0"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    spacing: 10

                                    Text {
                                        text: (index + 1) + "."
                                        color: Style.secondaryTextColor
                                        font.bold: true
                                        Layout.preferredWidth: 30
                                    }

                                    Text {
                                        text: model.question
                                        font.pixelSize: Style.bodySize
                                        color: Style.textColor
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        text: "→"
                                        color: Style.primaryColor
                                        font.bold: true
                                    }

                                    Text {
                                        text: model.answer
                                        font.pixelSize: Style.bodySize
                                        color: Style.textColor
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }

                                    Button {
                                        visible: mode !== "view"
                                        text: "×"
                                        flat: true
                                        Layout.preferredWidth: 30
                                        contentItem: Text {
                                            text: parent.text
                                            color: Style.errorColor
                                            font.bold: true
                                            font.pixelSize: 18
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        onClicked: itemsModel.remove(index)
                                    }
                                }
                            }
                        }

                        // Empty state
                        Text {
                            visible: itemsModel.count === 0
                            anchors.centerIn: parent
                            text: mode === "view" ? "No items in this game." : "Add items using the fields above."
                            color: Style.secondaryTextColor
                        }
                    }

                    Rectangle { height: 1; Layout.fillWidth: true; color: "#e0e0e0" }

                    // Actions
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 15

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "Cancel"
                            background: Rectangle {
                                color: Style.secondaryColor
                                radius: Style.cornerRadius
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: stackView.pop()
                        }

                        Button {
                            visible: mode !== "view"
                            text: mode === "create" ? "Create Game" : "Save Changes"
                            enabled: itemsModel.count > 0
                            background: Rectangle {
                                color: parent.enabled ? Style.primaryColor : Style.secondaryColor
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
                                statusMessage.text = "Saving..."
                                statusMessage.color = Style.secondaryTextColor

                                var jsonData = generateJson()

                                if (mode === "create") {
                                    networkManager.requestAdminCreateGame(
                                        typeCombo.currentText,
                                        levelCombo.currentText,
                                        jsonData
                                    )
                                } else {
                                    networkManager.requestAdminUpdateGame(
                                        gameId,
                                        typeCombo.currentText,
                                        levelCombo.currentText,
                                        jsonData
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
