import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Pending Submissions"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestPendingSubmissions()
    }

    Connections {
        target: networkManager
        function onPendingSubmissionsReceived(listData) {
            submissionModel.clear()
            var parts = listData.split(';')
            // First part is count, skip it
            if (parts.length > 1) {
                for (var i = 1; i < parts.length; i++) {
                    var itemParts = parts[i].split('|')
                    if (itemParts.length >= 7) {
                        submissionModel.append({
                            "resultId": itemParts[0],
                            "userName": itemParts[1],
                            "targetType": itemParts[2],
                            "targetTitle": itemParts[3],
                            "submittedAt": itemParts[4],
                            "userAnswer": itemParts[5],
                            "targetId": itemParts[6]
                        })
                    }
                }
            }
        }
    }

    ListView {
        id: submissionListView
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: 10
        clip: true

        model: ListModel { id: submissionModel }

        delegate: Rectangle {
            width: submissionListView.width
            height: 100
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    stackView.push("GradingScreen.qml", {
                        "resultId": model.resultId,
                        "userName": model.userName,
                        "targetType": model.targetType,
                        "targetTitle": model.targetTitle,
                        "userAnswer": model.userAnswer,
                        "targetId": model.targetId
                    })
                }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Text {
                    text: model.targetTitle + " (" + model.targetType + ")"
                    font.pixelSize: Style.subHeadingSize
                    font.bold: true
                    color: Style.textColor
                }

                Text {
                    text: "User: " + model.userName
                    font.pixelSize: Style.bodySize
                    color: Style.textColor
                }

                Text {
                    text: "Submitted: " + model.submittedAt
                    font.pixelSize: Style.smallSize
                    color: "gray"
                }
            }
        }
    }
}
