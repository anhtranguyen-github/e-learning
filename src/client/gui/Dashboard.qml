import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Dashboard"

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 20

        Text {
            text: "Main Menu"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Button {
            text: "Lessons"
            Layout.fillWidth: true
            onClicked: stackView.push("LessonListScreen.qml")
        }

        Button {
            text: "Exercises"
            Layout.fillWidth: true
            onClicked: stackView.push("ExerciseListScreen.qml")
        }

        Button {
            text: "Exams"
            Layout.fillWidth: true
            onClicked: stackView.push("ExamListScreen.qml")
        }

        Button {
            text: "My Results"
            Layout.fillWidth: true
            onClicked: stackView.push("ResultListScreen.qml")
        }

        Button {
            text: "Logout"
            Layout.fillWidth: true
            onClicked: {
                networkManager.disconnect()
                stackView.pop(null) // Pop to root (Login)
            }
        }
    }
}
