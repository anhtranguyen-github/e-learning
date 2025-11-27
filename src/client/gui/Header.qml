import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Rectangle {
    id: root
    height: Style.headerHeight
    color: Style.cardBackground
    
    // Shadow effect (simulated with a bottom border for simplicity in Qt 5)
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: "#e0e0e0"
    }

    property string title: ""
    property bool showBack: true
    signal backClicked()

    RowLayout {
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: Style.margin

        Button {
            visible: root.showBack
            text: "← Back"
            font.pixelSize: Style.bodySize
            flat: true
            
            contentItem: Text {
                text: parent.text
                font: parent.font
                color: Style.primaryColor
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
            
            background: Rectangle {
                color: "transparent"
            }

            onClicked: root.backClicked()
        }

        Text {
            text: root.title
            font.family: Style.fontFamily
            font.pixelSize: Style.subHeadingSize
            font.bold: true
            color: Style.textColor
            Layout.fillWidth: true
            horizontalAlignment: root.showBack ? Text.AlignLeft : Text.AlignHCenter
        }
        
        // Spacer to balance the back button if centered title is desired, 
        // but for now left-aligned title next to back button is fine.
    }
}
