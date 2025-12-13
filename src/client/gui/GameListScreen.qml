import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }

    ListModel {
        id: gameTypeModel
    }

    Component.onCompleted: {
        networkManager.requestGameList()
    }

    Connections {
        target: networkManager
        
        function onGameListReceived(listData) {
            gameTypeModel.clear()
            if (listData.length === 0) return

            var parts = listData.split(";")
            
            for (var i = 0; i < parts.length; i++) {
                if (parts[i].length > 0) {
                    var dtoParts = parts[i].split("|")
                    if (dtoParts.length >= 2) {
                        var rawType = dtoParts[0]
                        var description = dtoParts[1]
                        
                        // Beautify the name
                        var displayName = rawType.replace("_", " ").toUpperCase()
                        var icon = "🎮"
                        var color = "#4e73df"

                        if (rawType === "sentence_match") { icon = "📝"; color = "#4e73df" }
                        else if (rawType === "word_match") { icon = "🔤"; color = "#1cc88a" }
                        else if (rawType === "image_match") { icon = "🖼️"; color = "#f6c23e" }

                        gameTypeModel.append({ 
                            type: rawType, 
                            displayName: displayName,
                            description: description,
                            icon: icon,
                            color: color
                        })
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Button {
                text: "← Back"
                background: Rectangle {
                    color: "transparent"
                    border.color: Style.primaryColor
                    border.width: 1
                    radius: Style.cornerRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: Style.primaryColor
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: stackView.pop()
            }

            Text {
                text: "Choose a Game"
                font.family: Style.fontFamily
                font.pixelSize: Style.headingSize
                font.bold: true
                color: Style.textColor
                Layout.fillWidth: true
            }
        }

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            cellWidth: width > 600 ? width / 3 : width / 2
            cellHeight: 180
            model: gameTypeModel
            clip: true
            
            delegate: Item {
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: Style.smallMargin
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    
                    border.color: "#e0e0e0"
                    border.width: 1

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.color = "#f8f9fa"
                        onExited: parent.color = Style.cardBackground
                        onClicked: {
                            stackView.push("GameLevelScreen.qml", { gameType: model.type })
                        }
                    }

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 10
                        
                        Text {
                            text: model.icon
                            font.pixelSize: 48
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: model.displayName
                            font.family: Style.fontFamily
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.textColor
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 4
                        color: model.color
                        radius: Style.cornerRadius
                    }
                }
            }
        }
    }
}
