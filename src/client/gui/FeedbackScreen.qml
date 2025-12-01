import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: root
    background: Rectangle { color: Style.backgroundColor }
    
    property string feedbackText: ""
    property string score: ""
    
    header: Header {
        title: "Feedback"
        onBackClicked: stackView.pop()
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 20

        Text {
            text: "Result"
            font.pixelSize: 32
            font.bold: true
            color: Style.primaryColor
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Score: " + score
            font.pixelSize: 24
            color: Style.textColor
            Layout.alignment: Qt.AlignHCenter
            visible: score !== ""
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: "#e0e0e0"
            border.width: 1

            ScrollView {
                anchors.fill: parent
                anchors.margins: 15
                
                TextArea {
                    text: feedbackText
                    readOnly: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: Style.bodySize
                    color: Style.textColor
                    background: null
                }
            }
        }

        Button {
            text: "Back to Dashboard"
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            
            background: Rectangle {
                color: Style.primaryColor
                radius: Style.cornerRadius
            }
            
            contentItem: Text {
                text: parent.text
                color: "white"
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            
            onClicked: {
                stackView.pop(null) // Pop to root
            }
        }
    }
}
