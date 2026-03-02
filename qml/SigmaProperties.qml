import QtQuick
import Sigma

// Global properties and parameters of the Sigma app
QtObject {

    // Main window properties
    property int defaultMainWindowWidth: 1440
    property int defaultMainWindowHeight: 810
    property string windowTitle:
        Application.displayName +
        (
            filePathNotEmpty ?
                ( " | " +  projectFileName + ( unsavedChanges ? "*" : "" ) ) :
                ""
        )

    // Color palette
    property bool systemThemeLight: Application.styleHints.colorScheme ===
                                    Qt.ColorScheme.Light
    property bool lightTheme: displayTheme === DisplayTheme.System ?
                                    systemThemeLight :
                                    ( displayTheme === DisplayTheme.Light )

    // Light theme color palette
    property string colorBrand_Light: "#00607F"
    property string colorTextStrong_Light: "#001D26"
    property string colorTextWeak_Light: "#515C60"
    property string colorStrokeStrong_Light: "#899599"
    property string colorStrokeWeak_Light: "#DBE3E6"
    property string colorFill_Light: "#F3F9FB"
    property string colorBase_Light: colorFill
    property string colorRaised_Light: "#FFFFFF"
    property string colorElevated_Light: colorRaised_Light
    property string colorError1000_Light: "#330000"
    property string colorWarning1000_Light: "#4F3A0C"
    property string colorSuccess1000_Light: "#002F1D"
    property string colorError500_Light: "#E54242"
    property string colorWarning500_Light: "#E1A928"
    property string colorSuccess500_Light: "#26A575"
    property string colorHover_Light: "#255772"
    property string colorDown_Light: "#23485C"
    property string colorTextHover_Light: "#E8EAEB"
    property string colorTextDown_Light: "#C7CCCE"
    property string colorGrey1000_Light: "#20333B"
    property var colorScale_Light: [
        "#26A575",
        "#67B273",
        "#7FBF43",
        "#A5B61C",
        "#A08400",
        "#BB7402",
        "#D35E23",
        "#E54242"
    ]

    // Dark theme color palette
    property string colorBrand_Dark: "#98E3FC"
    property string colorTextStrong_Dark: "#FFFFFF"
    property string colorTextWeak_Dark: "#CDD6D9"
    property string colorStrokeStrong_Dark: "#95A2A7"
    property string colorStrokeWeak_Dark: "#373E40"
    property string colorFill_Dark: "#262F32"
    property string colorBase_Dark: "#12181A"
    property string colorRaised_Dark: "#191F21"
    property string colorElevated_Dark: "#1F2628"
    property string colorError1000_Dark: "#5F1A18"
    property string colorWarning1000_Dark: "#644D17"
    property string colorSuccess1000_Dark: "#12482F"
    property string colorError500_Dark: "#FF7979"
    property string colorWarning500_Dark: "#E6C26E"
    property string colorSuccess500_Dark: "#5FBE9A"
    property string colorHover_Dark: "#B7E6FA"
    property string colorDown_Dark: "#CEEEFC"
    property string colorTextHover_Dark: "#3A3E40"
    property string colorTextDown_Dark: "#7A7D7E"
    property string colorGrey1000_Dark: "#FFFFFF"
    property var colorScale_Dark: [
        "#5fbe9a",
        "#57AF67",
        "#87B363",
        "#C5CF65",
        "#E3BC46",
        "#E4994A",
        "#E17449",
        "#FF7979"
    ]

    // Actual color palette (based on bool lightTheme)
    property string colorBrand: lightTheme ? colorBrand_Light : colorBrand_Dark
    property string colorTextStrong: lightTheme ? colorTextStrong_Light :
                                                  colorTextStrong_Dark
    property string colorTextWeak: lightTheme ? colorTextWeak_Light :
                                                colorTextWeak_Dark
    property string colorStrokeStrong: lightTheme ? colorStrokeStrong_Light :
                                                    colorStrokeStrong_Dark
    property string colorStrokeWeak: lightTheme ? colorStrokeWeak_Light :
                                                  colorStrokeWeak_Dark
    property string colorFill: lightTheme ? colorFill_Light : colorFill_Dark
    property string colorBase: lightTheme ? colorBase_Light : colorBase_Dark
    property string colorRaised: lightTheme ? colorRaised_Light :
                                              colorRaised_Dark
    property string colorElevated: lightTheme ? colorElevated_Light :
                                                colorElevated_Dark
    property string colorError1000: lightTheme ? colorError1000_Light :
                                                 colorError1000_Dark
    property string colorWarning1000: lightTheme ? colorWarning1000_Light :
                                                   colorWarning1000_Dark
    property string colorSuccess1000: lightTheme ? colorSuccess1000_Light :
                                                   colorSuccess1000_Dark
    property string colorError500: lightTheme ? colorError500_Light :
                                                colorError500_Dark
    property string colorWarning500: lightTheme ? colorWarning500_Light :
                                                  colorWarning500_Dark
    property string colorSuccess500: lightTheme ? colorSuccess500_Light :
                                                  colorSuccess500_Dark
    property string colorHover: lightTheme ? colorHover_Light : colorHover_Dark
    property string colorDown: lightTheme ? colorDown_Light : colorDown_Dark
    property string colorTextHover: lightTheme ? colorTextHover_Light :
                                                 colorTextHover_Dark
    property string colorTextDown: lightTheme ? colorTextDown_Light :
                                                colorTextDown_Dark
    property string colorGrey1000: lightTheme ? colorGrey1000_Light :
                                                colorGrey1000_Dark
    property string colorTransparent: "transparent"
    property string colorClose: "#E81123"
    property var colorScale: lightTheme ? colorScale_Light : colorScale_Dark

    // Spacing options
    property int spacingXXS: 2
    property int spacingXS: 4
    property int spacingS: 8
    property int spacingM: 16
    property int spacingL: 24
    property int spacingXL: 32
    property int spacingXXL: 48
    // Compensate for 'activefocus border' on buttons:
    property int spacingButtons: spacingM - 4 * borderWidth

    // Radius options
    property int radiusXS: 2
    property int radiusS: 4
    property int radiusM: 8

    // Elevation options
    property int elevationRaised: 2
    property int elevationElevated: 5

    // Dimensions of UI elements
    property int titleBarTextLeftMargin: 70
    property int buttonHeight: 24
    property int rowHeight: 26
    property int separatorHeight: 20
    property int borderWidth: 1
    property int scrollBarHeight: 10
    property int progressBarHeight: 6
    property int validIconWidth: 18
    property int colorScaleWidth: 8
    property int splitHandleIndicatorInitialLength: 50
    property int splitHandleIndicatorScaleFactor: 2
    property int splitHandleIndicatorThickness: 2
    property int busyIndicatorWidth: 55
    property int checkBoxWidth: 24
    property int textFieldHeight: 24
    property int radioButtonWidth: 24
    property int radioButtonCheckWidth: 10
    property int fieldWidth: 150
    property int contextMenuWidth: 215
    property int contextMenuWidthTextField: 120
    property int minimumTitleBarHeight: 28
    property int defaultTableWidth: 100
    property int appIconWidth: 128
    property int titeBarButtonWidth: 40
    property int titleBarIconWidth: 20
    property int titleBarButtonIconWidth: 10

    // Time constants
    property int animationDuration: 100
    property int toolTipDelay: 1000
    property int toolTipTimeout: -1
    property int busyIndicatorOpacityDuration: 250

    // Opacity
    property real toolTipBackgroundOpacity: 0.92

    // Font sizes
    property int fontSizeTitleBar: 12
    property int fontSizeHeader: 22
    property int fontSizeBody: 13
    property int fontSizeHistogram: 12
    property int fontSizeValidIcon: 11
    property int fontSizeDialogIcon: 50
    property int fontSizeCheckBoxIcon: 13
    property int fontSizeStepperIcon: 13
    property int fontSizeComboBoxIcon: 12
    property int fontSizeUnacceptableInputIcon: 9
    property int fontSizeFieldButtonIcon: 15
    property int fontSizeTitleBarCloseIcon: 18
    property real headingLetterSpacing: -0.6

    // FontAwesome icons (unicode)
    property string validIcon: "\uf00c"
    property string invalidIcon: "\uf00d"
    property string triangleExclamationIcon: "\uf071"
    property string circleXmarkIcon: "\uf057"
    property string settingsIcon: "\uf1de"
    property string minusIcon: "\uf068"
    property string plusIcon: "\u002b"
    property string chevronDownIcon: "\uf078"
    property string crossIcon: "\uf00d"
    property string calculatorIcon: "\uf1ec"
    property string symbolIcon: "α"
    property string titleBarCloseIcon: "🞨"

    // Default SplitView dimensions
    property int minWidthInputParam: 450
    property int preferredWidthInputParam: 600
    property int minWidthOutputParam: 460
    property int minWidthParam: minWidthInputParam +
                                minWidthOutputParam +
                                spacingM
    property int preferredWidthParam: 1100
    property int minWidthMonteCarlo: 340
    property int minWidthMonteCarloTableHeader: 150
    property int minHeightParam: 300
    property int preferredHeightParam: 390
    property int minHeightResults: 300

    // String constants
    property string fileExtension: "sig"
    property string titleInputParameters: "Input parameters"
    property string titleOutputParameters: "Output parameters"
    property string titleMonteCarlo: "Monte Carlo simulation"
    property string titleEstimatedDistribution: "Estimated output distribution"
    property string titleCombinedUncertainty: "Combined uncertainty"
    property string titleUncertaintyComponents: "Uncertainty components"
    property string buttonTextNew: "New"
    property string buttonTextEdit: "Edit"
    property string buttonTextDelete: "Delete"
    property string buttonTextStart: "Start"
    property string buttonTextStop: "Stop"
    property string buttonTextCancel: "Cancel"
    property string buttonTextClose: "Close"
    property string buttonTextOK: "OK"
    property string tipNewInputParameter: "Add new input parameter"
    property string tipNewOutputParameter: "Add new output parameter"
    property string tipEditInputParameter: "Edit selected input parameter"
    property string tipEditOutputParameter: "Edit selected output parameter"
    property string tipDeleteInputParameter: "Delete selected input parameter"
    property string tipDeleteOutputParameter: "Delete selected output parameter"
    property string tipMonteCarloRunning: "Monte Carlo simulation running"
    property string tipAddInputParamFirst: "Add a new input parameter first"
    property string tipAddOutputParamFirst: "Add a new output parameter first"
    property string tipStartMonteCarlo: "Start Monte Carlo simulation"
    property string tipStopMonteCarlo: "Stop Monte Carlo simulation"
    property string tipShowCorrelations:
        "Define the correlations between the input parameters"
    property string tipSelectValidOutput: "Select a valid output parameter"
    property string unacceptableInputValueText: "Enter a valid value"
    property string tipNewCorrelation: "Add new correlation"
    property string tipAdd2InputParamFirst: "Add two input parameters first"
    property string tipNoDistribution: "No probability distribution defined"
    property string errorIdenticalInputParam:
        "The input parameters must be different"
    property string errorCorrelationNotUnique:
        "The combination of input parameters already exists"
    property string errorConstantParamCor: "No probability distribution defined"

    // App icon url
    property url appIcon_Light:
        "qrc:/qt/qml/Sigma/resources/icons/Icon1024_light.png"
    property url appIcon_Dark:
        "qrc:/qt/qml/Sigma/resources/icons/Icon1024_dark.png"
    property url appIcon: lightTheme ? appIcon_Light : appIcon_Dark

    // C++ properties
    property bool autoSaveProject: appSettings.autoSaveProject
    property bool filePathNotEmpty: projectFileName.length > 0
    property bool inputParamAvailable: inputName.length > 0
    property bool outputLocked: calculation.outputLocked
    property bool outputParamAvailable: outputName.length > 0
    property bool restoreLastProject: appSettings.restoreLastProject
    property bool unsavedChanges: calculation.unsavedChanges
    property int displayTheme: appSettings.displayTheme
    property string inputName: calculation.inputName
    property string outputName: calculation.outputName
    property string outputUnit: calculation.outputUnit
    property string projectFileName: calculation.projectFileName
    property url projectFilePath: calculation.projectFilePath

    // OS
    property bool macOS: ( Qt.platform.os === "osx" ) ||
                         ( Qt.platform.os === "macos" )
    property bool windows: Qt.platform.os === "windows"

    // Window flags
    property int windowFlags:
        Qt.Window |
        (
            macOS ?
              ( Qt.NoTitleBarBackgroundHint | Qt.ExpandedClientAreaHint ) :
              ( windows ? Qt.FramelessWindowHint : Qt.Window )
        )

    property int dialogFlags:
        Qt.Window |
        (
            macOS ?
              ( Qt.FramelessWindowHint | Qt.NoDropShadowWindowHint ) :
              ( windows ? Qt.FramelessWindowHint : Qt.Window )
        )
}
