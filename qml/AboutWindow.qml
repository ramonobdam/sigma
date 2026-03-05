import QtQuick
import QtQuick.Controls
import QtCore

// Window with version and license information
BaseWindow {
    id: control

    windowTitle: "About " + Application.displayName
    width: 460
    height: container.childrenRect.height +
            titleBar.height +
            container.anchors.margins * 2
    settingsKey: "aboutWindow"
    settingsObject: Settings {}

    Item {
        id: container

        anchors {
            left: parent.left
            right: parent.right
            top: titleBar.bottom
            bottom: parent.bottom
            margins: properties.spacingM
        }

        focus: true
        Keys.onEscapePressed: { control.close() }

        // App icon
        Image {
            id: icon

            anchors.horizontalCenter: parent.horizontalCenter

            source: properties.appIcon
            width: properties.appIconWidth
            height: width
            fillMode: Image.PreserveAspectFit
            mipmap: true
            antialiasing: true
        }

        // Title + version
        SigmaText {
            id: heading

            anchors {
                top: icon.bottom
                topMargin: properties.spacingM
                left: parent.left
                right: parent.right
            }

            text: Application.displayName
            color: properties.colorTextStrong
        }

        SigmaText {
            id: version

            anchors {
                top: heading.bottom
                topMargin: properties.spacingS
                left: parent.left
                right: parent.right
            }

            text: "Version " + Application.version
        }

        // Description text
        SigmaRichText {
            id: description

            anchors {
                top: version.bottom
                topMargin: properties.spacingM
                left: parent.left
                right: parent.right
            }

            text:
                "<p>
                The " + Application.name +
                " implements measurement uncertainty calculation in accordance
                with the Guide to the expression of Uncertainty in
                Measurement (GUM).
                </p>

                <p>
                The combined measurement uncertainty and uncertainty budget are
                calculated using the method of propagation of uncertainties as
                given in <a style=\"color: " + properties.colorBrand  +
                ";\" href='https://doi.org/10.59161/JCGM100-2008E'>"+
                "GUM JCGM 100:2008</a>.
                </p>

                <p>
                In addition, a more broadly applicable Monte Carlo simulation
                option is available that implements the propagation of
                distributions method as given in
                <a style=\"color: " + properties.colorBrand  +
                ";\" href='https://doi.org/10.59161/JCGM101-2008'>
                GUM JCGM 101:2008</a>.
                </p>"
        }

        ContextMenuRule {
            id: rule

            anchors {
                top: description.bottom
                topMargin: properties.spacingM
                left: parent.left
                right: parent.right
            }
        }

        // License text
        SigmaRichText {
            anchors {
                top: rule.bottom
                topMargin: properties.spacingM
                left: parent.left
                right: parent.right
            }

            text: "
            <p>
            Copyright © 2025–2026 Ramon Obdam
            </p>

            <p>
            Sigma is licensed under the
            <a style=\"color: " +
            properties.colorBrand  +
            ";\" href='https://opensource.org/licenses/MIT'>MIT License</a>.
            </p>

            <p>
            This product dynamically links against the Qt framework.
            Qt is licensed under the
            <a style=\"color: " +
            properties.colorBrand  +
            ";\" href='https://www.gnu.org/licenses/lgpl-3.0.html'>
            GNU Lesser General Public License v3 (LGPL v3)</a>.
            </p>

            <p>
            Third-party components include Qt, ExprTk, ALGLIB,
            Font Awesome and the Inter typeface.
            </p>

            <p>
            Full license texts can be found in the Sigma installion folder.
            </p>
            "
        }
    }
}
