// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "third_party/alglib/specialfunctions.h"
#include "mathconstants.h"
#include "outputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <third_party/Eigen/Dense>
#include <QRegularExpression>
#include <QUuid>
#include <QtAssert>
#include <QtMinMax>
#include <cmath>
#include <deque>


ModelControl<OutputParameter *> OutputParameter::mOutputModel = {};


OutputParameter::OutputParameter( QObject *parent )
    :   Parameter { parent },
        mFormula {},
        mConfidence { 0.95 },
        mError {},
        mComponents {},
        mMonteCarlo { MonteCarlo( this ) },
        mMixedCopulaSampler { MixedCopulaSampler( this ) }
{
    setNominalValue( MathConstants::nan );
    createConnections();
}


OutputParameter::OutputParameter( const OutputParameter &op )
    :   Parameter { op },
        mFormula { op.getFormula() },
        mConfidence { op.getConfidence() },
        mError { op.getError() },
        mComponents { op.getComponents() },
        mMonteCarlo { op.getMonteCarlo() },
        mMixedCopulaSampler { op.getMixedCopulaSampler() }
{
    mMonteCarlo.setOutputParameter( this );
    mMixedCopulaSampler.setOutputParameter( this );
    createConnections();
}


OutputParameter & OutputParameter::operator= ( const OutputParameter &op ) {
    if ( this != &op ) {
        Parameter::operator=( op );
        setFormula( op.getFormula() );
        setConfidence( op.getConfidence() );
        setError( op.getError() );
        setComponents( op.getComponents() );
        setMonteCarlo( op.getMonteCarlo() );
        setMixedCopulaSampler( op.getMixedCopulaSampler() );
        mMonteCarlo.setOutputParameter( this );
        mMixedCopulaSampler.setOutputParameter( this );
        createConnections();
    }

    return *this;
}


bool OutputParameter::operator== ( const OutputParameter &op ) const {
    // Two OutputParameters are considered equal when their numerical output
    // results are the same.
    // So their name and unit can be different.
    return ( mFormula == op.getFormula() && mConfidence == op.getConfidence() );
}


bool OutputParameter::operator!= ( const OutputParameter &op ) const {
    return !( *this == op );
}


DataType OutputParameter::OutputParameter::dataType() const {
    return DataType::OutputParameter;
}


MixedCopulaSampler OutputParameter::getMixedCopulaSampler() const {
    return mMixedCopulaSampler;
}


MonteCarlo OutputParameter::getMonteCarlo() const {
    return mMonteCarlo;
}


OutputParameter * OutputParameter::appendToModel() {
    // Append this OutputParameter to the model if the name is valid
    return insertIntoModel( mOutputModel.rowCount() );
}


QJsonObject OutputParameter::toJson() const {
    QJsonObject json {};
    json[ sIdString ] = getId().toString();
    json[ sNameString ] = getName();
    json[ sUnitString ] = getUnit();
    json[ sFormulaString ] = getFormula();
    json[ sConfidenceString] = getConfidence();
    json[ sMonteCarloString ] = mMonteCarlo.toJson();
    return json;
}


QList<UncertaintyComponent> OutputParameter::getComponents() const {
    return mComponents;
}


QList<double> OutputParameter::getHistogramValues() const {
    return mMonteCarlo.getHistogramValues();
}


QString OutputParameter::componentsToCSVString() const {
    // Add title and header labels
    QString componentsString {
        sUncertaintyComponentsHeaderString + getName() + ":"
    };
    QString result {};
    result += StringUtils::addQuotes( componentsString ) + StringUtils::endl;
    result += UncertaintyComponent::headerLabels.join(
                  StringUtils::csvSeparator
              ) +
              StringUtils::endl;

    // Add a data row for each component
    qsizetype rows { mComponents.size() };
    for ( int row { 0 }; row < rows; ++row ) {
        UncertaintyComponent *component { getComponent( row ) };
        if ( component ) {
            int columns {
                static_cast<int>( UncertaintyComponent::headerLabels.size() )
            };

            // The first columns contain component data
            for ( int column { 0 }; column < columns - 3; ++column ) {
                QVariant var { component->get( column, true ) };
                result += var.toString() + StringUtils::csvSeparator;
            }

            // The last 3 columns with contribution percentages depend on the
            // combined standard uncertainty of the OutputParameter
            for ( int column { columns - 3 }; column < columns; ++column ) {
                result += getContributionAsString( row, column );
                if ( column < columns ) {
                    result += StringUtils::csvSeparator;
                }
            }
            result += StringUtils::endl;
        }
    }

    return result;
}


QString OutputParameter::getComponentContributionAsString( int row ) const {
    double contri { getComponentContribution( row ) };
    return StringUtils::contributionToPercentageString( contri );
}


QString OutputParameter::getContributionAsString( int row, int column ) const {
    double contri { getContribution( row, column )};
    return StringUtils::contributionToPercentageString( contri );
}


QString OutputParameter::getCorrelationContributionAsString( int row ) const {
    double contri { getCorrelationContribution( row ) };
    return StringUtils::contributionToPercentageString( contri );
}


QString OutputParameter::getTotalContributionAsString( int row ) const {
    double contri { getTotalContribution( row ) };
    return StringUtils::contributionToPercentageString( contri );;
}


QString OutputParameter::histogramToCSVString() const {
    return mMonteCarlo.histogramToCSVString();
}


QString OutputParameter::resultsToCSVString() const {
    QStringList results { getResults( true ) };
    return results.join( StringUtils::csvSeparator );
}


QString OutputParameter::toCSVString() const {
    QStringList results {};
    for ( int column { 0 }; column < columnCount(); ++column ) {
        results.append( get( column, true ).toString() );
    }
    return results.join( StringUtils::csvSeparator );
}


QStringList OutputParameter::getMonteCarloResults( bool csvMode ) const {
    QStringList results {};
    results.fill( "", MonteCarlo::headerLabels.size() );
    if ( mValid ) {
        results[ 0 ] = getName( csvMode );
        results[ 1 ] = getUnit( csvMode );
        results[ 2 ] = mMonteCarlo.getStatus( csvMode );
        if ( !mLocked && mMonteCarlo.getValid() ) {
            results[ 3 ] = mMonteCarlo.getNumericalToleranceAsString();
            results[ 4 ] = mMonteCarlo.getMeanAsString( csvMode );
            results[ 5 ] = mMonteCarlo.getStdDeviationAsString( csvMode );
            results[ 6 ] = mMonteCarlo.getExpandedUncertaintyAsString( csvMode);
            results[ 7 ] = getConfidenceAsString();
        }
    }
    return results;
}


QStringList OutputParameter::getResults( bool csvMode ) const {
    QStringList results {};
    results.fill( "", resultLabels.size() );
    if ( mValid ) {
        results[ 0 ] = getName( csvMode );
        results[ 1 ] = getUnit( csvMode );
        results[ 2 ] = getNominalValueAsString( csvMode );
        results[ 3 ] = getCombinedStdUncertaintyAsString( csvMode );
        results[ 4 ] = getEffectiveDOFAsString();
        results[ 5 ] = getCoverageFactorAsString( csvMode );
        results[ 6 ] = getExpandedUncertaintyAsString( csvMode );
        results[ 7 ] = getConfidenceAsString();
    }
    return results;
}


QVariant OutputParameter::get( int column, bool csvMode ) const {
    switch( column ) {
        case 0:
            return QVariant( getName( csvMode ) );
        case 1:
            return QVariant( getFormula( csvMode ) );
        case 2:
            return QVariant( getUnit( csvMode ) );
        case 3:
            return QVariant( getConfidenceAsString() );
        case 4:
            return QVariant( getError( csvMode ) );
        case 5:
            return QVariant( getValid() );
        default:
            return QVariant();
    }
}


QVariant OutputParameter::getResult( int column, bool csvMode ) const {
    switch( column ) {
        case 0:
            return getName( csvMode );
        case 1:
            return getUnit( csvMode );
        case 2:
            return getNominalValueAsString( csvMode );
        case 3:
            return getCombinedStdUncertaintyAsString( csvMode );
        case 4:
            return getEffectiveDOFAsString();
        case 5:
            return getCoverageFactorAsString( csvMode );
        case 6:
            return getExpandedUncertaintyAsString( csvMode );
        case 7:
            return getConfidenceAsString();
        default:
            return QVariant();
    }
}


QVariant OutputParameter::headerData( int column ) const {
    if ( column >= 0 && column < headerLabels.size() ) {
        return headerLabels[ column ];
    }
    return QVariant();
}


UncertaintyComponent * OutputParameter::getComponent( int row ) const {
    if ( row >= 0 && row < getNumberOfComponents() ) {
        return const_cast<UncertaintyComponent *> ( &mComponents[ row ] );
    }
    return nullptr;
}


bool OutputParameter::getMonteCarloValid() const {
    return mMonteCarlo.getValid();
}


bool OutputParameter::isInputParameterReferenced( const QUuid &id ) const {
    if ( !id.isNull() ) {
        for ( const UncertaintyComponent &component : mComponents ) {
            if ( component.getInputParameterId() == id )  {
                return true;
            }
        }
    }
    return false;
}


double OutputParameter::getCombinedStdUncertainty() const {
    double combinedUncertainty { 0. };
    for ( const UncertaintyComponent &component : mComponents ) {
        // Constants (i.e. distribution is none) don't contribute to the
        // combined uncertainty
        if ( component.getDistribution() != Distribution::Type::none ) {
            combinedUncertainty += std::pow( component.getComponentValue(), 2 );
            combinedUncertainty += component.getCorrelationValue();
        }
    }
    return std::sqrt( combinedUncertainty );
}


double OutputParameter::getComponentContribution( int row ) const {
    // The coefficient of contribution needs to be calculated based
    // on the combined uncertainty of all components. This term gives the
    // relative contribution due to the variance only.
    UncertaintyComponent *component { getComponent( row ) };
    if ( component ) {
        return (
            std::pow( component->getComponentValue(), 2 )
            /
            std::pow( getCombinedStdUncertainty(), 2 )
        );
    }
    return 0.;
}


double OutputParameter::getContribution( int row, int column ) const {
    // Return the contributions for the last 3 columns of the uncertainty
    // components table
    if ( column == UncertaintyComponent::headerLabels.size() - 3 ) {
        return getComponentContribution( row );
    }
    else if ( column == UncertaintyComponent::headerLabels.size() - 2 ) {
        return getCorrelationContribution( row );
    }
    else if ( column == UncertaintyComponent::headerLabels.size() - 1 ) {
        return getTotalContribution( row );
    }
    return 0.;
}


double OutputParameter::getCorrelationContribution( int row ) const {
    // The coefficient of contribution needs to be calculated based
    // on the combined uncertainty of all components. This term gives the
    // contribution due to the correlations of this component and can be
    // negative.
    UncertaintyComponent *component { getComponent( row ) };
    if ( component ) {
        return (
            component->getCorrelationValue() /
            std::pow( getCombinedStdUncertainty(), 2 )
        );
    }
    return 0.;
}


double OutputParameter::getCoverageFactor() const {
    // Use the inverse cumulative distribution of student's t to determine the
    // coverage factor based on the effective DOF and the level of confidence
    // [-]. Note that invstudenttdistribution gives the leftsided inverse of the
    // student't t-distribution, hence p = 0.5 * ( 1. - confidence ).
    int DOF { getEffectiveDOF() };
    double confidence { getConfidence() };
    if ( DOF > 0 && confidence >= 0. && confidence < 1. ) {
        return std::abs(
            alglib::invstudenttdistribution( DOF, 0.5 * ( 1. - confidence ) )
        );
    }
    return 0.;
}


double OutputParameter::getExpandedUncertainty() const {
    return getCoverageFactor() * getCombinedStdUncertainty();
}


double OutputParameter::getHistogramXMax() const {
    return mMonteCarlo.getHistogramXMax();
}


double OutputParameter::getHistogramXMin() const {
    return mMonteCarlo.getHistogramXMin();
}


double OutputParameter::getHistogramYMax() const {
    return mMonteCarlo.getHistogramYMax();
}


double OutputParameter::getMonteCarloConvergenceFactor() const {
    return mMonteCarlo.getConvergenceFactor();
}


double OutputParameter::getTotalContribution( int row ) const {
    return getComponentContribution( row ) + getCorrelationContribution( row );
}


int OutputParameter::columnCount() const {
    return headerLabels.size();
}


int OutputParameter::getEffectiveDOF() const {
    // Calculate the effective degrees of freedom using the 'Welch-Satterthwaite
    // Relation for Correlated Errors' (H. Castrup)
    double numerator { std::pow( getCombinedStdUncertainty(), 4 ) };
    double denominator { 0. };

    // Sum the denominator terms
    for ( const UncertaintyComponent &component : mComponents ) {
        denominator += component.getWelchSatterthwaiteTerms();
    }

    // Calculate the effective DOF
    if ( denominator > 0. ) {
        double effectiveDOF { std::floor( numerator / denominator ) };
        return static_cast<int> ( effectiveDOF );
    }

    // Infinite DOF
    return MathConstants::maxInt;
}


int OutputParameter::getHistogramHigherIndex() const {
    return mMonteCarlo.getHistogramHigherIndex();
}


int OutputParameter::getHistogramLowerIndex() const {
    return mMonteCarlo.getHistogramLowerIndex();
}


int OutputParameter::getNumberOfComponents() const {
    return mComponents.size();
}


std::wstring OutputParameter::getFormulaStdWString() const {
    return mFormula.toStdWString();
}


void OutputParameter::addComponent( const UncertaintyComponent &component ) {
    mComponents.append( component );
    mMixedCopulaSampler.addVariable( component.getInvCDF() );
}


void OutputParameter::clearComponents() {
    mComponents.clear();
    mMixedCopulaSampler.clear();
}


void OutputParameter::resetResults( bool resetMonteCarlo ) {
    setNominalValue( MathConstants::nan );
    setError();
    clearComponents();
    if ( resetMonteCarlo ) {
        mMonteCarlo.resetResults();
    }
}


void OutputParameter::resetSymbolValues() {
    for ( UncertaintyComponent &component : mComponents ) {
        component.resetSymbolValue();
    }
}


void OutputParameter::set( int column, const QVariant &value ) {
    switch ( column ) {
        case 0:
            setName( value.toString() );
            break;
        case 1:
            setFormula( value.toString() );
            break;
        case 2:
            setUnit( value.toString() );
            break;
        case 3:
            setConfidence( value.toDouble() );
            break;
    }
}


void OutputParameter::setComponents(
    const QList<UncertaintyComponent> &components
) {
    mComponents = components;
}


void OutputParameter::setError( const QString &error ) {
    mError = error;
}


void OutputParameter::setLocked( bool locked ) {
    mLocked = locked;
    emit lockedChanged();
}


void OutputParameter::setMixedCopulaSampler(
    const MixedCopulaSampler &mixedCopulaSampler
) {
    mMixedCopulaSampler = mixedCopulaSampler;
}


void OutputParameter::setMonteCarlo( const MonteCarlo &monteCarlo ) {
    mMonteCarlo = monteCarlo;
}


void OutputParameter::setRandomSymbolValues() {
    mMixedCopulaSampler.setRandomSymbolValues();
}


void OutputParameter::startMonteCarlo() {
    mMonteCarlo.start();
}


void OutputParameter::stopMonteCarlo() {
    mMonteCarlo.stop();
}


void OutputParameter::updateFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ sIdString ]; v.isString() ) {
        setId( QUuid::fromString( v.toString() ) );
    }
    if ( const QJsonValue v = json[ sNameString ]; v.isString() ) {
        setName( v.toString() );
    }
    if ( const QJsonValue v = json[ sUnitString ]; v.isString() ) {
        setUnit( v.toString() );
    }
    if ( const QJsonValue v = json[ sFormulaString ]; v.isString() ) {
        setFormula( v.toString() );
    }
    if ( const QJsonValue v = json[ sConfidenceString ]; v.isDouble() ) {
        setConfidence( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sMonteCarloString ]; v.isObject() ) {
        MonteCarlo monteCarlo { MonteCarlo::fromJson( v.toObject() ) };
        setMonteCarlo( monteCarlo );
    }
}

QString OutputParameter::getConfidenceAsString() const {
    return QString::number( mConfidence * 100., 'f', 1 ) + "%";
}


QString OutputParameter::getError( bool csvMode ) const {
    return csvMode ? StringUtils::addQuotes( mError ) : mError;
}


QString OutputParameter::getFormula( bool csvMode ) const {
    return csvMode ? StringUtils::addQuotes( mFormula ) : mFormula;
}


QString OutputParameter::getNominalValueAsString( bool csvMode ) const {
    // Convert possible inf and nan using conversion to std::string
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    std::string stdStringVersion { std::to_string( mNominalValue ) };
    QString ret { QString::fromStdString( stdStringVersion ) };
    if ( std::isfinite( getNominalValue() ) ) {
        ret = StringUtils::doubleToString( getNominalValue(), precision );
    }
    return ret;
}


double OutputParameter::getConfidence() const {
    return mConfidence;
}


void OutputParameter::compile( bool resetMonteCarlo ) {
    resetResults( resetMonteCarlo );
    bool valid { true };
    expression_t expression {};

    // Make sure that the expression (i.e. measurement formula) is not empty.
    if ( mFormula.size() == 0 ) {
        setError( sEmptyExpressionString );
        valid = false;
    }

    if ( valid ) {
        // Try to compile the expression.
        static QRegularExpression re = QRegularExpression( sRegExErrorReplace );
        expression.register_symbol_table( InputParameter::symbolTable );
        if ( !parser.compile(mFormula.toStdWString(), expression ) ) {
            // Compilation error
            QString errorString {};
            errorString = QString::fromStdWString( parser.error() );
            errorString = errorString.mid( sErrorCodeLength );
            errorString.replace( re, "" );
            setError( errorString );
            valid = false;
        }
    }

    if ( valid ) {
        // The expression successfully compiled.
        setNominalValue( expression.value() );
        if ( !std::isfinite( getNominalValue() ) ) {
            // The nominal value is invalid.
            setError( sInvalidNominalString + getNominalValueAsString() );
            valid = false;
        }

        // Store the components.
        std::deque<symbol_t> variables {};
        parser.dec().symbols( variables );
        for ( auto &variable: variables ) {
            QString inputName { QString::fromStdWString( variable.first ) };
            InputParameter *inputParameter {
                InputParameter::getByName( inputName )
            };
            if ( inputParameter ) {
                UncertaintyComponent component {
                    UncertaintyComponent( inputParameter )
                };
                if ( valid ) {
                    // Calculate the sensitivity coefficient (i.e. the partial
                    // derivative)
                    bool validSensitivity { true };

                    // Obtain the component values to evaluate and the stepsize
                    QList<double> evaluationValues {
                        component.getEvaluationValues()
                    };
                    double stepSize {
                        ( evaluationValues[1] - evaluationValues[2] ) / 2.
                    };

                    // Calculate the output parameter values by setting the
                    // symbol value to the evaluation values
                    QList<double> outputValues {};
                    for ( double &value : evaluationValues ) {
                        component.setSymbolValue( value );
                        double outputValue { expression.value() };
                        if ( std::isfinite( outputValue ) ) {
                            // The output value is finite, so valid
                            outputValues.append( outputValue );
                        }
                        else {
                            // The output value is not valid, so stop
                            validSensitivity = false;
                            break;
                        }
                    }
                    if ( validSensitivity ) {
                        component.calculateSensitivity(
                            stepSize,
                            outputValues
                        );
                    }
                    // Set the symbol value back to its nominal value
                    component.resetSymbolValue();
                }
                addComponent( component );
            }
        }

        qsizetype n { mComponents.size() };
        if ( valid && n > 0 ) {
            // Valid and at least one input parameter detected
            setError( sValidExpressionString + getVariables() );

            bool allNormal { allComponentsNormal() };

            // Set latent correlation matrix
            Eigen::MatrixXd rho = Eigen::MatrixXd::Identity( n, n );

            // Determine the correlations between the components
            for ( int i { 0 }; i < n; ++i ) {
                InputParameter *paramI { mComponents[ i ].getInputParameter() };
                for ( int j { 0 }; j < n; ++j ) {
                    if ( i != j ) {
                        InputParameter *paramJ {
                            mComponents[ j ].getInputParameter()
                        };
                        Correlation *correlation {
                            Correlation::getCorrelation(
                                paramI->getId(),
                                paramJ->getId()
                            )
                        };
                        if ( correlation ) {
                            // Correlation is defined, store pointer to the
                            // component
                            mComponents[ i ].addCorrelatedComponent(
                                &mComponents[ j ]
                            );

                            double c { correlation->getCorrelation() };
                            // Pearson correlation = latent correlation.
                            // When not all components are normally distributed,
                            // the entered correlation is interpreted as
                            // Spearman rank correlation and converted to
                            // Pearson correlation using
                            // Pearson = 2 * sin( pi * Spearman / 6 )

                            if ( !allNormal ) {
                                c = 2. * std::sin( MathConstants::pi * c / 6. );
                            }

                            // Set latent correlation matrix
                            rho( i , j ) = c;
                        }
                    }
                }
            }

            mMixedCopulaSampler.setLatentCorrelation( rho );
        }
        else if ( valid ) {
            // Valid but no input parameters detected.
            setError( sNoInputParametersString );
        }
    }

    setValid( valid );
}


void OutputParameter::setConfidence( double confidence ) {
    mConfidence = confidence;
}


void OutputParameter::setFormula( const QString &formula ) {
    mFormula = formula.trimmed();
}


void OutputParameter::setToSelected() {
    const OutputParameter *parameter { mOutputModel.getSelected() };
    if ( parameter ) {
        *this = *parameter;
    }
}


ModelControl<OutputParameter *> * OutputParameter::getOutputModel() {
    return &mOutputModel;
}


OutputParameter OutputParameter::fromJson(
    const QJsonObject &json,
    bool appendToModel,
    QObject *parent
) {
    OutputParameter parameter { OutputParameter( parent ) };
    parameter.updateFromJson( json );

    if ( appendToModel ) {
        parameter.compile( false );
        parameter.appendToModel();
    }

    return parameter;
}


OutputParameter * OutputParameter::getByName( const QString &name ) {
    return mOutputModel.getByName( name );
}


OutputParameter * OutputParameter::getById( const QUuid &id ) {
    return mOutputModel.getById( id );
}


OutputParameter * OutputParameter::getSelected() {
    return mOutputModel.getSelected();
}

QJsonArray OutputParameter::parametersToJson() {
    QJsonArray paramArray {};
    for ( const OutputParameter *parameter : getAll() ) {
        paramArray.append( parameter->toJson() );
    }
    return paramArray;
}


QJsonObject OutputParameter::currentJson( const QUuid &id ) {
    OutputParameter *parameter { getById( id ) };
    return parameter ? parameter->toJson() : QJsonObject {};
}


const QList<OutputParameter *> & OutputParameter::getAll() {
    return mOutputModel.getAllRows();
}


QString OutputParameter::parametersToCSVString() {
    QString result {};
    // Add title, header labels and OutputParameter data
    result += sOutputParametersHeaderString + StringUtils::endl;
    result += headerLabels.join( StringUtils::csvSeparator ) +
              StringUtils::endl;
    for ( OutputParameter *parameter : getAll() ) {
        if ( parameter) {
            result += parameter->toCSVString() + StringUtils::endl;
        }
    }
    result += StringUtils::endl;

    // Add title, header labels and combined uncertainty for each valid
    // OutputParameter
    result += sCombinedUncertaintyHeaderString + StringUtils::endl;
    result += resultLabels.join( StringUtils::csvSeparator ) +
              StringUtils::endl;
    for ( const OutputParameter *parameter : getAll() ) {
        if ( parameter && parameter->getValid() ) {
            result += parameter->resultsToCSVString() + StringUtils::endl;
        }
    }
    result += StringUtils::endl;

    // Add the components for each valid OutputParameter. This already includes
    // a title and header labels
    for ( const OutputParameter *parameter : getAll() ) {
        if ( parameter && parameter->getValid() ) {
            result += parameter->componentsToCSVString() + StringUtils::endl;
        }
    }

    // Add title, header labels and Monte Carlo simulation results for each
    // valid OutputParameter
    result += sMonteCarloHeaderString + StringUtils::endl;
    result += MonteCarlo::headerLabels.join( StringUtils::csvSeparator ) +
              StringUtils::endl;
    for ( const OutputParameter *parameter : getAll() ) {
        if ( parameter && parameter->getValid() ) {
            QStringList mcResults { parameter->getMonteCarloResults( true ) };
            result += mcResults.join( StringUtils::csvSeparator ) +
                      StringUtils::endl;
        }
    }
    result += StringUtils::endl;

    // Add title, header labels and histogram data for each valid
    // OutputParameter
    for ( const OutputParameter *parameter : getAll() ) {
        if ( parameter && parameter->getValid() ) {
            QString histogramString { parameter->histogramToCSVString() };
            if ( histogramString.size() > 0 ) {
                result += histogramString + StringUtils::endl;
            }
        }
    }

    return result;
}


QUuid OutputParameter::getSelectedId() {
    return mOutputModel.getSelectedId();
}


bool OutputParameter::remove( const QUuid &id ) {
    bool parameterRemoved { mOutputModel.removeById( id ) };
    Q_ASSERT_X(
        parameterRemoved,
        "OutputParameter::remove",
        "Parameter could not be removed"
    );
    return parameterRemoved;
}


bool OutputParameter::update( const QUuid &id, OutputParameter *parameter ) {
    // The new name has to be valid (unique) or equal to the original parameter
    OutputParameter *originalParameter { getById( id )};
    Q_ASSERT_X(
        originalParameter,
        "OutputParameter::update",
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
        bool parameterUpdated = mOutputModel.updateById( id, *parameter );
        Q_ASSERT_X(
            parameterUpdated,
            "OutputParameter::update",
            "Parameter could not be updated"
        );
        return parameterUpdated;
    }
    return false;
}


bool OutputParameter::validName(
    const QString &name,
    bool checkCurrentSelection
) {
    // The name has to be unique or equal to the current selected parameter when
    // checkCurrentSelection = true
    if ( checkCurrentSelection && mOutputModel.nameIsSelected( name ) ) {
        return true;
    }
    return !mOutputModel.nameIsPresent( name ) && name.size() > 0;
}


int OutputParameter::getRowIndex( const QUuid &id ) {
    return mOutputModel.getRowIndex( id );
}


int OutputParameter::getSelectedRow() {
    return mOutputModel.selectedRow();
}


void OutputParameter::applyDiff( const JsonDiff &diff ) {
    if ( diff.after.isEmpty() ) {
        // Target state is empty — remove from model
        bool parameterRemoved { remove( diff.objectId ) };
        Q_ASSERT_X(
            parameterRemoved,
            "OutputParameter::applyDiff",
            "After empty - Parameter could not be removed"
        );
    } else if ( diff.before.isEmpty() ) {
        // Before state is empty — insert in the previous position
        OutputParameter parameter { fromJson( diff.after, false ) };
        bool parameterInserted = parameter.insertIntoModel( diff.row );
        Q_ASSERT_X(
            parameterInserted,
            "OutputParameter::applyDiff",
            "Before empty - Parameter could not be inserted"
        );
    } else {
        // Update the existing parameter in the model
        OutputParameter afterParam { *( getById( diff.objectId ) ) };
        afterParam.updateFromJson( diff.after );
        bool parameterUpdated { update( diff.objectId, &afterParam ) };
        Q_ASSERT_X(
            parameterUpdated,
            "OutputParameter::applyDiff",
            "Update - Parameter could not be updated"
        );
    }
}


void OutputParameter::clearModel() {
    mOutputModel.clear();
}


void OutputParameter::parametersFromJson(
    const QJsonArray &jsonArray,
    QObject *parent
) {
    for ( const QJsonValue &parameter : jsonArray ) {
        fromJson( parameter.toObject(), true, parent );
    }
}


void OutputParameter::onDisplayPrecisionChanged() {
    mOutputModel.emitAllDataChanged();
}


void OutputParameter::recompileAllExpressions( bool resetMonteCarlo ) {
    const QList<OutputParameter *> &params { mOutputModel.getAllRows() };
    for ( OutputParameter *parameter : params ) {
        parameter->compile( resetMonteCarlo );
        mOutputModel.emitIdChanged( parameter->getId() );
    }
}


void OutputParameter::setCollectVariables( bool collect ) {
    parser.dec().collect_variables() = collect;
}


void OutputParameter::setSelectionLocked( bool locked ) {
    mOutputModel.setSelectionLocked( locked );
}


OutputParameter * OutputParameter::insertIntoModel( int row ) {
    // Insert this OutputParameter into the model at row if the name is valid
    if ( validName( getName() ) ) {
        const int boundedRow { qBound( 0, row, mOutputModel.rowCount() ) };
        return mOutputModel.insertRow( boundedRow, *this );
    }
    return nullptr;
}


QString OutputParameter::getCombinedStdUncertaintyAsString(
    bool csvMode
) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString(
        getCombinedStdUncertainty(),
        precision
    );
}


QString OutputParameter::getCoverageFactorAsString( bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString( getCoverageFactor(), precision );
}


QString OutputParameter::getEffectiveDOFAsString() const {
    int DOF { getEffectiveDOF() };
    if ( DOF < MathConstants::maxInt ) {
        return QString::number( DOF, 'f', 0 );
    }
    return StringUtils::infinite;
}


QString OutputParameter::getExpandedUncertaintyAsString( bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return QString( "±" ) +
           StringUtils::doubleToString( getExpandedUncertainty(), precision );
}


QString OutputParameter::getVariables() const {
    QStringList variableList {};
    for ( const UncertaintyComponent &component : mComponents ) {
        variableList.append( component.getName() );
    }
    return variableList.join( ", " );
}


bool OutputParameter::allComponentsNormal() const {
    // Are all components referencing InputParameters with a Normal
    // distribution?
    for ( const UncertaintyComponent &component : mComponents ) {
        InputParameter *inputParameter { component.getInputParameter() };
        if (
            !inputParameter ||
            inputParameter->getDistribution() != Distribution::Type::normal
        ) {
            return false;
        }
    }
    return true;
}


void OutputParameter::createConnections() {
    // Connect to the Monte Carlo object. The UniqueConnection flag is used in
    // case the assignment operator is used and the connections already exist.
    connect(
        &mMonteCarlo,
        &MonteCarlo::started,
        this,
        &OutputParameter::monteCarloStarted,
        Qt::UniqueConnection
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::finished,
        this,
        &OutputParameter::monteCarloFinished,
        Qt::UniqueConnection
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::monteCarloStatusChanged,
        this,
        &OutputParameter::monteCarloStatusChanged,
        Qt::UniqueConnection
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::convergenceFactorChanged,
        this,
        &OutputParameter::monteCarloConvergenceFactorChanged,
        Qt::UniqueConnection
    );
}
