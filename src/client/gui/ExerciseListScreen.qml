import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Exercises"

    Component.onCompleted: {
        networkManager.requestExerciseList()
    }

    Connections {
        target: networkManager
        function onExerciseListReceived(listData) {
            exerciseModel.clear()
            var parts = listData.split(";")
            var count = parseInt(parts[0])
            
            for (var i = 1; i < parts.length; i++) {
                var meta = parts[i]
                if (meta === "") continue
                
                var fields = meta.split("|")
                if (fields.length >= 5) {
                    exerciseModel.append({
                        "exerciseId": fields[0],
                        "lessonId": fields[1],
                        "title": fields[2],
                        "type": fields[3],
                        "level": fields[4]
                    })
                }
            }
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        model: ListModel { id: exerciseModel }

        delegate: ItemDelegate {
            width: listView.width
            contentItem: ColumnLayout {
                Text { text: model.title; font.bold: true }
                Text { text: "Type: " + model.type + " | Level: " + model.level }
            }
            onClicked: {
                // Map type string to int code if needed, or pass string and handle in DoExerciseScreen
                // For now, passing ID and Type string
                stackView.push("DoExerciseScreen.qml", { 
                    "exerciseId": parseInt(model.exerciseId),
                    "exerciseTypeStr": model.type
                })
            }
        }
    }
}
