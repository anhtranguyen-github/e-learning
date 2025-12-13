import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    property string gameType: ""

    ListModel {
        id: levelModel
    }

    Component.onCompleted: {
        networkManager.requestGameLevelList(gameType)
    }

    Connections {
        target: networkManager
        
        function onGameLevelListReceived(listData) {
            levelModel.clear()
            if (listData.length === 0) return

            var parts = listData.split(";")
            
            for (var i = 0; i < parts.length; i++) {
                if (parts[i].length > 0) {
                    var dtoParts = parts[i].split("|")
                    if (dtoParts.length >= 2) {
                        var id = dtoParts[0]
                        var level = dtoParts[1]
                        var status = dtoParts[2] // "unlocked"

                        var displayName = "Level: " + level.charAt(0).toUpperCase() + level.slice(1)
                        var color = Style.successColor // Beginner
                        if (level === "intermediate") color = Style.primaryColor
                        if (level === "advanced") color = Style.errorColor

                        levelModel.append({ 
                            gameId: id,
                            level: level, 
                            displayName: displayName,
                            status: status,
                            color: color.toString()
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
                text: "Select Level"
                font.family: Style.fontFamily
                font.pixelSize: Style.headingSize
                font.bold: true
                color: Style.textColor
                Layout.fillWidth: true
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: levelModel
            clip: true
            spacing: 15
            
            delegate: Item {
                width: ListView.view.width
                height: 80
                
                Rectangle {
                    anchors.fill: parent
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
                            stackView.push("GamePlayScreen.qml", { gameId: model.gameId })
                        }
                    }

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: 20
                        
                        Rectangle {
                            width: 15
                            height: 15
                            radius: 7.5
                            color: model.color
                        }

                        Text {
                            text: model.displayName
                            font.family: Style.fontFamily
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.textColor
                        }
                    }
                }
            }
        }
    }
}
