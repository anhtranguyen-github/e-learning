import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Study Lesson"
        onBackClicked: stackView.pop()
    }

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

    RowLayout {
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: Style.margin

        // Sidebar for controls (Left side)
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Style.margin
                spacing: 10
                
                Text {
                    text: "View Mode"
                    font.family: Style.fontFamily
                    font.bold: true
                    color: Style.secondaryTextColor
                }

                Button {
                    text: "Full Lesson"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: networkManager.requestStudyLesson(lessonId, "FULL")
                }
                Button {
                    text: "Text Only"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: networkManager.requestStudyLesson(lessonId, "TEXT")
                }
                Button {
                    text: "Vocabulary"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: networkManager.requestStudyLesson(lessonId, "VOCABULARY")
                }
                Button {
                    text: "Grammar"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: networkManager.requestStudyLesson(lessonId, "GRAMMAR")
                }
                
                Item { Layout.fillHeight: true } // Spacer
            }
        }

        // Main Content Area (Right side)
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: Style.margin
                
                TextArea {
                    id: contentArea
                    readOnly: true
                    wrapMode: Text.WordWrap
                    text: "Loading..."
                    font.family: Style.fontFamily
                    font.pixelSize: Style.bodySize
                    color: Style.textColor
                    background: null
                    selectByMouse: true
                }
            }
        }
    }
}
