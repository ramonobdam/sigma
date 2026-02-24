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


void ApplicationSettings::emitAllSettingsChanged() {
    emit autoSaveProjectChanged();
    emit displayPrecisionChanged();
    emit displayThemeChanged();
    emit lastProjectFilePathChanged();
    emit monteCarloBatchSizeChanged();
    emit monteCarloDigitsChanged();
    emit monteCarloMaxNumBatchesChanged();
    emit restoreLastProjectChanged();
    emit restoreLastProjectChanged();
}


void ApplicationSettings::load() {
    // During loading no 'changed' signals need to be emitted, so base class
    // method can be used.
    Settings::setCSVPrecision(
        mPersistentSettings.value(
            mCSVPrecisionString,
            getDefaultCSVPrecision()
        ).toInt()
    );
    Settings::setDisplayPrecision(
        mPersistentSettings.value(
           mDisplayPrecisionString,
           getDefaultDisplayPrecision()
        ).toInt()
    );
    Settings::setDisplayTheme(
        mPersistentSettings.value(
            mDisplayThemeString,
            getDefaultDisplayTheme()
        ).value<DisplayTheme>()
    );
    Settings::setMonteCarloBatchSize(
        mPersistentSettings.value(
            mMonteCarloBatchSizeString,
            getDefaultMonteCarloBatchSize()
        ).toInt()
    );
    Settings::setMonteCarloMaxNumBatches(
        mPersistentSettings.value(
           mMonteCarloMaxNumOfBatchesString,
           getDefaultMonteCarloMaxNumOfBatches()
        ).toInt()
    );
    Settings::setMonteCarloDigits(
        mPersistentSettings.value(
            mMonteCarloDigitsString,
            getDefaultMonteCarloDigits()
        ).toInt()
    );
    Settings::setLastProjectFilePath(
        mPersistentSettings.value(
            mLastProjectFilePathString,
            getLastProjectFilePath()
        ).value<QUrl>()
    );
    Settings::setAutoSaveProject(
        mPersistentSettings.value(
            mAutoSaveProjectString,
            getAutoSaveProject()
        ).toBool()
    );
    Settings::setRestoreLastProject(
        mPersistentSettings.value(
            mRestoreLastProjectString,
            getRestoreLastProject()
        ).toBool()
    );
}


void ApplicationSettings::save() {
    mPersistentSettings.setValue(
        mCSVPrecisionString,
        getCSVPrecision()
    );
    mPersistentSettings.setValue(
        mDisplayPrecisionString,
        getDisplayPrecision()
    );
    mPersistentSettings.setValue(
        mDisplayThemeString,
        getDisplayTheme()
    );
    mPersistentSettings.setValue(
        mMonteCarloBatchSizeString,
        getMonteCarloBatchSize()
    );
    mPersistentSettings.setValue(
        mMonteCarloMaxNumOfBatchesString,
        getMonteCarloMaxNumOfBatches()
    );
    mPersistentSettings.setValue(
        mMonteCarloDigitsString,
        getMonteCarloDigits()
    );
    mPersistentSettings.setValue(
        mLastProjectFilePathString,
        getLastProjectFilePath()
    );
    mPersistentSettings.setValue(
        mAutoSaveProjectString,
        getAutoSaveProject()
    );
    mPersistentSettings.setValue(
        mRestoreLastProjectString,
        getRestoreLastProject()
    );
    mPersistentSettings.sync();
}


void ApplicationSettings::setAutoSaveProject( const bool &autoSaveProject ) {
    Settings::setAutoSaveProject( autoSaveProject );
    emit autoSaveProjectChanged();
}


void ApplicationSettings::setCSVPrecision( const int &csvPrecision ) {
    Settings::setCSVPrecision( csvPrecision );
    emit csvPrecisionChanged();
}


void ApplicationSettings::setDisplayPrecision( const int &numberOfDigits ) {
    Settings::setDisplayPrecision( numberOfDigits );
    emit displayPrecisionChanged();
}


void ApplicationSettings::setDisplayTheme(const DisplayTheme &theme) {
    Settings::setDisplayTheme( theme );
    emit displayThemeChanged();
}


void ApplicationSettings::setLastProjectFilePath(
    const QUrl &lastProjectFilePath
) {
    Settings::setLastProjectFilePath( lastProjectFilePath );
    emit lastProjectFilePathChanged();
}


void ApplicationSettings::setMonteCarloBatchSize( const int &batchSize ) {
    Settings::setMonteCarloBatchSize( batchSize );
    emit monteCarloBatchSizeChanged();
}


void ApplicationSettings::setMonteCarloDigits( const int &numberOfDigits ) {
    Settings::setMonteCarloDigits( numberOfDigits );
    emit monteCarloDigitsChanged();
}


void ApplicationSettings::setMonteCarloMaxNumBatches(
    const int &maxNumOfBatches
) {
    Settings::setMonteCarloMaxNumBatches( maxNumOfBatches );
    emit monteCarloMaxNumBatchesChanged();
}


void ApplicationSettings::setRestoreLastProject(
    const bool &restoreLastProject
) {
    Settings::setRestoreLastProject( restoreLastProject );
    emit restoreLastProjectChanged();
}


void ApplicationSettings::setToDefaults() {
    Settings::setToDefaults();
    emitAllSettingsChanged();
}
