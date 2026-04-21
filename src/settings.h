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

    static DisplayTheme getDisplayTheme();
    static DisplayTheme getDefaultDisplayTheme();
    static QUrl getLastProjectFilePath();
    static bool getAutoSaveProject();
    static bool getRestoreLastProject();
    static int getCSVPrecision();
    static int getDefaultCSVPrecision();
    static int getDefaultDisplayPrecision();
    static int getDefaultMonteCarloBatchSize();
    static int getDefaultMonteCarloDigits();
    static int getDefaultMonteCarloMaxNumOfBatches();
    static int getDisplayPrecision();
    static int getMonteCarloBatchSize();
    static int getMonteCarloDigits();
    static int getMonteCarloMaxNumOfBatches();
    static void setAutoSaveProject( bool autoSaveProject );
    static void setCSVPrecision( int csvPrecision );
    static void setDisplayPrecision( int numberOfDigits );
    static void setDisplayTheme ( DisplayTheme displayTheme );
    static void setLastProjectFilePath( const QUrl &lastProjectFilePath );
    static void setMonteCarloBatchSize( int batchSize );
    static void setMonteCarloDigits( int numberOfDigits );
    static void setMonteCarloMaxOfNumBatches ( int maxNumOfBatches );
    static void setRestoreLastProject( bool restoreLastProject );
    static void setToDefaults();

private:
    static constexpr DisplayTheme sDefaultDisplayTheme { System };
    static constexpr bool sDefaultAutoSaveProject { false };
    static constexpr bool sDefaultRestoreLastProject { false };
    static constexpr int sDefaultCSVPrecision { 10 };
    static constexpr int sDefaultDisplayPrecision { 6 };
    static constexpr int sDefaultMonteCarloBatchSize { 100000 };
    static constexpr int sDefaultMonteCarloDigits { 2 };
    static constexpr int sDefaultMonteCarloMaxNumOfBatches { 1000 };

    static QUrl sLastProjectFilePath;
    static DisplayTheme sDisplayTheme;
    static bool sAutoSaveProject;
    static bool sRestoreLastProject;
    static int sCSVPrecision;
    static int sDisplayPrecision;
    static int sMonteCarloBatchSize;
    static int sMonteCarloDigits;
    static int sMonteCarloMaxNumOfBatches;
};

// Create a foreign namespace to make the Settings::DisplayTheme enum availble
// in QML.
namespace DisplayThemeForeign {
    Q_NAMESPACE
    QML_NAMED_ELEMENT( DisplayTheme )
    QML_FOREIGN_NAMESPACE( Settings )
}

#endif // SETTINGS_H
