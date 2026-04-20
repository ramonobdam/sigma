// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <QtAssert>


ModelControl<Correlation *> Correlation::mCorrelationModel = {};


Correlation::Correlation(
    QObject *parent,
    InputParameter *inputParameterA,
    InputParameter *inputParameterB,
    double correlation
)   :   QObject { parent },
        Data {},
        mInputParameterA { inputParameterA },
        mInputParameterB { inputParameterB },
        mInputParameterAId {},
        mInputParameterBId {},
        mCorrelation { correlation }
{}


Correlation::Correlation( const Correlation &cor )
    :   QObject { cor.parent() },
        Data { cor },
        mInputParameterA { cor.getInputParameterA() },
        mInputParameterB { cor.getInputParameterB() },
        mInputParameterAId { cor.getInputParameterAId() },
        mInputParameterBId { cor.getInputParameterBId() },
        mCorrelation { cor.getCorrelation() }
{}


Correlation & Correlation::operator= ( const Correlation &cor ) {
    if ( this != &cor ) {
        Data::operator=( cor );
        setParent( cor.parent() );
        setInputParameterA( cor.getInputParameterA() );
        setInputParameterB( cor.getInputParameterB() );
        setInputParameterAId( cor.getInputParameterAId() );
        setInputParameterBId( cor.getInputParameterBId() );
        setCorrelation( cor.getCorrelation() );
    }

    return *this;
}


bool Correlation::operator== ( const Correlation &cor ) const {
    // Two Correlations are considered equal when both reference the same
    // InputParameters (in arbitrary order) and their correlation coefficient is
    // equal.
    return (
        sameInputParameters( this, &cor ) &&
        mCorrelation == cor.getCorrelation()
    );
}


bool Correlation::operator!= ( const Correlation &cor ) const {
    return !( *this == cor );
}


Correlation * Correlation::appendToModel() {
    // Append this Correlation to the model if it is unique
    return insertIntoModel( mCorrelationModel.rowCount() );
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
    json[ sIdString ] = getId().toString();
    json[ sIdInputAString ] = getInputParameterAId().toString();
    json[ sIdInputBString ] = getInputParameterBId().toString();
    json[ sCorrelationString ] = getCorrelation();
    return json;
}


QString Correlation::getName( bool csvMode ) const {
    // A corralation does not have a name, return the name of the two
    // InputParameters
    return getInputParameterNameA( csvMode ) +
           "_" +
           getInputParameterNameB( csvMode );
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
    return mInputParameterAId;
}


QUuid Correlation::getInputParameterBId() const {
    return mInputParameterBId;
}


bool Correlation::getValid() const {
    // Both InputParameters must be different and the coefficient between -1 and
    // 1
    return (
        !mInputParameterAId.isNull() &&
        !mInputParameterBId.isNull() &&
        mInputParameterAId != mInputParameterBId &&
        mCorrelation >= -1. &&
        mCorrelation <= 1.
    );
}


int Correlation::columnCount() const {
    return headerLabels.size();
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
            setInputParameterBByName( value.toString() );
            break;
        case 2:
            setCorrelation( value.toDouble() );
            break;
    }
}


void Correlation::setInputParameterA( InputParameter *inputParameter ) {
    mInputParameterA = inputParameter;
    mInputParameterAId = inputParameter ? inputParameter->getId() : QUuid {};
}


void Correlation::setInputParameterAById( const QUuid &id ) {
    if ( !id.isNull( ) ) {
        InputParameter *param { InputParameter::getById( id ) };
        if ( param ) {
            setInputParameterA( param );
        }
    }
}


void Correlation::setInputParameterAId( const QUuid &id ) {
    mInputParameterAId = id;
}


void Correlation::setInputParameterB( InputParameter *inputParameter ) {
    mInputParameterB = inputParameter;
    mInputParameterBId = inputParameter ? inputParameter->getId() : QUuid {};
}


void Correlation::setInputParameterBById( const QUuid &id ) {
    if ( !id.isNull( ) ) {
        InputParameter *param { InputParameter::getById( id ) };
        if ( param) {
            setInputParameterB( param );
        }
    }
}


void Correlation::setInputParameterBId( const QUuid &id ) {
    mInputParameterBId = id;
}


void Correlation::updateFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ sIdString ]; v.isString() ) {
        setId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ sIdInputAString ]; v.isString() ) {
        setInputParameterAId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ sIdInputBString ]; v.isString() ) {
        setInputParameterBId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ sCorrelationString ]; v.isDouble() ) {
        setCorrelation( v.toDouble() );
    }
}


QString Correlation::getInputParameterNameA( bool csvMode ) const {
    return mInputParameterA ? mInputParameterA->getName( csvMode ) : "";
}


QString Correlation::getInputParameterNameB( bool csvMode ) const {
    return mInputParameterB ? mInputParameterB->getName( csvMode ) : "";

}


bool Correlation::isUnique( bool checkCurrentSelection ) const {
    return correlationIsUnique( this, checkCurrentSelection );
}


bool Correlation::setInputParameterAByName( const QString &name ) {
    InputParameter *parameter { InputParameter::getByName( name ) };
    if ( parameter ) {
        setInputParameterA( parameter );
        return true;
    }
    return false;
}


bool Correlation::setInputParameterBByName( const QString &name ) {
    InputParameter *parameter { InputParameter::getByName( name ) };
    if ( parameter ) {
        setInputParameterB( parameter );
        return true;
    }
    return false;
}


double Correlation::getCorrelation() const {
    return mCorrelation;
}


void Correlation::setCorrelation( double correlation ) {
    mCorrelation = correlation;
}


void Correlation::setToSelected() {
    // Set this Correlation to the values of the selected Correlation, or reset
    // is no Correlation is selected
    const Correlation *correlation { mCorrelationModel.getSelected() };
    if ( correlation ) {
        *this = *correlation;
    }
    else {
        reset();
    }
}

Correlation * Correlation::getById( const QUuid &id ) {
    return mCorrelationModel.getById( id );
}


Correlation * Correlation::getCorrelation(
    const QUuid &idA,
    const QUuid &idB
) {
    // Return a pointer to the correlation between the InputParameters with idA
    // and idB, if it exists
    if ( !idA.isNull() && !idB.isNull() ) {
        for ( Correlation *correlation : getAll() ) {
            QUuid corParAid { correlation->getInputParameterAId() };
            QUuid corParBid { correlation->getInputParameterBId() };
            if (
                ( idA == corParAid && idB == corParBid ) ||
                ( idA == corParBid && idB == corParAid )
            ) {
                // Correlation found
                return correlation;
            }
        }
    }
    return nullptr;
}


Correlation * Correlation::getSelected() {
    return mCorrelationModel.getSelected();
}


Correlation Correlation::fromJson(
    const QJsonObject &json,
    bool appendToModel,
    QObject *parent
) {
    Correlation correlation { Correlation( parent ) };
    correlation.updateFromJson( json );

    if ( appendToModel ) {
        correlation.appendToModel();
    }

    return correlation;
}


ModelControl<Correlation *> * Correlation::getCorrelationModel() {
    return &mCorrelationModel;
}


QJsonArray Correlation::correlationsToJson() {
    QJsonArray correlationsArray {};
    for ( const Correlation *correlation : getAll() ) {
        correlationsArray.append( correlation->toJson() );
    }
    return correlationsArray;
}


QJsonObject Correlation::currentJson( const QUuid &id ) {
    Correlation *correlation { getById( id ) };
    return correlation ? correlation->toJson() : QJsonObject {};
}


const QList<Correlation *> & Correlation::getAll() {
    return mCorrelationModel.getAllRows();
}


QList<Correlation *> Correlation::getCorrelationsForInputParameter(
    const QUuid &id
) {
    // Return the correlations that reference the InputParameter with id
    QList<Correlation *> result {};
    if ( !id.isNull() ) {
        for ( Correlation *correlation : getAll() ) {
            if (
                correlation->getInputParameterAId() == id ||
                correlation->getInputParameterBId() == id
            ) {
                result.append( correlation );
            }
        }
    }
    return result;
}


QString Correlation::correlationsToCSVString() {
    QString result { sCorrelationsHeaderString + StringUtils::endl };
    result += headerLabels.join( StringUtils::csvSeparator ) +
              StringUtils::endl;
    for ( const Correlation *correlation : getAll() ) {
        result += correlation->toCSVString() + StringUtils::endl;
    }
    return result;
}


QUuid Correlation::getSelectedId() {
    return mCorrelationModel.getSelectedId();
}


bool Correlation::correlationIsUnique(
    const Correlation *newCorrelation,
    bool checkCurrentSelection
) {
    // Is the same correlation already stored in the model? When
    // checkCurrentSelection = True, the correlation is allowed to be the same
    // as the currently selected one (i.e. can use the same InputParameters).
    Correlation *correlation {
        getCorrelation(
            newCorrelation->getInputParameterAId(),
            newCorrelation->getInputParameterBId()
        )
    };
    Correlation *selectedCorrelation { mCorrelationModel.getSelected() };

    if ( correlation && !(
                            checkCurrentSelection &&
                            selectedCorrelation &&
                            sameInputParameters(
                                correlation,
                                selectedCorrelation
                            )
                        )
    ) {
        return false;
    }
    return true;
}


bool Correlation::remove( const QUuid &id ) {
    bool correlationRemoved { mCorrelationModel.removeById( id ) };
    Q_ASSERT_X(
        correlationRemoved,
        "Correlation::remove",
        "Correlation could not be removed"
    );
    return correlationRemoved;
}


bool Correlation::update( const QUuid &id, Correlation *correlation ) {
    // The new correlation has to be unique or use the same InputParameters as
    // the original correlation
    Correlation *originalCorrelation { getById( id ) };
    Q_ASSERT_X(
        originalCorrelation,
        "Correlation::update",
        "Original correlation not found"
    );
    if (
        correlation &&
        originalCorrelation &&
        (
            correlationIsUnique( correlation, false ) ||
            sameInputParameters( correlation, originalCorrelation )
        )
    ) {
        bool correlationUpdated =
            mCorrelationModel.updateById( id, *correlation );
        Q_ASSERT_X(
            correlationUpdated,
            "Correlation::update",
            "Correlation could not be updated"
        );
        return correlationUpdated;
    }
    return false;
}


int Correlation::getRowIndex( const QUuid &id ) {
    return mCorrelationModel.getRowIndex( id );
}


void Correlation::applyDiff( const JsonDiff &diff ) {
    if ( diff.after.isEmpty() ) {
        // Target state is empty — remove from model
        bool correlationRemoved { remove( diff.objectId ) };
        Q_ASSERT_X(
            correlationRemoved,
            "Correlation::applyDiff",
            "After empty - Correlation could not be removed"
        );
    } else if ( diff.before.isEmpty() ) {
        // Before state is empty — insert in the previous position
        Correlation correlation { fromJson( diff.after, false ) };
        bool correlationInserted = correlation.insertIntoModel( diff.row );
        Q_ASSERT_X(
            correlationInserted,
            "Correlation::applyDiff",
            "Before empty - Correlation could not be inserted"
        );
    } else {
        // Update the existing correlation in the model
        Correlation afterCorrelation { *( getById( diff.objectId ) ) };
        afterCorrelation.updateFromJson( diff.after );
        bool correlationUpdated { update( diff.objectId, &afterCorrelation ) };
        Q_ASSERT_X(
            correlationUpdated,
            "Correlation::applyDiff",
            "Update - Correlation could not be updated"
        );

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


void Correlation::notifyInputParameterChanged( const QUuid &id ) {
    // Emit the 'dataChanged' signal for rows containing a Correlation that
    // references the changed InputParameter
    QList<Correlation *> corrs { getCorrelationsForInputParameter( id ) };
    for ( Correlation *correlation : corrs ) {
        mCorrelationModel.emitIdChanged( correlation->getId() );
    }
}


void Correlation::onDisplayPrecisionChanged() {
    mCorrelationModel.emitAllDataChanged();
}


void Correlation::reconnectAllCorrelations() {
    // Set the pointers to both the InputParameters based on the Ids for all
    // stored correlations. Needed when restoring the model states from Json.
    for ( Correlation *correlation : getAll() ) {
        correlation->reconnectInputParameters();
    }
}


void Correlation::setSelectionLocked( bool locked ) {
    mCorrelationModel.setSelectionLocked( locked );
}


Correlation * Correlation::insertIntoModel( int row ) {
    // Insert this Correlation into the model at row if it is unique
    if ( correlationIsUnique( this ) ) {
        const int boundedRow { qBound( 0, row, mCorrelationModel.rowCount() ) };
        return mCorrelationModel.insertRow( boundedRow, *this );
    }
    return nullptr;
}


void Correlation::reconnectInputParameters() {
    // Set the pointers to both the InputParameters based on the Ids
    setInputParameterAById( mInputParameterAId );
    setInputParameterBById( mInputParameterBId );
}


bool Correlation::sameInputParameters(
    const Correlation *corrA,
    const Correlation *corrB
) {
    // Do two correlations reference the same InputParameters (in arbitrary
    // order)
    if ( corrA && corrB ) {
        if (
            (
                corrA->getInputParameterAId() == corrB->getInputParameterAId() &&
                corrA->getInputParameterBId() == corrB->getInputParameterBId()
            )
            ||
            (
                corrA->getInputParameterAId() == corrB->getInputParameterBId() &&
                corrA->getInputParameterBId() == corrB->getInputParameterAId()
            )
        ) {
            return true;
        }
    }
    return false;
}
