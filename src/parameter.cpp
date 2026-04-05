// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "parameter.h"


QString Parameter::mNameString = "name";
QString Parameter::mNominalValueString = "nominalValue";
QString Parameter::mUnitString = "unit";
QString Parameter::mValidString = "valid";


Parameter::Parameter( QObject *parent )
    :   Parameter {
            parent,
            {},
            {},
            {},
            {},
            {}
        }
{}


Parameter::Parameter(
    QObject *parent,
    const QString &name,
    const QString &unit,
    const double &nominalValue,
    const bool &locked,
    const bool &valid
)
    :   QObject { parent },
        mName { name },
        mUnit { unit },
        mNominalValue { nominalValue },
        mLocked { locked },
        mValid { valid }
{}


bool Parameter::getLocked() const {
    return mLocked;
}


std::wstring Parameter::getNameStdWString() const {
    return mName.toStdWString();
}


void Parameter::setValid( const bool &valid ) {
    mValid = valid;
}


QString Parameter::getName( const bool csvMode ) const {
    return csvMode ? addQuotes( mName ) : mName;
}


QString Parameter::getUnit( const bool csvMode ) const {
    return csvMode ? addQuotes( mUnit ) : mUnit;
}


bool Parameter::getValid() const {
    return mValid;
}


double Parameter::getNominalValue() const {
    return mNominalValue;
}


void Parameter::setLocked( const bool &locked ) {
    mLocked = locked;
}


void Parameter::setName( const QString &name ) {
    mName = name.trimmed();
}


void Parameter::setNominalValue( const double &nominalValue ) {
    mNominalValue = nominalValue;
}


void Parameter::setUnit( const QString &unit ) {
    mUnit = unit.trimmed();
}
