// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "settings.h"


QUrl Settings::sLastProjectFilePath {};
Settings::DisplayTheme Settings::sDisplayTheme {};
bool Settings::sAutoSaveProject {};
bool Settings::sRestoreLastProject {};
int Settings::sCSVPrecision = {};
int Settings::sDisplayPrecision = {};
int Settings::sMonteCarloBatchSize = {};
int Settings::sMonteCarloDigits = {};
int Settings::sMonteCarloMaxNumOfBatches = {};


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
    setMonteCarloMaxOfNumBatches( settings.getMonteCarloMaxNumOfBatches() );
    setRestoreLastProject( settings.getRestoreLastProject() );

    return *this;
}


Settings::DisplayTheme Settings::getDisplayTheme() {
    return sDisplayTheme;
}


Settings::DisplayTheme Settings::getDefaultDisplayTheme() {
    return sDefaultDisplayTheme;
}


QUrl Settings::getLastProjectFilePath() {
    return sLastProjectFilePath;
}


bool Settings::getAutoSaveProject() {
    return sAutoSaveProject;
}


bool Settings::getRestoreLastProject() {
    return sRestoreLastProject;
}


int Settings::getCSVPrecision() {
    return sCSVPrecision;
}


int Settings::getDefaultCSVPrecision() {
    return sDefaultCSVPrecision;
}


int Settings::getDefaultDisplayPrecision() {
    return sDefaultDisplayPrecision;
}


int Settings::getDefaultMonteCarloBatchSize() {
    return sDefaultMonteCarloBatchSize;
}


int Settings::getDefaultMonteCarloDigits() {
    return sDefaultMonteCarloDigits;
}


int Settings::getDefaultMonteCarloMaxNumOfBatches() {
    return sDefaultMonteCarloMaxNumOfBatches;
}


int Settings::getDisplayPrecision() {
    return sDisplayPrecision;
}


int Settings::getMonteCarloBatchSize() {
    return sMonteCarloBatchSize;
}


int Settings::getMonteCarloDigits() {
    return sMonteCarloDigits;
}


int Settings::getMonteCarloMaxNumOfBatches() {
    return sMonteCarloMaxNumOfBatches;
}


void Settings::setAutoSaveProject( bool autoSaveProject ) {
    sAutoSaveProject = autoSaveProject;
}


void Settings::setCSVPrecision( int csvPrecision ) {
    sCSVPrecision = csvPrecision;
}


void Settings::setDisplayPrecision( int numberOfDigits ) {
    sDisplayPrecision = numberOfDigits;
}


void Settings::setDisplayTheme( DisplayTheme displayTheme ) {
    sDisplayTheme = displayTheme;
}


void Settings::setLastProjectFilePath( const QUrl &lastProjectFilePath ) {
    sLastProjectFilePath = lastProjectFilePath;
}


void Settings::setMonteCarloBatchSize( int batchSize ) {
    sMonteCarloBatchSize = batchSize;
}


void Settings::setMonteCarloDigits( int numberOfDigits ) {
    sMonteCarloDigits = numberOfDigits;
}


void Settings::setMonteCarloMaxOfNumBatches( int maxNumOfBatches ) {
    sMonteCarloMaxNumOfBatches = maxNumOfBatches;
}


void Settings::setRestoreLastProject( bool restoreLastProject ) {
    sRestoreLastProject = restoreLastProject;
}


void Settings::setToDefaults() {
    setAutoSaveProject( sDefaultAutoSaveProject );
    setCSVPrecision( sDefaultCSVPrecision );
    setDisplayPrecision( sDefaultDisplayPrecision );
    setDisplayTheme( sDefaultDisplayTheme );
    setMonteCarloBatchSize( sDefaultMonteCarloBatchSize );
    setMonteCarloDigits( sDefaultMonteCarloDigits );
    setMonteCarloMaxOfNumBatches( sDefaultMonteCarloMaxNumOfBatches );
    setRestoreLastProject( sDefaultRestoreLastProject );
}
