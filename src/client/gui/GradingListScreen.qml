import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Student Submissions"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestPendingSubmissions()
    }

    // Filter buttons
    property string statusFilter: "all" // "all", "pending", "graded"

    Connections {
        target: networkManager
        function onPendingSubmissionsReceived(listData) {
            submissionModel.clear()
            var parts = listData.split(';')
            // First part is count
            if (parts.length > 1) {
                for (var i = 1; i < parts.length; i++) {
                    var itemParts = parts[i].split('|')
                    // SubmissionDTO: resultId|studentName|targetType|targetTitle|targetId|submittedAt|status|score|userAnswer
                    if (itemParts.length >= 7) {
                        submissionModel.append({
                            "resultId": itemParts[0],
                            "studentName": itemParts[1],
                            "targetType": itemParts[2],
                            "targetTitle": itemParts[3],
                            "targetId": itemParts[4],
                            "submittedAt": itemParts[5],
                            "status": itemParts[6] || "pending",
                            "score": itemParts.length >= 8 ? itemParts[7] : "0",
                            "userAnswer": itemParts.length >= 9 ? itemParts[8] : ""
                        })
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: 15

        // Filter Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Filter:"
                font.pixelSize: Style.bodySize
                color: Style.textColor
            }

            Button {
                text: "All"
                flat: statusFilter !== "all"
                highlighted: statusFilter === "all"
                onClicked: statusFilter = "all"
            }

            Button {
                text: "Pending"
                flat: statusFilter !== "pending"
                highlighted: statusFilter === "pending"
                onClicked: statusFilter = "pending"
            }

            Button {
                text: "Graded"
                flat: statusFilter !== "graded"
                highlighted: statusFilter === "graded"
                onClicked: statusFilter = "graded"
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "Total: " + submissionModel.count
                font.pixelSize: Style.smallSize
                color: "gray"
            }
        }

        ListView {
            id: submissionListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            clip: true

            model: ListModel { id: submissionModel }

            delegate: Rectangle {
                visible: statusFilter === "all" || model.status === statusFilter
                width: submissionListView.width
                height: visible ? 120 : 0
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: model.status === "pending" ? Style.warningColor : Style.successColor
                border.width: 2

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        stackView.push("GradingScreen.qml", {
                            "resultId": model.resultId,
                            "userName": model.studentName,
                            "targetType": model.targetType,
                            "targetTitle": model.targetTitle,
                            "userAnswer": model.userAnswer,
                            "targetId": model.targetId
                        })
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    // Status Badge
                    Rectangle {
                        width: 80
                        height: 30
                        radius: 15
                        color: model.status === "pending" ? Style.warningColor : Style.successColor

                        Text {
                            anchors.centerIn: parent
                            text: model.status === "pending" ? "PENDING" : "GRADED"
                            font.pixelSize: Style.smallSize
                            font.bold: true
                            color: "white"
                        }
                    }

                    // Main Content
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        Text {
                            text: model.targetTitle
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.textColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 15

                            Text {
                                text: "Student: " + model.studentName
                                font.pixelSize: Style.bodySize
                                color: Style.textColor
                            }

                            Text {
                                text: "Type: " + model.targetType.charAt(0).toUpperCase() + model.targetType.slice(1)
                                font.pixelSize: Style.bodySize
                                color: "gray"
                            }
                        }

                        Text {
                            text: "Submitted: " + model.submittedAt
                            font.pixelSize: Style.smallSize
                            color: "gray"
                        }
                    }

                    // Score (if graded)
                    Rectangle {
                        visible: model.status === "graded"
                        width: 60
                        height: 60
                        radius: 30
                        color: Style.primaryColor

                        Text {
                            anchors.centerIn: parent
                            text: parseFloat(model.score).toFixed(0) + "%"
                            font.pixelSize: Style.bodySize
                            font.bold: true
                            color: "white"
                        }
                    }

                    // Arrow
                    Text {
                        text: "→"
                        font.pixelSize: 24
                        color: Style.primaryColor
                    }
                }
            }
        }
    }
}
