// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include "settings.h"
#include <QLatin1StringView>
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

    Q_INVOKABLE void notifyAllChanged();
    Q_INVOKABLE void load();
    Q_INVOKABLE void save();
    Q_INVOKABLE void setAutoSaveProject( bool autoSaveProject ) override;
    Q_INVOKABLE void setCSVPrecision( int csvPrecision ) override;
    Q_INVOKABLE void setDisplayPrecision( int numberOfDigits ) override;
    Q_INVOKABLE void setDisplayTheme( DisplayTheme theme ) override;
    Q_INVOKABLE void setLastProjectFilePath(
        const QUrl &lastProjectFilePath
    ) override;
    Q_INVOKABLE void setMonteCarloBatchSize( int batchSize ) override;
    Q_INVOKABLE void setMonteCarloDigits( int numberOfDigits ) override;
    Q_INVOKABLE void setMonteCarloMaxOfNumBatches(
        int maxNumOfBatches
    ) override;
    Q_INVOKABLE void setRestoreLastProject( bool restoreLastProject ) override;
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
        QUrl lastProjectFilePath
        READ getLastProjectFilePath
        NOTIFY lastProjectFilePathChanged
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
        NOTIFY monteCarloMaxNumOfBatchesChanged
    )

signals:
    void autoSaveProjectChanged();
    void csvPrecisionChanged();
    void displayPrecisionChanged();
    void displayThemeChanged();
    void lastProjectFilePathChanged();
    void monteCarloBatchSizeChanged();
    void monteCarloDigitsChanged();
    void monteCarloMaxNumOfBatchesChanged();
    void restoreLastProjectChanged();

private:
    static constexpr QLatin1StringView sAutoSaveProjectString {
        "autoSaveProject"
    };
    static constexpr QLatin1StringView sCSVPrecisionString { "csvPrecision" };
    static constexpr QLatin1StringView sDisplayPrecisionString {
        "displayPrecision"
    };
    static constexpr QLatin1StringView sDisplayThemeString { "displayTheme" };
    static constexpr QLatin1StringView sLastProjectFilePathString {
        "lastProjectFilePath"
    };
    static constexpr QLatin1StringView sMonteCarloBatchSizeString {
        "monteCarloBatchSize"
    };
    static constexpr QLatin1StringView sMonteCarloDigitsString {
        "monteCarloDigits"
    };
    static constexpr QLatin1StringView sMonteCarloMaxNumOfBatchesString {
        "monteCarloMaxNumOfBatches"
    };
    static constexpr QLatin1StringView sRestoreLastProjectString {
        "restoreLastProject"
    };

    QSettings mPersistentSettings;
};

#endif // APPLICATIONSETTINGS_H
