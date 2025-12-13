import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Study Lesson"
        onBackClicked: {
            // mediaPlayer.stop()
            stackView.pop()
        }
    }

    property int lessonId
    property var lessonData: ({})

    Component.onCompleted: {
        networkManager.requestStudyLesson(lessonId, "FULL")
    }

    Connections {
        target: networkManager
        function onLessonContentReceived(content) {
            // Parse DTO: id|title|topic|level|videoUrl|audioUrl|textContent|vocab|grammar
            var parts = content.split('|');
            if (parts.length >= 7) {
                lessonData = {
                    id: parts[0],
                    title: parts[1],
                    topic: parts[2],
                    level: parts[3],
                    videoUrl: parts[4],
                    audioUrl: parts[5],
                    textContent: parts[6],
                    vocabulary: parts.length > 7 ? parts[7] : "",
                    grammar: parts.length > 8 ? parts[8] : ""
                };
                
                contentArea.text = lessonData.textContent;
                
                /*
                if (lessonData.videoUrl) {
                    mediaPlayer.source = lessonData.videoUrl;
                } else if (lessonData.audioUrl) {
                    mediaPlayer.source = lessonData.audioUrl;
                }
                */
            } else {
                contentArea.text = content; // Fallback
            }
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
                    onClicked: {
                        contentArea.text = lessonData.textContent
                        videoContainer.visible = !!lessonData.videoUrl
                    }
                }
                Button {
                    text: "Vocabulary"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: {
                        contentArea.text = lessonData.vocabulary ? lessonData.vocabulary.split(',').join('\n') : "No vocabulary"
                        videoContainer.visible = false
                    }
                }
                Button {
                    text: "Grammar"
                    Layout.fillWidth: true
                    flat: true
                    background: Rectangle { color: parent.down ? "#e0e0e0" : "transparent" }
                    contentItem: Text { text: parent.text; color: Style.primaryColor; font.bold: true }
                    onClicked: {
                        contentArea.text = lessonData.grammar ? lessonData.grammar.split(',').join('\n') : "No grammar"
                        videoContainer.visible = false
                    }
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

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Style.margin
                spacing: 10

                // Video/Audio Player Area
                Rectangle {
                    id: videoContainer
                    Layout.fillWidth: true
                    Layout.preferredHeight: visible ? 400 : 0
                    visible: !!lessonData.videoUrl || !!lessonData.audioUrl
                    color: "black"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Multimedia content not supported in this environment"
                        color: "white"
                    }
                    
                    /*
                    VideoOutput {
                        anchors.fill: parent
                        source: mediaPlayer
                        visible: !!lessonData.videoUrl
                    }
                    
                    // Simple controls
                    RowLayout {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.margins: 10
                        
                        Button {
                            text: mediaPlayer.playbackState === MediaPlayer.PlayingState ? "Pause" : "Play"
                            onClicked: mediaPlayer.playbackState === MediaPlayer.PlayingState ? mediaPlayer.pause() : mediaPlayer.play()
                        }
                    }
                    */
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    
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
}
