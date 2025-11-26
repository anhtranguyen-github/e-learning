import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Exams"

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
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        model: ListModel { id: examModel }

        delegate: ItemDelegate {
            width: listView.width
            contentItem: ColumnLayout {
                Text { text: model.title; font.bold: true }
                Text { text: "Type: " + model.type + " | Level: " + model.level }
            }
            onClicked: {
                // Reuse DoExerciseScreen for exams as they are similar in submission
                // But we need to specify targetType as "exam" in submitAnswer
                // DoExerciseScreen currently hardcodes "exercise".
                // We should make it flexible.
                // For now, let's just show a message or create DoExamScreen if needed.
                // Or update DoExerciseScreen to accept targetType.
                // Let's update DoExerciseScreen in next step or pass a property.
                
                stackView.push("DoExerciseScreen.qml", { 
                    "exerciseId": parseInt(model.examId),
                    "exerciseTypeStr": model.type,
                    "targetType": "exam"
                })
            }
        }
    }
}
