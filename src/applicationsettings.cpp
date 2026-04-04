// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "applicationsettings.h"


ApplicationSettings::ApplicationSettings( QObject *parent )
    :   QObject { parent },
        mPersistentSettings { QSettings( this ) }
{
    load();
}


ApplicationSettings::~ApplicationSettings() {
    save();
}


void ApplicationSettings::notifyAllChanged() {
    emit autoSaveProjectChanged();
    emit csvPrecisionChanged();
    emit displayPrecisionChanged();
    emit displayThemeChanged();
    emit lastProjectFilePathChanged();
    emit monteCarloBatchSizeChanged();
    emit monteCarloDigitsChanged();
    emit monteCarloMaxNumOfBatchesChanged();
    emit restoreLastProjectChanged();
}


void ApplicationSettings::load() {
    // During loading no 'changed' signals need to be emitted, so base class
    // method can be used.
    Settings::setCSVPrecision(
        mPersistentSettings.value(
            sCSVPrecisionString,
            getDefaultCSVPrecision()
        ).toInt()
    );
    Settings::setDisplayPrecision(
        mPersistentSettings.value(
           sDisplayPrecisionString,
           getDefaultDisplayPrecision()
        ).toInt()
    );
    Settings::setDisplayTheme(
        mPersistentSettings.value(
            sDisplayThemeString,
            getDefaultDisplayTheme()
        ).value<DisplayTheme>()
    );
    Settings::setMonteCarloBatchSize(
        mPersistentSettings.value(
            sMonteCarloBatchSizeString,
            getDefaultMonteCarloBatchSize()
        ).toInt()
    );
    Settings::setMonteCarloMaxOfNumBatches(
        mPersistentSettings.value(
           sMonteCarloMaxNumOfBatchesString,
           getDefaultMonteCarloMaxNumOfBatches()
        ).toInt()
    );
    Settings::setMonteCarloDigits(
        mPersistentSettings.value(
            sMonteCarloDigitsString,
            getDefaultMonteCarloDigits()
        ).toInt()
    );
    Settings::setLastProjectFilePath(
        mPersistentSettings.value(
            sLastProjectFilePathString,
            getLastProjectFilePath()
        ).value<QUrl>()
    );
    Settings::setAutoSaveProject(
        mPersistentSettings.value(
            sAutoSaveProjectString,
            getAutoSaveProject()
        ).toBool()
    );
    Settings::setRestoreLastProject(
        mPersistentSettings.value(
            sRestoreLastProjectString,
            getRestoreLastProject()
        ).toBool()
    );
}


void ApplicationSettings::save() {
    mPersistentSettings.setValue(
        sCSVPrecisionString,
        getCSVPrecision()
    );
    mPersistentSettings.setValue(
        sDisplayPrecisionString,
        getDisplayPrecision()
    );
    mPersistentSettings.setValue(
        sDisplayThemeString,
        getDisplayTheme()
    );
    mPersistentSettings.setValue(
        sMonteCarloBatchSizeString,
        getMonteCarloBatchSize()
    );
    mPersistentSettings.setValue(
        sMonteCarloMaxNumOfBatchesString,
        getMonteCarloMaxNumOfBatches()
    );
    mPersistentSettings.setValue(
        sMonteCarloDigitsString,
        getMonteCarloDigits()
    );
    mPersistentSettings.setValue(
        sLastProjectFilePathString,
        getLastProjectFilePath()
    );
    mPersistentSettings.setValue(
        sAutoSaveProjectString,
        getAutoSaveProject()
    );
    mPersistentSettings.setValue(
        sRestoreLastProjectString,
        getRestoreLastProject()
    );
    mPersistentSettings.sync();
}


void ApplicationSettings::setAutoSaveProject( bool autoSaveProject ) {
    if ( getAutoSaveProject() == autoSaveProject ) return;
    Settings::setAutoSaveProject( autoSaveProject );
    emit autoSaveProjectChanged();
}


void ApplicationSettings::setCSVPrecision( int csvPrecision ) {
    if ( getCSVPrecision() == csvPrecision ) return;
    Settings::setCSVPrecision( csvPrecision );
    emit csvPrecisionChanged();
}


void ApplicationSettings::setDisplayPrecision( int numberOfDigits ) {
    if ( getDisplayPrecision() == numberOfDigits ) return;
    Settings::setDisplayPrecision( numberOfDigits );
    emit displayPrecisionChanged();
}


void ApplicationSettings::setDisplayTheme( DisplayTheme theme) {
    if ( getDisplayTheme() == theme ) return;
    Settings::setDisplayTheme( theme );
    emit displayThemeChanged();
}


void ApplicationSettings::setLastProjectFilePath(
    const QUrl &lastProjectFilePath
) {
    if ( getLastProjectFilePath() == lastProjectFilePath ) return;
    Settings::setLastProjectFilePath( lastProjectFilePath );
    emit lastProjectFilePathChanged();
}


void ApplicationSettings::setMonteCarloBatchSize( int batchSize ) {
    if ( getMonteCarloBatchSize() == batchSize ) return;
    Settings::setMonteCarloBatchSize( batchSize );
    emit monteCarloBatchSizeChanged();
}


void ApplicationSettings::setMonteCarloDigits( int numberOfDigits ) {
    if ( getMonteCarloDigits() == numberOfDigits ) return;
    Settings::setMonteCarloDigits( numberOfDigits );
    emit monteCarloDigitsChanged();
}


void ApplicationSettings::setMonteCarloMaxOfNumBatches( int maxNumOfBatches ) {
    if ( getMonteCarloMaxNumOfBatches() == maxNumOfBatches ) return;
    Settings::setMonteCarloMaxOfNumBatches( maxNumOfBatches );
    emit monteCarloMaxNumOfBatchesChanged();
}


void ApplicationSettings::setRestoreLastProject( bool restoreLastProject ) {
    if ( getRestoreLastProject() == restoreLastProject ) return;
    Settings::setRestoreLastProject( restoreLastProject );
    emit restoreLastProjectChanged();
}


void ApplicationSettings::setToDefaults() {
    Settings::setToDefaults();
    notifyAllChanged();
}
