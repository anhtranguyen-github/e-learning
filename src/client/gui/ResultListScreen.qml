import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "My Results"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestResultList()
    }

    Connections {
        target: networkManager
        function onResultListReceived(listData) {
            resultModel.clear()
            var parts = listData.split(';')
            // First part is count, skip it
            if (parts.length > 1) {
                for (var i = 1; i < parts.length; i++) {
                    var itemParts = parts[i].split('|')
                    if (itemParts.length >= 4) {
                        resultModel.append({
                            "targetId": itemParts[0],
                            "score": itemParts[1],
                            "status": itemParts[2],
                            "feedback": itemParts[3]
                        })
                    } else if (itemParts.length >= 3) {
                         resultModel.append({
                            "targetId": itemParts[0],
                            "score": itemParts[1],
                            "status": itemParts[2],
                            "feedback": ""
                        })
                    }
                }
            }
        }
    }

    ListView {
        id: resultListView
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: 10
        clip: true

        model: ListModel { id: resultModel }

        delegate: Rectangle {
            width: resultListView.width
            height: 80
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Text {
                    text: "ID: " + model.targetId
                    font.pixelSize: Style.subHeaderSize
                    font.bold: true
                    color: Style.textColor
                }

                RowLayout {
                    spacing: 20
                    
                    Text {
                        text: "Status: " + model.status
                        font.pixelSize: Style.bodySize
                        color: model.status === "graded" ? "green" : "orange"
                    }

                    Text {
                        text: model.status === "graded" ? "Score: " + parseFloat(model.score).toFixed(1) + "%" : "Score: Pending"
                        font.pixelSize: Style.bodySize
                        color: Style.textColor
                        visible: true
                    }
                }

                Text {
                    text: model.feedback ? "Feedback: " + model.feedback : ""
                    font.pixelSize: Style.smallSize
                    color: "gray"
                    visible: model.feedback !== ""
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
        }
    }
}
