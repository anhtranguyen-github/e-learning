import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }
    
    header: Header {
        title: "My Results"
        onBackClicked: stackView.pop()
    }

    Component.onCompleted: {
        networkManager.requestResultList()
    }

    Connections {
        target: networkManager
        function onResultListReceived(listData) {
            resultText.text = listData
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Style.margin
        
        TextArea {
            id: resultText
            readOnly: true
            wrapMode: Text.WordWrap
            text: "Loading results..."
            font.family: Style.fontFamily
            font.pixelSize: Style.bodySize
            color: Style.textColor
            background: Rectangle {
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"
            }
            padding: Style.margin
        }
    }
}
