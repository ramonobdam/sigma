// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

// Base class to get/set program and calculation settings. The formatNumber()
// method formats a double based on the current displayPrecision setting.
class Settings {
    Q_GADGET

public:
    // The names of enum values must begin with a capital letter in order to be
    // accessible from QML. Q_ENUM only works with 'old-school' enum definition
    // and not with enum class.
    enum DisplayTheme { System, Light, Dark };
    Q_ENUM( DisplayTheme )

    Settings();
    virtual ~Settings() = default;

    Settings& operator= ( const Settings &settings );

    virtual DisplayTheme getDisplayTheme() const;
    virtual DisplayTheme getDefaultDisplayTheme() const;
    virtual QUrl getLastProjectFilePath() const;
    virtual bool getAutoSaveProject() const;
    virtual bool getRestoreLastProject() const;
    virtual int getCSVPrecision() const;
    virtual int getDefaultCSVPrecision() const;
    virtual int getDefaultDisplayPrecision() const;
    virtual int getDefaultMonteCarloBatchSize() const;
    virtual int getDefaultMonteCarloDigits() const;
    virtual int getDefaultMonteCarloMaxNumOfBatches() const;
    virtual int getDisplayPrecision() const;
    virtual int getMonteCarloBatchSize() const;
    virtual int getMonteCarloDigits() const;
    virtual int getMonteCarloMaxNumOfBatches() const;
    virtual void setAutoSaveProject( bool autoSaveProject );
    virtual void setCSVPrecision( int csvPrecision );
    virtual void setDisplayPrecision( int numberOfDigits );
    virtual void setDisplayTheme ( DisplayTheme displayTheme );
    virtual void setLastProjectFilePath( const QUrl &lastProjectFilePath );
    virtual void setMonteCarloBatchSize( int batchSize );
    virtual void setMonteCarloDigits( int numberOfDigits );
    virtual void setMonteCarloMaxOfNumBatches ( int maxNumOfBatches );
    virtual void setRestoreLastProject( bool restoreLastProject );
    virtual void setToDefaults();

private:
    const DisplayTheme mDefaultDisplayTheme { System };
    const bool mDefaultAutoSaveProject { false };
    const bool mDefaultRestoreLastProject { false };
    const int mDefaultCSVPrecision { 10 };
    const int mDefaultDisplayPrecision { 6 };
    const int mDefaultMonteCarloBatchSize { 100000 };
    const int mDefaultMonteCarloDigits { 2 };
    const int mDefaultMonteCarloMaxNumOfBatches { 1000 };

    static QUrl mLastProjectFilePath;
    static DisplayTheme mDisplayTheme;
    static bool mAutoSaveProject;
    static bool mRestoreLastProject;
    static int mCSVPrecision;
    static int mDisplayPrecision;
    static int mMonteCarloBatchSize;
    static int mMonteCarloDigits;
    static int mMonteCarloMaxNumOfBatches;

protected:
    QString formatNumber( double number, bool csvMode = false ) const;
    static QString mCSVSeparator;
};

// Create a foreign namespace to make the Settings::DisplayTheme enum availble
// in QML.
namespace DisplayThemeForeign {
    Q_NAMESPACE
    QML_NAMED_ELEMENT( DisplayTheme )
    QML_FOREIGN_NAMESPACE( Settings )
}

#endif // SETTINGS_H
