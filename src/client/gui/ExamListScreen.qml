import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Exams"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestExamList()
    }

    Connections {
        target: networkManager
        function onExamListReceived(listData) {
            examModel.clear()
            var parts = listData.split(";")
            var count = parseInt(parts[0])
            
            for (var i = 1; i < parts.length; i++) {
                var meta = parts[i]
                if (meta === "") continue
                
                var fields = meta.split("|")
                if (fields.length >= 5) {
                    examModel.append({
                        "examId": fields[0],
                        "lessonId": fields[1],
                        "title": fields[2],
                        "type": fields[3],
                        "level": fields[4]
                    })
                }
            }
        }

        function onExamContentReceived(content) {
            console.log("ExamListScreen: Received exam content, navigating to DoExerciseScreen")
            // Parse content here or pass raw content to DoExerciseScreen
            // DoExerciseScreen parses it in onExamContentReceived, but we are navigating now.
            // We can pass the content as a property.
            
            // Parse ExamDTO: id|lessonId|title|type|level|questions
            var parts = content.split('|');
            if (parts.length >= 6) {
                var questionsStr = parts.slice(5).join('|'); 
                var questions = questionsStr.split('^');
                
                var examData = {
                    id: parts[0],
                    lessonId: parts[1],
                    title: parts[2],
                    type: parts[3],
                    level: parts[4],
                    questions: questions
                };

                stackView.push("DoExerciseScreen.qml", { 
                    "exerciseId": parseInt(parts[0]),
                    "exerciseTypeStr": parts[3],
                    "targetType": "exam",
                    "preloadedExamData": examData
                })
            }
        }

        function onExamAlreadyTaken(message) {
            console.log("ExamListScreen: Exam already taken")
            messageDialog.messageText = message
            messageDialog.open()
        }
    }

    Dialog {
        id: messageDialog
        title: "Notification"
        property string messageText: ""
        
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 300
        modal: true
        standardButtons: Dialog.Ok

        contentItem: Label {
            text: messageDialog.messageText
            wrapMode: Text.WordWrap
            color: Style.textColor
            font.pixelSize: Style.bodySize
        }
    }

    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: Style.margin
        cellWidth: width > 800 ? width / 3 : (width > 500 ? width / 2 : width)
        cellHeight: 120
        model: ListModel { id: examModel }

        delegate: Item {
            width: gridView.cellWidth
            height: gridView.cellHeight
            
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
                        // Request exam first
                        networkManager.requestExam(parseInt(model.examId))
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Style.margin
                    spacing: 5
                    
                    Text {
                        text: model.title
                        font.family: Style.fontFamily
                        font.pixelSize: Style.subHeadingSize
                        font.bold: true
                        color: Style.textColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    
                    RowLayout {
                        spacing: Style.smallMargin
                        
                        Rectangle {
                            color: "#e0f7fa"
                            radius: 4
                            width: typeText.width + Style.smallMargin * 2
                            height: typeText.height + Style.smallMargin
                            Text {
                                id: typeText
                                anchors.centerIn: parent
                                text: model.type
                                font.pixelSize: Style.smallSize
                                font.bold: true
                                color: "#006064"
                            }
                        }
                        
                        Rectangle {
                            color: "#e8f5e9"
                            radius: 4
                            width: levelText.width + Style.smallMargin * 2
                            height: levelText.height + Style.smallMargin
                            Text {
                                id: levelText
                                anchors.centerIn: parent
                                text: model.level
                                font.pixelSize: Style.smallSize
                                font.bold: true
                                color: Style.successColor
                            }
                        }
                    }
                }
            }
        }
    }
}
