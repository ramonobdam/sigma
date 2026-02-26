import QtQuick

// Component to access the fonts used in the Sigma app
Item {
    property alias inter: inter
    property alias interSemiBold: interSemiBold
    property alias fontAwesome: fontAwesome

    FontLoader {
        id: inter

        source:
            "qrc:/qt/qml/Sigma/resources/third_party/inter/Inter-Regular.otf"
    }

    FontLoader {
        id: interSemiBold

        source:
            "qrc:/qt/qml/Sigma/resources/third_party/inter/Inter-SemiBold.otf"
    }

    FontLoader {
        id: fontAwesome

        source:
            "qrc:/qt/qml/Sigma/resources/third_party/Font Awesome/Font Awesome 7 Free-Solid-900.otf"
    }

    FontLoader {
        id: fontAwesomeRegular

        source:
            "qrc:/qt/qml/Sigma/resources/third_party/Font Awesome/Font Awesome 7 Free-Regular-400.otf"
    }
}
