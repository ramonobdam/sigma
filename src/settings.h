// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

// Settings class to get/set global program and calculation settings. Inherited
// by ApplicationSettings, which handles persistence and QML integration.
class Settings {
    Q_GADGET

public:
    // The names of enum values must begin with a capital letter in order to be
    // accessible from QML. Q_ENUM only works with 'old-school' enum definition
    // and not with enum class.
    enum DisplayTheme { System, Light, Dark };
    Q_ENUM( DisplayTheme )

    Settings();
    Settings& operator= ( const Settings &settings );

    static DisplayTheme getDefaultDisplayTheme();
    static DisplayTheme getDisplayTheme();
    static QUrl getLastProjectFilePath();
    static bool getAutoSaveProject();
    static bool getDefaultAutoSaveProject();
    static bool getDefaultRestoreLastProject();
    static bool getRestoreLastProject();
    static bool setAutoSaveProject( bool autoSaveProject );
    static bool setCSVPrecision( int csvPrecision );
    static bool setDisplayPrecision( int numberOfDigits );
    static bool setDisplayTheme ( DisplayTheme displayTheme );
    static bool setMonteCarloBatchSize( int batchSize );
    static bool setMonteCarloDigits( int numberOfDigits );
    static bool setMonteCarloMaxNumOfBatches ( int maxNumOfBatches );
    static bool setRestoreLastProject( bool restoreLastProject );
    static int getCSVPrecision();
    static int getDefaultCSVPrecision();
    static int getDefaultDisplayPrecision();
    static int getDefaultMonteCarloBatchSize();
    static int getDefaultMonteCarloDigits();
    static int getDefaultMonteCarloMaxNumOfBatches();
    static int getDisplayPrecision();
    static int getMaxCSVPrecision();
    static int getMaxDisplayPrecision();
    static int getMaxMonteCarloBatchSize();
    static int getMaxMonteCarloDigits();
    static int getMaxMonteCarloMaxNumOfBatches();
    static int getMinCSVPrecision();
    static int getMinDisplayPrecision();
    static int getMinMonteCarloBatchSize();
    static int getMinMonteCarloDigits();
    static int getMinMonteCarloMaxNumOfBatches();
    static int getMonteCarloBatchSize();
    static int getMonteCarloDigits();
    static int getMonteCarloMaxNumOfBatches();
    static void setLastProjectFilePath( const QUrl &lastProjectFilePath );
    static void setToDefaults();

private:
    template<typename T>
    struct Setting {
        Setting( T minVal, T defVal, T maxVal ) {
            min = minVal;
            def = defVal;
            max = maxVal;
            value = def;
        }

        bool set( T val ) {
            // Set the value only if it is within the allowable range
            if ( val >= min && val <= max ) {
                value = val;
                return true;
            }
            return false;
        }

        T min;
        T def;
        T max;
        T value;
    };

    inline static QUrl sLastProjectFilePath {};
    inline static Setting<DisplayTheme> sDisplayTheme { System, System, Dark };
    inline static Setting<bool> sAutoSaveProject { false, false, true };
    inline static Setting<bool> sRestoreLastProject { false, false, true };
    inline static Setting<int> sCSVPrecision { 1, 10, 20 };
    inline static Setting<int> sDisplayPrecision { 1, 6, 10 };
    inline static Setting<int> sMonteCarloBatchSize { 100, 100000, 1000000 };
    inline static Setting<int> sMonteCarloDigits { 1, 2, 3 };
    inline static Setting<int> sMonteCarloMaxNumOfBatches { 10, 1000, 100000 };
};

// Create a foreign namespace to make the Settings::DisplayTheme enum availble
// in QML.
namespace DisplayThemeForeign {
    Q_NAMESPACE
    QML_NAMED_ELEMENT( DisplayTheme )
    QML_FOREIGN_NAMESPACE( Settings )
}

#endif // SETTINGS_H
