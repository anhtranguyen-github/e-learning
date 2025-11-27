pragma Singleton
import QtQuick 2.15

QtObject {
    // Colors
    property color primaryColor: "#007bff"
    property color primaryDarkColor: "#0056b3"
    property color secondaryColor: "#6c757d"
    property color backgroundColor: "#f8f9fa"
    property color cardBackground: "#ffffff"
    property color textColor: "#212529"
    property color secondaryTextColor: "#6c757d"
    property color errorColor: "#dc3545"
    property color successColor: "#28a745"

    // Fonts
    property string fontFamily: "Segoe UI, Roboto, Helvetica, Arial, sans-serif"
    property int headingSize: 24
    property int subHeadingSize: 18
    property int bodySize: 14
    property int smallSize: 12

    // Dimensions
    property int margin: 16
    property int smallMargin: 8
    property int cornerRadius: 8
    property int buttonHeight: 40
    property int headerHeight: 60
}
