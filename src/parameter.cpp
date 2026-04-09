// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "parameter.h"
#include "stringutils.h"


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
        Data {},
        mName { name },
        mUnit { unit },
        mNominalValue { nominalValue },
        mLocked { locked },
        mValid { valid }
{}


Parameter::Parameter( const Parameter &par )
    :   QObject { par.parent() },
        Data { par },
        mName { par.getName() },
        mUnit { par.getUnit() },
        mNominalValue { par.getNominalValue() },
        mLocked { par.getLocked() },
        mValid { par.getValid() }
{}


Parameter & Parameter::operator=( const Parameter &par ) {
    if ( this != &par ) {
        Data::operator=( par );
        setParent( par.parent() );
        setName( par.getName() );
        setUnit( par.getUnit() );
        setNominalValue( par.getNominalValue() );
        setLocked( par.getLocked() );
        setValid( par.getValid() );
    }

    return *this;
}


bool Parameter::getLocked() const {
    return mLocked;
}


std::wstring Parameter::getNameStdWString() const {
    return mName.toStdWString();
}


void Parameter::setValid( const bool &valid ) {
    mValid = valid;
}


QString Parameter::getName( bool csvMode ) const {
    return csvMode ? StringUtils::addQuotes( mName ) : mName;
}


QString Parameter::getUnit( const bool csvMode ) const {
    return csvMode ? StringUtils::addQuotes( mUnit ) : mUnit;
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
