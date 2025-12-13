import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "Dashboard"
        showBack: false
    }

    ListModel {
        id: menuModel
    }

    Component.onCompleted: {
        menuModel.clear()
        var role = networkManager.userRole
        
        if (role === "admin") {
            menuModel.append({ name: "Manage Games", icon: "🎮", page: "AdminGameScreen.qml", color: "#e74a3b" })
            menuModel.append({ name: "Lessons", icon: "📖", page: "LessonListScreen.qml", color: "#4e73df" })
            menuModel.append({ name: "Chat", icon: "💬", page: "ChatScreen.qml", color: "#8e44ad" })
        } else if (role === "teacher") {
            menuModel.append({ name: "Lessons", icon: "📖", page: "LessonListScreen.qml", color: "#4e73df" })
            menuModel.append({ name: "Grading", icon: "📝", page: "GradingListScreen.qml", color: "#4e73df" })
            menuModel.append({ name: "Chat", icon: "💬", page: "ChatScreen.qml", color: "#8e44ad" })
        } else {
            // Default to student view
            menuModel.append({ name: "Games", icon: "🎮", page: "GameListScreen.qml", color: "#e74a3b" })
            menuModel.append({ name: "Lessons", icon: "📖", page: "LessonListScreen.qml", color: "#4e73df" })
            menuModel.append({ name: "Exercises", icon: "✍️", page: "ExerciseListScreen.qml", color: "#1cc88a" })
            menuModel.append({ name: "Exams", icon: "📝", page: "ExamListScreen.qml", color: "#36b9cc" })
            menuModel.append({ name: "My Results", icon: "📊", page: "ResultListScreen.qml", color: "#f6c23e" })
            menuModel.append({ name: "Chat", icon: "💬", page: "ChatScreen.qml", color: "#8e44ad" })
        }
        menuModel.append({ name: "Settings", icon: "⚙️", page: "SettingsScreen.qml", color: "#7f8c8d" })
        menuModel.append({ name: "Logout", icon: "🚪", page: "LOGOUT", color: "#e74a3b" })
    }

    GridView {
        anchors.fill: parent
        anchors.margins: Style.margin
        cellWidth: width > 600 ? width / 3 : width / 2
        cellHeight: 180
        model: menuModel
        
        delegate: Item {
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight
            
            Rectangle {
                anchors.fill: parent
                anchors.margins: Style.smallMargin
                color: Style.cardBackground
                radius: Style.cornerRadius
                
                // Simple shadow/border
                border.color: "#e0e0e0"
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.color = "#f8f9fa"
                    onExited: parent.color = Style.cardBackground
                    onClicked: {
                        if (model.page === "LOGOUT") {
                            networkManager.disconnect()
                            stackView.pop(null)
                        } else {
                            stackView.push(model.page)
                        }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 10
                    
                    Text {
                        text: model.icon
                        font.pixelSize: 48
                        Layout.alignment: Qt.AlignHCenter
                    }
                    
                    Text {
                        text: model.name
                        font.family: Style.fontFamily
                        font.pixelSize: Style.subHeadingSize
                        font.bold: true
                        color: Style.textColor
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
                
                // Colored strip at bottom
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 4
                    color: model.color
                    radius: Style.cornerRadius
                }
            }
        }
    }
}
