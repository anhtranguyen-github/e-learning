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
                    // Expected: targetId|score|status|feedback|targetType|title
                    if (itemParts.length >= 6) {
                        resultModel.append({
                            "targetId": itemParts[0],
                            "score": itemParts[1],
                            "status": itemParts[2],
                            "feedback": itemParts[3],
                            "targetType": itemParts[4],
                            "title": itemParts[5]
                        })
                    } else if (itemParts.length >= 5) {
                         // Backward compatibility
                        resultModel.append({
                            "targetId": itemParts[0],
                            "score": itemParts[1],
                            "status": itemParts[2],
                            "feedback": itemParts[3],
                            "targetType": itemParts[4],
                            "title": "Unknown Title"
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
            height: 100 // Increased height for type
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: model.title || "Unknown Title"
                        font.pixelSize: Style.subHeadingSize
                        font.bold: true
                        color: Style.textColor
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Text {
                        text: model.targetType === "exam" ? "Exam" : (model.targetType === "exercise" ? "Exercise" : model.targetType)
                        font.pixelSize: Style.bodySize
                        font.bold: true
                        color: Style.primaryColor
                    }
                }

                RowLayout {
                    spacing: 20
                    
                    Text {
                        text: "Status: " + model.status
                        font.pixelSize: Style.bodySize
                        color: model.status === "graded" ? "green" : "orange"
                    }

                    Text {
                        text: model.status === "graded" ? "Score: " + (model.score ? parseFloat(model.score).toFixed(1) : "0.0") + "%" : "Score: Pending"
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
            
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stackView.push("ResultDetailScreen.qml", {
                        "targetType": model.targetType,
                        "targetId": model.targetId
                    })
                }
            }
        }
    }
}
