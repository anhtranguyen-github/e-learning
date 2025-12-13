pragma Singleton
import QtQuick 2.15

QtObject {
    // Modern SaaS Palette (Tailwind-inspired)
    property color primaryColor: "#2563EB"      // Blue-600
    property color primaryDarkColor: "#1D4ED8"  // Blue-700
    property color secondaryColor: "#64748B"    // Slate-500
    property color backgroundColor: "#F1F5F9"   // Slate-100
    property color cardBackground: "#FFFFFF"    // White
    property color textColor: "#1E293B"         // Slate-800
    property color secondaryTextColor: "#475569" // Slate-600
    property color errorColor: "#EF4444"        // Red-500
    property color successColor: "#10B981"      // Emerald-500

    // Fonts
    property string fontFamily: "Inter, Segoe UI, Roboto, Helvetica, Arial, sans-serif"
    property int headingSize: 28
    property int subHeadingSize: 20
    property int bodySize: 16
    property int smallSize: 14
    
    // Standard Headings
    property int h1Size: 32
    property int h2Size: 24
    property int h3Size: 20

    // Dimensions
    property int margin: 24
    property int smallMargin: 12
    property int cornerRadius: 12
    property int buttonHeight: 48
    property int headerHeight: 72
}
