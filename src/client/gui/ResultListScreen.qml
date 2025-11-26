import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    title: "Results"

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
        anchors.margins: 10
        TextArea {
            id: resultText
            readOnly: true
            wrapMode: Text.WordWrap
            text: "Loading results..."
        }
    }
}
