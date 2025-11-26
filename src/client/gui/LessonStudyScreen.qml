import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Study Lesson"
    property int lessonId

    Component.onCompleted: {
        networkManager.requestStudyLesson(lessonId, "FULL")
    }

    Connections {
        target: networkManager
        function onLessonContentReceived(content) {
            contentArea.text = content
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Button { text: "Full"; onClicked: networkManager.requestStudyLesson(lessonId, "FULL") }
            Button { text: "Text"; onClicked: networkManager.requestStudyLesson(lessonId, "TEXT") }
            Button { text: "Vocab"; onClicked: networkManager.requestStudyLesson(lessonId, "VOCABULARY") }
            Button { text: "Grammar"; onClicked: networkManager.requestStudyLesson(lessonId, "GRAMMAR") }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            TextArea {
                id: contentArea
                readOnly: true
                wrapMode: Text.WordWrap
                text: "Loading..."
            }
        }
    }
}
