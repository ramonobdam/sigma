// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "inputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <QJsonValue>


QString InputParameter::mDOFInfiniteString = "DOFInfinite";
QString InputParameter::mDOFString = "DOF";
QString InputParameter::mDistributionString = "distribution";
QString InputParameter::mInputParametersHeaderString = "Input parameters:";
QString InputParameter::mStdUncertaintyString = "stdUncertainty";

ModelControl<InputParameter *> InputParameter::mInputModel = \
    ModelControl<InputParameter *>();

symbol_table_t InputParameter::symbolTable = symbol_table_t();

QStringList InputParameter::headerLabels = {
    "Input name",
    "Unit",
    "Input estimate",
    "Standard uncertainty",
    "Distribution",
    "Degrees of freedom"
};

QString InputParameter::defaultName = "X";

QString InputParameter::infiniteString = "infinite";

QList<int> InputParameter::columnWidths = { 90, 55, 105, 148, 92, 144 };


InputParameter::InputParameter( QObject *parent )
    :   Parameter { parent },
        mStdUncertainty { 1. },
        mDistribution { Distribution::Type::normal },
        mDOFInfinite { true },
        mDOF { 1 },
        mSymbolPtr {}
{}


InputParameter::InputParameter( const InputParameter &ip )
    :   Parameter {
            ip.parent(),
            ip.getName(),
            ip.getUnit(),
            ip.getNominalValue(),
            ip.getLocked(),
            ip.getValid()
        },
        mStdUncertainty { ip.getStdUncertainty() },
        mDistribution { ip.getDistribution() },
        mDOFInfinite { ip.getDOFInfinite() },
        mDOF { ip.getDOF() },
        mSymbolPtr { ip.getSymbolPtr() }
{}


InputParameter::~InputParameter(){}


InputParameter& InputParameter::operator= ( const InputParameter &ip ) {
    if ( this == &ip ) {
        return *this;
    }

    setName( ip.getName() );
    setUnit( ip.getUnit() );
    setNominalValue( ip.getNominalValue() );
    setStdUncertainty( ip.getStdUncertainty() );
    setDistribution( ip.getDistribution() );
    setDOFInfinite( ip.getDOFInfinite() );
    setDOF( ip.getDOF() );
    setSymbolPtr( ip.getSymbolPtr() );
    setLocked( ip.getLocked() );
    setParent( ip.parent() );

    return *this;
}


bool InputParameter::operator== ( const InputParameter &ip ) const {
    // Two InputParameters are considered equal when the output estimate or
    // Monte Carlo results of an OutputParameter that uses either of them does
    // not change.
    // I.e. the InputParameter unitcan be updated without the need to recompile
    // OutputParameters that use it.
    return (
        mName == ip.getName() &&
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



Distribution::Type InputParameter::getDistribution() const {
    return mDistribution;
}


InputParameter * InputParameter::addToModel() {
    if ( validName( getName() ) && addToSymbolTable() ) {
        return mInputModel.appendRow( *this );
    }
    return nullptr;
}


InputParameter * InputParameter::updateSelectedModelRow() {
    if ( validName( getName(), true ) ) {
        InputParameter *currentParameter { mInputModel.getSelectedRow() };
        if ( currentParameter && removeSymbol( currentParameter->getName() ) ) {
            if ( addToSymbolTable() ) {
                mInputModel.updateSelectedRow( *this );
                return currentParameter;
            }
        }
    }
    return nullptr;
}


QJsonObject InputParameter::toJson() const {
    QJsonObject json {};
    json[ mNameString ] = getName();
    json[ mUnitString ] = getUnit();
    json[ mNominalValueString ] = getNominalValue();
    json[ mStdUncertaintyString ] = getStdUncertainty();
    json[ mDOFInfiniteString ] = getDOFInfinite();
    json[ mDOFString ] = getDOF();
    json[ mDistributionString ] = getDistributionAsString();
    return json;
}


QString InputParameter::getDOFAsString() const {
    if ( mDOFInfinite ) {
        return InputParameter::infiniteString;
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


double * InputParameter::getSymbolPtr() const {
    return mSymbolPtr;
}


int InputParameter::columnCount() const {
    return headerLabels.size();
}


void InputParameter::resetSymbolValue() {
    if ( mSymbolPtr ) {
        *mSymbolPtr = getNominalValue();
    }
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
        case 7:
            setSymbolPtr( value.value<double *>() );
            break;
    }
}


void InputParameter::setDistribution( const Distribution::Type &distribution ) {
    mDistribution = distribution;
}


void InputParameter::setSymbolPtr( double *symbolPtr ) {
    mSymbolPtr = symbolPtr;
}


void InputParameter::setSymbolValue( const double &value ) {
    if ( mSymbolPtr ) {
        *mSymbolPtr = value;
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


void InputParameter::setDOF( const int &DOF ) {
    mDOF = DOF;
}


void InputParameter::setDOFInfinite( const bool &DOFInfinite ) {
    mDOFInfinite = DOFInfinite;
}


void InputParameter::setDistribution( const QString &distributionString ) {
    setDistribution( Distribution::stringToDistribution( distributionString ) );
}


void InputParameter::setStdUncertainty( const double &stdUncertainty ) {
    mStdUncertainty = stdUncertainty;
}


void InputParameter::setToSelected() {
    const InputParameter *parameter { mInputModel.getSelectedRow() };
    if ( parameter ) {
        *this = *parameter;
    }
}


InputParameter InputParameter::fromJson(
    const QJsonObject &json,
    const bool &addToModel,
    QObject *parent
) {
    InputParameter parameter { InputParameter( parent ) };

    if ( const QJsonValue v = json[ mNameString ]; v.isString() ) {
        parameter.setName( v.toString() );
    }
    if ( const QJsonValue v = json[ mUnitString ]; v.isString() ) {
        parameter.setUnit( v.toString() );
    }
    if ( const QJsonValue v = json[ mNominalValueString ]; v.isDouble() ) {
        parameter.setNominalValue( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mStdUncertaintyString ]; v.isDouble() ) {
        parameter.setStdUncertainty( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mDOFInfiniteString ]; v.isBool() ) {
        parameter.setDOFInfinite( v.toBool() );
    }
    if ( const QJsonValue v = json[ mDOFString ]; v.isDouble() ) {
        parameter.setDOF( v.toInt() );
    }
    if ( const QJsonValue v = json[ mDistributionString ]; v.isString() ) {
        parameter.setDistribution( v.toString() );
    }

    if ( addToModel ) {
        parameter.addToModel();
    }

    return parameter;
}


InputParameter * InputParameter::getInputParameter( const QString &name ) {
    int rows { mInputModel.rowCount() };
    for ( int row {0}; row < rows; ++row ) {
        InputParameter *parameter { mInputModel.getRow( row ) };
        if (
            parameter &&
            ( parameter->getName().toLower() == name.toLower() )
        ) {
            return parameter;
        }
    }
    return nullptr;
}


InputParameter * InputParameter::removeSelectedModelRow() {
    InputParameter *parameter { mInputModel.getSelectedRow() };
    if (
        parameter &&
        removeSymbol( parameter->getName() ) &&
        mInputModel.removeSelectedRow()
    ) {
        Correlation::removeCorrelatedInputParameter( parameter );
        return parameter;
    }
    return nullptr;
}


ModelControl<InputParameter *> * InputParameter::getInputModel() {
    return &mInputModel;
}


QJsonArray InputParameter::parametersToJson() {
    QJsonArray paramArray {};
    for ( InputParameter * &parameter : mInputModel.getAllRows() ) {
        paramArray.append( parameter->toJson() );
    }
    return paramArray;
}


QString InputParameter::parametersToCSVString() {
    QString result { mInputParametersHeaderString + StringUtils::endl };
    result += headerLabels.join( StringUtils::csvSeparator ) + StringUtils::endl;
    for ( InputParameter * &parameter : mInputModel.getAllRows() ) {
        result += parameter->toCSVString() + StringUtils::endl;
    }
    return result;
}


bool InputParameter::inputParameterIsConstant( const QString &name ) {
    InputParameter *inputParameter { getInputParameter( name ) };
    if (
        inputParameter &&
        inputParameter->getDistribution() == Distribution::Type::none
    ) {
        return true;
    }
    return false;
}


bool InputParameter::validName(
    const QString &name,
    const bool &checkCurrentSelection
) {
    if ( checkCurrentSelection && mInputModel.nameIsSelected( name) ) {
        return true;
    }
    return ( !symbolExists( name ) && validSymbol( name ) );
}


void InputParameter::addConstantsToSymbolTable() {
    symbolTable.add_constants();
}


void InputParameter::clearModel() {
    mInputModel.clear();
    clearSymbolTable();
    addConstantsToSymbolTable();
}


void InputParameter::clearSymbolTable() {
    symbolTable.clear();
}


void InputParameter::parametersFromJson(
    const QJsonArray &jsonArray,
    QObject *parent
) {
    for ( const QJsonValue &parameter : jsonArray ) {
        fromJson( parameter.toObject(), true, parent );
    }
}


bool InputParameter::addToSymbolTable() {
    if ( validSymbol( getName() ) && !symbolExists( getName() ) ) {
        double *x { new double( getNominalValue() ) };
        setSymbolPtr( x );
        return symbolTable.add_variable( getNameStdWString(), *x );
    }
    return false;
}


bool InputParameter::removeSymbol( const QString &name ) {
    // Remove the symbol from the table and also the double it points to
    return symbolTable.remove_variable( name.toStdWString(), true );
}


bool InputParameter::symbolExists( const QString &name ) {
    return symbolTable.symbol_exists( name.toStdWString() );
}


bool InputParameter::validSymbol( const QString &name ) {
    return symbolTable.valid_symbol( name.toStdWString() );
}
