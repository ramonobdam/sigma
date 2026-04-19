// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <QJsonValue>
#include <QtAssert>
#include <QtMinMax>


ModelControl<InputParameter *> InputParameter::mInputModel = {};


InputParameter::InputParameter( QObject *parent )
    :   Parameter { parent },
        mStdUncertainty { 1. },
        mDistribution { Distribution::Type::normal },
        mDOFInfinite { true },
        mDOF { 1 },
        mSymbolValue {}
{}


InputParameter::InputParameter( const InputParameter &ip )
    :   Parameter { ip },
        mStdUncertainty { ip.getStdUncertainty() },
        mDistribution { ip.getDistribution() },
        mDOFInfinite { ip.getDOFInfinite() },
        mDOF { ip.getDOF() },
        mSymbolValue { ip.getSymbolValue() }
{}


InputParameter & InputParameter::operator= ( const InputParameter &ip ) {
    if ( this != &ip ) {
        Parameter::operator=( ip );
        setStdUncertainty( ip.getStdUncertainty() );
        setDistribution( ip.getDistribution() );
        setDOFInfinite( ip.getDOFInfinite() );
        setDOF( ip.getDOF() );
        setSymbolValue( ip.getSymbolValue() );
    }

    return *this;
}


bool InputParameter::operator== ( const InputParameter &ip ) const {
    // Two InputParameters are considered equal when the output estimate and
    // Monte Carlo results of an OutputParameter that uses either of them does
    // not change:
    //  - The InputParameter unit can be updated without the need to recompile
    //    OutputParameters that use it.
    //  - The InputParameter names may differ in capitalization because ExprTk
    //    does not make a distinction between the upper/lower case version of a
    //    symbol.
    return (
        mName.toLower() == ip.getName().toLower() &&
        mNominalValue == ip.getNominalValue() &&
        mStdUncertainty == ip.getStdUncertainty() &&
        mDistribution == ip.getDistribution() &&
        mDOF == ip.getDOF() &&
        mDOFInfinite == ip.getDOFInfinite()
    );
}


bool InputParameter::operator!= ( const InputParameter &ip ) const {
    return !( *this == ip );
}


DataType InputParameter::InputParameter::dataType() const {
    return DataType::InputParameter;
}


Distribution::Type InputParameter::getDistribution() const {
    return mDistribution;
}


InputParameter * InputParameter::appendToModel() {
    // Append this InputParameter to the model and add its name to the symbol
    // table if its name is valid
    return insertIntoModel( mInputModel.rowCount() );
}


QJsonObject InputParameter::toJson() const {
    QJsonObject json {};
    json[ sIdString ] = getId().toString();
    json[ sNameString ] = getName();
    json[ sUnitString ] = getUnit();
    json[ sNominalValueString ] = getNominalValue();
    json[ sStdUncertaintyString ] = getStdUncertainty();
    json[ sDOFInfiniteString ] = getDOFInfinite();
    json[ sDOFString ] = getDOF();
    json[ sDistributionString ] = getDistributionAsString();
    return json;
}


QString InputParameter::getDOFAsString() const {
    if ( mDOFInfinite ) {
        return StringUtils::infinite;
    }
    return QString::number( mDOF );
}


QString InputParameter::toCSVString() const {
    QStringList resultList {};
    for ( int i { 0 }; i < columnCount(); ++i ) {
        resultList.append( get( i, true ).toString() );
    }
    return resultList.join( StringUtils::csvSeparator );
}


QVariant InputParameter::get( int column, bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    switch ( column ) {
        case 0:
            return QVariant( getName( csvMode ) );
        case 1:
            return QVariant( getUnit( csvMode ) );
        case 2:
            return QVariant(
                StringUtils::doubleToString( getNominalValue(), precision )
            );
        case 3:
            return QVariant(
                StringUtils::doubleToString( getStdUncertainty(), precision )
            );
        case 4:
            return QVariant( getDistributionAsString() );
        case 5:
            return QVariant( getDOFAsString() );
        default:
            return QVariant();
    }
}


QVariant InputParameter::headerData( int column ) const {
    if ( column >= 0 && column < headerLabels.size() ) {
        return headerLabels[ column ];
    }
    return QVariant();
}


Distribution::InvCDF InputParameter::getInvCDF() const {
    return Distribution::getInvCDF(
        mDistribution,
        mNominalValue,
        mStdUncertainty,
        mDOF,
        mDOFInfinite
    );
}


double InputParameter::getSymbolValue() const {
    return mSymbolValue;
}


int InputParameter::columnCount() const {
    return headerLabels.size();
}


void InputParameter::resetSymbolValue() {
    mSymbolValue = mNominalValue;
}


void InputParameter::set( int column, const QVariant &value ) {
    switch ( column ) {
        case 0:
            setName( value.toString() );
            break;
        case 1:
            setUnit( value.toString() );
            break;
        case 2:
            setNominalValue( value.toDouble() );
            break;
        case 3:
            setStdUncertainty( value.toDouble() );
            break;
        case 4:
            setDistribution( value.toString() );
            break;
        case 5:
            setDOFInfinite( value.toBool() );
            break;
        case 6:
            setDOF( value.toInt() );
            break;
    }
}


void InputParameter::setDistribution( Distribution::Type distribution ) {
    mDistribution = distribution;
}


void InputParameter::setSymbolValue( double value ) {
    mSymbolValue = value;
}


void InputParameter::updateFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ sIdString ]; v.isString() ) {
        setId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ sNameString ]; v.isString() ) {
        setName( v.toString() );
    }
    if ( const QJsonValue v = json[ sUnitString ]; v.isString() ) {
        setUnit( v.toString() );
    }
    if ( const QJsonValue v = json[ sNominalValueString ]; v.isDouble() ) {
        setNominalValue( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sStdUncertaintyString ]; v.isDouble() ) {
        setStdUncertainty( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sDOFInfiniteString ]; v.isBool() ) {
        setDOFInfinite( v.toBool() );
    }
    if ( const QJsonValue v = json[ sDOFString ]; v.isDouble() ) {
        setDOF( v.toInt() );
    }
    if ( const QJsonValue v = json[ sDistributionString ]; v.isString() ) {
        setDistribution( v.toString() );
    }
}


QString InputParameter::getDistributionAsString() const {
    return Distribution::distributionToString( mDistribution );
}


bool InputParameter::getDOFInfinite() const {
    return mDOFInfinite;
}


double InputParameter::getStdUncertainty() const {
    return mStdUncertainty;
}


int InputParameter::getDOF() const {
    return mDOF;
}


void InputParameter::setDOF( int DOF ) {
    mDOF = DOF;
}


void InputParameter::setDOFInfinite( bool DOFInfinite ) {
    mDOFInfinite = DOFInfinite;
}


void InputParameter::setDistribution( const QString &distributionString ) {
    setDistribution( Distribution::stringToDistribution( distributionString ) );
}


void InputParameter::setStdUncertainty( double stdUncertainty ) {
    mStdUncertainty = stdUncertainty;
}


void InputParameter::setToSelected() {
    const InputParameter *parameter { mInputModel.getSelected() };
    if ( parameter ) {
        *this = *parameter;
    }
}


InputParameter InputParameter::fromJson(
    const QJsonObject &json,
    bool appendToModel,
    QObject *parent
) {
    InputParameter parameter { InputParameter( parent ) };
    parameter.updateFromJson( json );

    if ( appendToModel ) {
        parameter.appendToModel();
    }

    return parameter;
}


InputParameter * InputParameter::getByName( const QString &name ) {
    return mInputModel.getByName( name );
}


InputParameter * InputParameter::getById( const QUuid &id ) {
    return mInputModel.getById( id );
}


InputParameter * InputParameter::getSelected() {
    return mInputModel.getSelected();
}

ModelControl<InputParameter *> * InputParameter::getInputModel() {
    return &mInputModel;
}


QJsonArray InputParameter::parametersToJson() {
    QJsonArray paramArray {};
    for ( const InputParameter *parameter : getAll() ) {
        paramArray.append( parameter->toJson() );
    }
    return paramArray;
}


QJsonObject InputParameter::currentJson( const QUuid &id ) {
    InputParameter *parameter { getById( id ) };
    return parameter ? parameter->toJson() : QJsonObject {};
}


const QList<InputParameter *> & InputParameter::getAll() {
    return mInputModel.getAllRows();
}


QString InputParameter::parametersToCSVString() {
    QString result { sInputParametersHeaderString + StringUtils::endl };
    result += headerLabels.join( StringUtils::csvSeparator ) +
              StringUtils::endl;
    for ( const InputParameter *parameter : getAll() ) {
        result += parameter->toCSVString() + StringUtils::endl;
    }
    return result;
}


QUuid InputParameter::getSelectedId() {
    return mInputModel.getSelectedId();
}


bool InputParameter::inputParameterIsConstant( const QString &name ) {
    InputParameter *inputParameter { getByName( name ) };
    if (
        inputParameter &&
        inputParameter->getDistribution() == Distribution::Type::none
    ) {
        return true;
    }
    return false;
}


bool InputParameter::remove( const QUuid &id ) {
    if ( !id.isNull() ) {
        InputParameter *inputParameter { getById( id ) };
        Q_ASSERT_X(
            inputParameter,
            "InputParameter::remove",
            "Parameter not found"
        );
        if ( inputParameter ) {
            bool symbolRemoved { removeSymbol( inputParameter->getName() ) };
            Q_ASSERT_X(
                symbolRemoved,
                "InputParameter::remove",
                "Symbol could not be removed"
            );
            if ( symbolRemoved ) {
                bool parameterRemoved { mInputModel.removeById( id ) };
                Q_ASSERT_X(
                    parameterRemoved,
                    "InputParameter::remove",
                    "Parameter could not be removed"
                );
                return parameterRemoved;
            }
        }
    }
    return false;
}


bool InputParameter::update( const QUuid &id, InputParameter *parameter ) {
    // The new name has to be valid or equal to the original parameter
    InputParameter *originalParameter { getById( id ) };
    Q_ASSERT_X(
        originalParameter,
        "InputParameter::update",
        "Original parameter not found"
    );
    if (
        parameter &&
        originalParameter &&
        (
            validName( parameter->getName(), false ) ||
            (
                parameter->getName().toLower() ==
                originalParameter->getName().toLower()
            )
        )
    ) {
        bool symbolRemoved { removeSymbol( originalParameter->getName() ) };
        Q_ASSERT_X(
            symbolRemoved,
            "InputParameter::update",
            "Symbol could not be removed"
        );
        if ( symbolRemoved ) {
            InputParameter *newParam {
                mInputModel.updateById( id, *parameter )
            };
            Q_ASSERT_X(
                newParam,
                "InputParameter::update",
                "Parameter could not be updated"
            );
            if ( newParam ) {
                // Notify the Correlation model that an InputParameter changed
                Correlation::notifyInputParameterChanged( id );

                bool symbolAdded { newParam->addToSymbolTable() };
                Q_ASSERT_X(
                    symbolAdded,
                    "InputParameter::update",
                    "Symbol could not be added"
                );
                return symbolAdded;
            }
        }
    }
    return false;
}


bool InputParameter::validName(
    const QString &name,
    bool checkCurrentSelection
) {
    if ( checkCurrentSelection && mInputModel.nameIsSelected( name ) ) {
        return true;
    }
    return ( !symbolExists( name ) && validSymbol( name ) );
}


int InputParameter::getRowIndex( const QUuid &id ) {
    return mInputModel.getRowIndex( id );
}


void InputParameter::addConstantsToSymbolTable() {
    symbolTable.add_constants();
}


void InputParameter::applyDiff( const JsonDiff &diff ) {
    if ( diff.after.isEmpty() ) {
        // Target state is empty — remove from model
        bool parameterRemoved { remove( diff.objectId ) };
        Q_ASSERT_X(
            parameterRemoved,
            "InputParameter::applyDiff",
            "After empty - Parameter could not be removed"
        );
    } else if ( diff.before.isEmpty() ) {
        // Before state is empty — insert in the previous position
        InputParameter parameter { fromJson( diff.after, false ) };
        bool parameterInserted = parameter.insertIntoModel( diff.row );
        Q_ASSERT_X(
            parameterInserted,
            "InputParameter::applyDiff",
            "Before empty - Parameter could not be inserted"
        );
    } else {
        // Remove symbol with before name, add new one with after name and
        // and update the model
        InputParameter afterParam { *( getById( diff.objectId ) ) };
        afterParam.updateFromJson( diff.after );
        bool parameterUpdated { update( diff.objectId, &afterParam ) };
        Q_ASSERT_X(
            parameterUpdated,
            "InputParameter::applyDiff",
            "Update - Parameter could not be updated"
        );
    }
}


void InputParameter::clearModel() {
    mInputModel.clear();
    clearSymbolTable();
    addConstantsToSymbolTable();
}


void InputParameter::clearSymbolTable() {
    symbolTable.clear();
}


void InputParameter::onDisplayPrecisionChanged() {
    mInputModel.emitAllDataChanged();
}


void InputParameter::parametersFromJson(
    const QJsonArray &jsonArray,
    QObject *parent
) {
    for ( const QJsonValue &parameter : jsonArray ) {
        fromJson( parameter.toObject(), true, parent );
    }
}


void InputParameter::setSelectionLocked( bool locked ) {
    mInputModel.setSelectionLocked( locked );
}


InputParameter * InputParameter::insertIntoModel( int row ) {
    // Insert this InputParameter into the model at row and add its name to the
    // symbol table, if its name is valid
    if ( validName( getName() ) ) {
        const int boundedRow { qBound( 0, row, mInputModel.rowCount() ) };
        InputParameter *newParam { mInputModel.insertRow( boundedRow, *this ) };
        if ( newParam ) {
            newParam->addToSymbolTable();
            return newParam;
        }
    }
    return nullptr;
}


bool InputParameter::addToSymbolTable() {
    if ( validSymbol( getName() ) && !symbolExists( getName() ) ) {
        resetSymbolValue();
        return symbolTable.add_variable( getNameStdWString(), mSymbolValue );
    }
    return false;
}


bool InputParameter::removeSymbol( const QString &name ) {
    // Remove the symbol from the table
    return symbolTable.remove_variable( name.toStdWString(), true );
}


bool InputParameter::symbolExists( const QString &name ) {
    return symbolTable.symbol_exists( name.toStdWString() );
}


bool InputParameter::validSymbol( const QString &name ) {
    return symbolTable.valid_symbol( name.toStdWString() );
}
