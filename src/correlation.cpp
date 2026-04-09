// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "settings.h"
#include "stringutils.h"


QStringList Correlation::headerLabels = {
    "Input parameter A",
    "Input parameter B",
    "Correlation coefficient"
};

QList<int> Correlation::columnWidths = { 155, 155, 158 };

ModelControl<Correlation *> Correlation::mCorrelationModel = \
    ModelControl<Correlation *>();

QString Correlation::mCorrelationsHeaderString = "Correlations:";
QString Correlation::mCorrelationString = "correlation";
QString Correlation::mIdInputAString = "IdInputParameterA";
QString Correlation::mIdInputBString = "IdInputParameterB";


Correlation::Correlation(
    QObject *parent,
    InputParameter *inputParameterA,
    InputParameter *inputParameterB,
    const double &correlation
)   :   QObject { parent },
        Data {},
        mInputParameterA { inputParameterA },
        mInputParameterB { inputParameterB },
        mCorrelation { correlation }
{}


Correlation::Correlation( const Correlation &cor )
    :   QObject { cor.parent() },
        Data { cor },
        mInputParameterA { cor.getInputParameterA() },
        mInputParameterB { cor.getInputParameterB() },
        mCorrelation { cor.getCorrelation() }
{}


Correlation & Correlation::operator= ( const Correlation &cor ) {
    if ( this != &cor ) {
        Data::operator=( cor );
        setParent( cor.parent() );
        setInputParameterA( cor.getInputParameterA() );
        setInputParameterB( cor.getInputParameterB() );
        setCorrelation( cor.getCorrelation() );
    }

    return *this;
}


bool Correlation::operator== ( const Correlation &cor ) const {
    // Two Correlations are considered equal when both reference the same
    // InputParameters (in arbitrary order) and their correlation coefficient is
    // equal.
    return (
        mCorrelation == cor.getCorrelation() &&
        (
            (
                mInputParameterA == cor.getInputParameterA() &&
                mInputParameterB == cor.getInputParameterB()
            )
            ||
            (
                mInputParameterA == cor.getInputParameterB() &&
                mInputParameterB == cor.getInputParameterA()
            )
        )
    );
}


bool Correlation::operator!= ( const Correlation &cor ) const {
    return !( *this == cor );
}


Correlation * Correlation::addToModel() {
    if ( correlationIsUnique( this ) ) {
        return mCorrelationModel.appendRow( *this );
    }
    return nullptr;
}


Correlation * Correlation::updateSelectedModelRow() {
    Correlation *currentCorrelation { mCorrelationModel.getSelectedRow() };
    if ( currentCorrelation ) {
        mCorrelationModel.updateSelectedRow( *this );
        return currentCorrelation;
    }
    return nullptr;
}


DataType Correlation::dataType() const {
    return DataType::Correlation;
}


InputParameter * Correlation::getInputParameterA() const {
    return mInputParameterA;
}


InputParameter * Correlation::getInputParameterB() const {
    return mInputParameterB;
}


QJsonObject Correlation::toJson() const {
    QJsonObject json {};
    json[ mIdString ] = getId().toString();
    json[ mIdInputAString ] = getInputParameterAId().toString();
    json[ mIdInputBString ] = getInputParameterBId().toString();
    json[ mCorrelationString ] = getCorrelation();
    return json;
}


QString Correlation::getName( bool csvMode ) const {
    // A corralation does not have a name, return the name of the two
    // InputParameters as unique identifier
    return getInputParameterNameA( csvMode ) +
           "_" +
           getInputParameterNameB( csvMode);
}


QString Correlation::toCSVString() const {
    QStringList resultList {};
    for ( int i { 0 }; i < columnCount(); ++i ) {
        resultList.append( get( i, true ).toString() );
    }
    return resultList.join( StringUtils::csvSeparator );
}


QVariant Correlation::get( int column, bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    switch ( column ) {
        case 0:
            return getInputParameterNameA( csvMode );
        case 1:
            return getInputParameterNameB( csvMode );
        case 2:
            return QVariant(
                StringUtils::doubleToString( getCorrelation(), precision )
            );
        default:
            return QVariant();
    }
}


QVariant Correlation::headerData( int column ) const {
    if ( column >= 0 && column < headerLabels.size() ) {
        return headerLabels[ column ];
    }
    return QVariant();
}


QUuid Correlation::getInputParameterAId() const {
    if ( mInputParameterA ) {
        return mInputParameterA->getId();
    }
    else {
        return QUuid();
    }
}


QUuid Correlation::getInputParameterBId() const {
    if ( mInputParameterB ) {
        return mInputParameterB->getId();
    }
    else {
        return QUuid();
    }
}


void Correlation::updateFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ mIdString ]; v.isString() ) {
        setId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ mIdInputAString ]; v.isString() ) {
        setInputParameterAById( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ mIdInputBString ]; v.isString() ) {
        setInputParameterBById( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ mCorrelationString ]; v.isDouble() ) {
        setCorrelation( v.toDouble() );
    }
}


QString Correlation::getInputParameterNameA( const bool csvMode ) const {
    if ( mInputParameterA ) {
        return mInputParameterA->getName( csvMode );
    }
    else {
        return QString( "" );
    }
}


QString Correlation::getInputParameterNameB( const bool csvMode ) const {
    if ( mInputParameterB ) {
        return mInputParameterB->getName( csvMode );
    }
    else {
        return QString( "" );
    }
}


bool Correlation::isUnique( const bool &checkCurrentSelection ) const {
    return correlationIsUnique( this, checkCurrentSelection );
}


bool Correlation::setInputParameterAByName( const QString &name ) {
    InputParameter *parameter {
        InputParameter::getInputParameterByName( name )
    };
    if ( parameter ) {
        setInputParameterA( parameter );
        return true;
    }
    return false;
}


bool Correlation::setInputParameterBByName( const QString &name ) {
    InputParameter *parameter {
        InputParameter::getInputParameterByName( name )
    };
    if ( parameter ) {
        setInputParameterB( parameter );
        return true;
    }
    return false;
}


double Correlation::getCorrelation() const {
    return mCorrelation;
}


void Correlation::setCorrelation( const double &correlation ) {
    mCorrelation = correlation;
}


void Correlation::setToSelected() {
    Correlation * const correlation { mCorrelationModel.getSelectedRow() };
    if ( correlation ) {
        *this = *correlation;
    }
    else {
        reset();
    }
}


bool Correlation::getValid() const {
    // Both InputParameters must be different and the coefficient between -1 and
    // 1
    if (
        mInputParameterA &&
        mInputParameterB &&
        mInputParameterA != mInputParameterB &&
        mCorrelation >= -1. &&
        mCorrelation <= 1.
        ) {
        return true;
    }
    return false;
}


int Correlation::columnCount() const {
    return headerLabels.size();
}


void Correlation::reconnectInputParameters() {
    // Set the pointers to both the InputParameters based on the Ids
    setInputParameterAById( getInputParameterAId() );
    setInputParameterBById( getInputParameterBId() );
}


void Correlation::reset() {
    setInputParameterA();
    setInputParameterB();
    setCorrelation();
}


void Correlation::set( int column, const QVariant &value ) {
    switch ( column ) {
        case 0:
            setInputParameterAByName( value.toString() );
            break;
        case 1:
            setInputParameterAByName( value.toString() );
            break;
        case 2:
            setCorrelation( value.toDouble() );
            break;
    }
}


void Correlation::setInputParameterA( InputParameter *inputParameter ) {
    mInputParameterA = inputParameter;
}


void Correlation::setInputParameterB( InputParameter *inputParameter ) {
    mInputParameterB = inputParameter;
}

void Correlation::setInputParameterAById( const QUuid &id ) {
    if ( !id.isNull( ) ) {
        InputParameter *param { InputParameter::getInputParameterById( id ) };
        if ( param ) {
            setInputParameterA( param );
        }
    }
}


void Correlation::setInputParameterBById( const QUuid &id ) {
    if ( !id.isNull( ) ) {
        InputParameter *param { InputParameter::getInputParameterById( id ) };
        if ( param) {
            setInputParameterB( param );
        }
    }
}


Correlation * Correlation::getCorrelation(
    InputParameter *inputParamA,
    InputParameter *inputParamB
) {
    // Return a pointer to the correlation between inputParameterA and
    // inputParameterB if it exists
    if ( inputParamA && inputParamB ) {
        QList<Correlation *> correlations { mCorrelationModel.getAllRows() };
        for ( Correlation *correlation : correlations ) {
            InputParameter *corParA { correlation->getInputParameterA() };
            InputParameter *corParB { correlation->getInputParameterB() };
            if ( corParA && corParB ) {
                if (
                    ( inputParamA == corParA && inputParamB == corParB ) ||
                    ( inputParamA == corParB && inputParamB == corParA )
                ) {
                    // Correlation found
                    return correlation;
                }
            }
        }
    }
    return nullptr;
}


Correlation Correlation::fromJson(
    const QJsonObject &json,
    const bool &addToModel,
    QObject *parent
) {
    Correlation correlation { Correlation( parent ) };
    correlation.updateFromJson( json );

    if ( addToModel ) {
        correlation.addToModel();
    }

    return correlation;
}


ModelControl<Correlation *> * Correlation::getCorrelationModel() {
    return &mCorrelationModel;
}


QJsonArray Correlation::correlationsToJson() {
    QJsonArray correlationsArray {};
    for ( Correlation * &correlation : mCorrelationModel.getAllRows() ) {
        correlationsArray.append( correlation->toJson() );
    }
    return correlationsArray;
}


QString Correlation::correlationsToCSVString() {
    QString result { mCorrelationsHeaderString + StringUtils::endl };
    result += headerLabels.join( StringUtils::csvSeparator ) + StringUtils::endl;
    for ( Correlation * &correlation : mCorrelationModel.getAllRows() ) {
        result += correlation->toCSVString() + StringUtils::endl;
    }
    return result;
}


bool Correlation::correlationIsUnique(
    const Correlation *newCorrelation,
    const bool &checkCurrentSelection
) {
    // Is the same correlation already stored in the model? When
    // checkCurrentSelection = True, the correlation is allowed to be the same
    // as the currently selected one.
    InputParameter *newParamA { newCorrelation->getInputParameterA() };
    InputParameter *newParamB { newCorrelation->getInputParameterB() };
    Correlation *correlation { getCorrelation( newParamA, newParamB) };
    Correlation *selectedCorrelation { mCorrelationModel.getSelectedRow() };

    if ( correlation && !(
                            checkCurrentSelection &&
                            correlation == selectedCorrelation
                        )
    ) {
        return false;
    }
    return true;
}


bool Correlation::inputParameterCorrelated( InputParameter *inputParameter ) {
    // Is the InputParameter part of the correlations table?
    QList<Correlation *> correlations { mCorrelationModel.getAllRows() };
    for ( Correlation *correlation : correlations ) {
        InputParameter *paramA { correlation->getInputParameterA() };
        InputParameter *paramB { correlation->getInputParameterB() };
        if (
            ( paramA && paramA == inputParameter ) ||
            ( paramB && paramB == inputParameter )
        ) {
            return true;
        }
    }
    return false;
}


bool Correlation::removeSelectedModelRow() {
    Correlation *correlation { mCorrelationModel.getSelectedRow() };
    if ( correlation ) {
        return mCorrelationModel.removeSelectedRow();
    }
    return false;
}


void Correlation::removeCorrelatedInputParameter(
    InputParameter *inputParameter
) {
    // Remove all correlations that contain the InputParameter
    if ( inputParameter ) {
        for ( int row { mCorrelationModel.rowCount() - 1 }; row >= 0; --row ) {
            Correlation *correlation { mCorrelationModel.getRow( row ) };
            if ( correlation ) {
                InputParameter *paramA { correlation->getInputParameterA() };
                InputParameter *paramB { correlation->getInputParameterB() };
                if (
                    ( paramA && paramA == inputParameter ) ||
                    ( paramB && paramB == inputParameter )
                ) {
                    mCorrelationModel.removeRow( row );
                }
            }
        }
    }
}


void Correlation::clearModel() {
    mCorrelationModel.clear();
}


void Correlation::correlationsFromJson(
    const QJsonArray &jsonArray,
    QObject *parent
) {
    for ( const QJsonValue &correlation : jsonArray ) {
        fromJson( correlation.toObject(), true, parent );
    }
}


void Correlation::reconnectAllCorrelations() {
    // Set the pointers to both the InputParameters based on the Ids for all
    // stored expressions. Needed when restoring the model states from Json.
    QList<Correlation *> correlations { mCorrelationModel.getAllRows() };
    for ( Correlation *correlation : correlations ) {
        correlation->reconnectInputParameters();
    }
}
