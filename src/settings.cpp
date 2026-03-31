// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "settings.h"


QString Settings::mCSVSeparator { "," };
QUrl Settings::mLastProjectFilePath {};
Settings::DisplayTheme Settings::mDisplayTheme {};
bool Settings::mAutoSaveProject {};
bool Settings::mRestoreLastProject {};
int Settings::mCSVPrecision = {};
int Settings::mDisplayPrecision = {};
int Settings::mMonteCarloBatchSize = {};
int Settings::mMonteCarloDigits = {};
int Settings::mMonteCarloMaxNumOfBatches = {};


Settings::Settings() {}


Settings& Settings::operator= ( const Settings &settings ) {
    if ( this == &settings ) {
        return *this;
    }

    setAutoSaveProject( settings.getAutoSaveProject() );
    setCSVPrecision( settings.getCSVPrecision() );
    setDisplayPrecision( settings.getDisplayPrecision() );
    setDisplayTheme( settings.getDisplayTheme() );
    setLastProjectFilePath( settings.getLastProjectFilePath() );
    setMonteCarloBatchSize( settings.getMonteCarloBatchSize() );
    setMonteCarloDigits( settings.getMonteCarloDigits() );
    setMonteCarloMaxNumBatches( settings.getMonteCarloMaxNumOfBatches() );
    setRestoreLastProject( settings.getRestoreLastProject() );

    return *this;
}


Settings::DisplayTheme Settings::getDisplayTheme() const {
    return mDisplayTheme;
}


Settings::DisplayTheme Settings::getDefaultDisplayTheme() const {
    return mDefaultDisplayTheme;
}


QUrl Settings::getLastProjectFilePath() const {
    return mLastProjectFilePath;
}


bool Settings::getAutoSaveProject() const {
    return mAutoSaveProject;
}


bool Settings::getRestoreLastProject() const {
    return mRestoreLastProject;
}


int Settings::getCSVPrecision() const {
    return mCSVPrecision;
}


int Settings::getDefaultCSVPrecision() const {
    return mDefaultCSVPrecision;
}


int Settings::getDefaultDisplayPrecision() const {
    return mDefaultDisplayPrecision;
}


int Settings::getDefaultMonteCarloBatchSize() const {
    return mDefaultMonteCarloBatchSize;
}


int Settings::getDefaultMonteCarloDigits() const {
    return mDefaultMonteCarloDigits;
}


int Settings::getDefaultMonteCarloMaxNumOfBatches() const {
    return mDefaultMonteCarloMaxNumOfBatches;
}


int Settings::getDisplayPrecision() const {
    return mDisplayPrecision;
}


int Settings::getMonteCarloBatchSize() const {
    return mMonteCarloBatchSize;
}


int Settings::getMonteCarloDigits() const {
    return mMonteCarloDigits;
}


int Settings::getMonteCarloMaxNumOfBatches() const {
    return mMonteCarloMaxNumOfBatches;
}


void Settings::setAutoSaveProject( const bool &autoSaveProject ) {
    mAutoSaveProject = autoSaveProject;
}


void Settings::setCSVPrecision( const int &csvPrecision ) {
    mCSVPrecision = csvPrecision;
}


void Settings::setDisplayPrecision( const int &numberOfDigits ) {
    mDisplayPrecision = numberOfDigits;
}


void Settings::setDisplayTheme( const DisplayTheme &displayTheme ) {
    mDisplayTheme = displayTheme;
}


void Settings::setLastProjectFilePath( const QUrl &lastProjectFilePath ) {
    mLastProjectFilePath = lastProjectFilePath;
}


void Settings::setMonteCarloBatchSize( const int &batchSize ) {
    mMonteCarloBatchSize = batchSize;
}


void Settings::setMonteCarloDigits( const int &numberOfDigits ) {
    mMonteCarloDigits = numberOfDigits;
}


void Settings::setMonteCarloMaxNumBatches( const int &maxNumOfBatches ) {
    mMonteCarloMaxNumOfBatches = maxNumOfBatches;
}


void Settings::setRestoreLastProject( const bool &restoreLastProject ) {
    mRestoreLastProject = restoreLastProject;
}


void Settings::setToDefaults() {
    setAutoSaveProject( mDefaultAutoSaveProject );
    setCSVPrecision( mDefaultCSVPrecision );
    setDisplayPrecision( mDefaultDisplayPrecision );
    setDisplayTheme( mDefaultDisplayTheme );
    setMonteCarloBatchSize( mDefaultMonteCarloBatchSize );
    setMonteCarloDigits( mDefaultMonteCarloDigits );
    setMonteCarloMaxNumBatches( mDefaultMonteCarloMaxNumOfBatches );
    setRestoreLastProject( mDefaultRestoreLastProject );
}


QString Settings::formatNumber(
    const double &number,
    const bool &csvMode
) const {
    int precision { csvMode ? mCSVPrecision : mDisplayPrecision };
    return QString::number( number, 'g', precision );
}
