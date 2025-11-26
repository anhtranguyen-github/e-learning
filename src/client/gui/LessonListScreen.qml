import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Lessons"

    Component.onCompleted: {
        networkManager.requestLessonList("", "")
    }

    Connections {
        target: networkManager
        function onLessonListReceived(listData) {
            lessonModel.clear()
            var parts = listData.split(";")
            var count = parseInt(parts[0])
            
            for (var i = 1; i < parts.length; i++) {
                var meta = parts[i]
                if (meta === "") continue
                
                var fields = meta.split("|")
                if (fields.length >= 4) {
                    lessonModel.append({
                        "lessonId": fields[0],
                        "title": fields[1],
                        "topic": fields[2],
                        "level": fields[3]
                    })
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: topicFilter
                placeholderText: "Topic"
                Layout.fillWidth: true
            }
            TextField {
                id: levelFilter
                placeholderText: "Level"
                Layout.fillWidth: true
            }
            Button {
                text: "Filter"
                onClicked: networkManager.requestLessonList(topicFilter.text, levelFilter.text)
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel { id: lessonModel }

            delegate: ItemDelegate {
                width: listView.width
                contentItem: ColumnLayout {
                    Text { text: model.title; font.bold: true }
                    Text { text: "Topic: " + model.topic + " | Level: " + model.level }
                }
                onClicked: {
                    stackView.push("LessonStudyScreen.qml", { "lessonId": parseInt(model.lessonId) })
                }
            }
        }
    }
}
