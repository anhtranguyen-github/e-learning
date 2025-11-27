import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Lessons"
        onBackClicked: stackView.pop()
    }

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
        anchors.margins: Style.margin
        spacing: Style.margin

        // Filter Bar
        RowLayout {
            Layout.fillWidth: true
            spacing: Style.smallMargin
            
            TextField {
                id: topicFilter
                placeholderText: "Topic"
                Layout.fillWidth: true
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: "#e0e0e0"
                }
                padding: 10
            }
            TextField {
                id: levelFilter
                placeholderText: "Level"
                Layout.fillWidth: true
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    border.color: "#e0e0e0"
                }
                padding: 10
            }
            Button {
                text: "Filter"
                font.pixelSize: Style.bodySize
                background: Rectangle {
                    color: Style.primaryColor
                    radius: Style.cornerRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: networkManager.requestLessonList(topicFilter.text, levelFilter.text)
            }
        }

        GridView {
            id: gridView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: width > 800 ? width / 3 : (width > 500 ? width / 2 : width)
            cellHeight: 120
            model: ListModel { id: lessonModel }

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
                            stackView.push("LessonStudyScreen.qml", { "lessonId": parseInt(model.lessonId) })
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
                            spacing: 10
                            
                            Rectangle {
                                color: "#e3f2fd"
                                radius: 4
                                width: topicText.width + 16
                                height: topicText.height + 8
                                Text {
                                    id: topicText
                                    anchors.centerIn: parent
                                    text: model.topic
                                    font.pixelSize: Style.smallSize
                                    color: Style.primaryDarkColor
                                }
                            }
                            
                            Rectangle {
                                color: "#e8f5e9"
                                radius: 4
                                width: levelText.width + 16
                                height: levelText.height + 8
                                Text {
                                    id: levelText
                                    anchors.centerIn: parent
                                    text: model.level
                                    font.pixelSize: Style.smallSize
                                    color: Style.successColor
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
