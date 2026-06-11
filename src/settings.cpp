// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "settings.h"


Settings::Settings() {}


Settings & Settings::operator= ( const Settings &settings ) {
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
    setMonteCarloMaxNumOfBatches( settings.getMonteCarloMaxNumOfBatches() );
    setRestoreLastProject( settings.getRestoreLastProject() );

    return *this;
}


Settings::DisplayTheme Settings::getDefaultDisplayTheme() {
    return sDisplayTheme.def;
}


Settings::DisplayTheme Settings::getDisplayTheme() {
    return sDisplayTheme.value;
}


QUrl Settings::getLastProjectFilePath() {
    return sLastProjectFilePath;
}


bool Settings::getAutoSaveProject() {
    return sAutoSaveProject.value;
}


bool Settings::getDefaultAutoSaveProject() {
    return sAutoSaveProject.def;
}


bool Settings::getDefaultRestoreLastProject() {
    return sRestoreLastProject.def;
}


bool Settings::getRestoreLastProject() {
    return sRestoreLastProject.value;
}


bool Settings::setAutoSaveProject( bool autoSaveProject ) {
    return sAutoSaveProject.set( autoSaveProject );
}


bool Settings::setCSVPrecision( int csvPrecision ) {
    return sCSVPrecision.set( csvPrecision );
}


bool Settings::setDisplayPrecision( int numberOfDigits ) {
    return sDisplayPrecision.set( numberOfDigits );
}


bool Settings::setDisplayTheme( DisplayTheme displayTheme ) {
    return sDisplayTheme.set( displayTheme );
}


bool Settings::setMonteCarloBatchSize( int batchSize ) {
    return sMonteCarloBatchSize.set( batchSize );
}


bool Settings::setMonteCarloDigits( int numberOfDigits ) {
    return sMonteCarloDigits.set( numberOfDigits );
}


bool Settings::setMonteCarloMaxNumOfBatches( int maxNumOfBatches ) {
    return sMonteCarloMaxNumOfBatches.set( maxNumOfBatches );
}


bool Settings::setRestoreLastProject( bool restoreLastProject ) {
    return sRestoreLastProject.set( restoreLastProject );
}


int Settings::getCSVPrecision() {
    return sCSVPrecision.value;
}


int Settings::getDefaultCSVPrecision() {
    return sCSVPrecision.def;
}


int Settings::getDefaultDisplayPrecision() {
    return sDisplayPrecision.def;
}


int Settings::getDefaultMonteCarloBatchSize() {
    return sMonteCarloBatchSize.def;
}


int Settings::getDefaultMonteCarloDigits() {
    return sMonteCarloDigits.def;
}


int Settings::getDefaultMonteCarloMaxNumOfBatches() {
    return sMonteCarloMaxNumOfBatches.def;
}


int Settings::getDisplayPrecision() {
    return sDisplayPrecision.value;
}


int Settings::getMaxCSVPrecision() {
    return sCSVPrecision.max;
}


int Settings::getMaxDisplayPrecision() {
    return sDisplayPrecision.max;
}


int Settings::getMaxMonteCarloBatchSize() {
    return sMonteCarloBatchSize.max;
}


int Settings::getMaxMonteCarloDigits() {
    return sMonteCarloDigits.max;
}


int Settings::getMaxMonteCarloMaxNumOfBatches() {
    return sMonteCarloMaxNumOfBatches.max;
}


int Settings::getMinCSVPrecision() {
    return sCSVPrecision.min;
}


int Settings::getMinDisplayPrecision() {
    return sDisplayPrecision.min;
}


int Settings::getMinMonteCarloBatchSize() {
    return sMonteCarloBatchSize.min;
}


int Settings::getMinMonteCarloDigits() {
    return sMonteCarloDigits.min;
}


int Settings::getMinMonteCarloMaxNumOfBatches() {
    return sMonteCarloMaxNumOfBatches.min;
}


int Settings::getMonteCarloBatchSize() {
    return sMonteCarloBatchSize.value;
}


int Settings::getMonteCarloDigits() {
    return sMonteCarloDigits.value;
}


int Settings::getMonteCarloMaxNumOfBatches() {
    return sMonteCarloMaxNumOfBatches.value;
}


void Settings::setLastProjectFilePath( const QUrl &lastProjectFilePath ) {
    sLastProjectFilePath = lastProjectFilePath;
}


void Settings::setToDefaults() {
    setAutoSaveProject( sAutoSaveProject.def );
    setCSVPrecision( sCSVPrecision.def );
    setDisplayPrecision( sDisplayPrecision.def );
    setDisplayTheme( sDisplayTheme.def );
    setMonteCarloBatchSize( sMonteCarloBatchSize.def );
    setMonteCarloDigits( sMonteCarloDigits.def );
    setMonteCarloMaxNumOfBatches( sMonteCarloMaxNumOfBatches.def );
    setRestoreLastProject( sRestoreLastProject.def );
}
