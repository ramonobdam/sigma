// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include "settings.h"
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

// Class that loads and saves the application persistent settings
class ApplicationSettings : public QObject, public Settings {
    Q_OBJECT

public:
    ApplicationSettings( QObject *parent = nullptr );
    ~ApplicationSettings();

    Q_INVOKABLE void emitAllSettingsChanged();
    Q_INVOKABLE void load();
    Q_INVOKABLE void save();
    Q_INVOKABLE void setAutoSaveProject(
        const bool &autoSaveProject
    ) override;
    Q_INVOKABLE void setCSVPrecision( const int &csvPrecision ) override;
    Q_INVOKABLE void setDisplayPrecision( const int &numberOfDigits ) override;
    Q_INVOKABLE void setDisplayTheme( const DisplayTheme &theme ) override;
    Q_INVOKABLE void setLastProjectFilePath(
        const QUrl &lastProjectFilePath
    ) override;
    Q_INVOKABLE void setMonteCarloBatchSize( const int &batchSize ) override;
    Q_INVOKABLE void setMonteCarloDigits( const int &numberOfDigits ) override;
    Q_INVOKABLE void setMonteCarloMaxNumBatches(
        const int &maxNumOfBatches
    ) override;
    Q_INVOKABLE void setRestoreLastProject(
        const bool &restoreLastProject
    ) override;
    Q_INVOKABLE void setToDefaults() override;

    Q_PROPERTY(
        bool autoSaveProject
        READ getAutoSaveProject
        NOTIFY autoSaveProjectChanged
    )
    Q_PROPERTY(
        bool restoreLastProject
        READ getRestoreLastProject
        NOTIFY restoreLastProjectChanged
    )
    Q_PROPERTY(
        int csvPrecision
        READ getCSVPrecision
        NOTIFY csvPrecisionChanged
    )
    Q_PROPERTY(
        int displayPrecision
        READ getDisplayPrecision
        NOTIFY displayPrecisionChanged
    )
    Q_PROPERTY(
        DisplayTheme displayTheme
        READ getDisplayTheme
        NOTIFY displayThemeChanged
    )
    Q_PROPERTY(
        int monteCarloBatchSize
        READ getMonteCarloBatchSize
        NOTIFY monteCarloBatchSizeChanged
    )
    Q_PROPERTY(
        int monteCarloDigits
        READ getMonteCarloDigits
        NOTIFY monteCarloDigitsChanged
    )
    Q_PROPERTY(
        int monteCarloMaxNumOfBatches
        READ getMonteCarloMaxNumOfBatches
        NOTIFY monteCarloMaxNumBatchesChanged
    )

signals:
    void autoSaveProjectChanged();
    void csvPrecisionChanged();
    void displayPrecisionChanged();
    void displayThemeChanged();
    void lastProjectFilePathChanged();
    void monteCarloBatchSizeChanged();
    void monteCarloDigitsChanged();
    void monteCarloMaxNumBatchesChanged();
    void restoreLastProjectChanged();

private:
    const QString mAutoSaveProjectString { "autoSaveProject" };
    const QString mCSVPrecisionString { "csvPrecision" };
    const QString mDisplayPrecisionString { "displayPrecision" };
    const QString mDisplayThemeString { "displayTheme" };
    const QString mLastProjectFilePathString { "lastProjectFilePath" };
    const QString mMonteCarloBatchSizeString { "monteCarloBatchSize" };
    const QString mMonteCarloDigitsString { "monteCarloDigits" };
    const QString mMonteCarloMaxNumOfBatchesString {
        "monteCarloMaxNumOfBatches"
    };
    const QString mRestoreLastProjectString { "restoreLastProject" };
    QSettings mPersistentSettings;
};

#endif // APPLICATIONSETTINGS_H
