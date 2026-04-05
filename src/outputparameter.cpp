// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "third_party/alglib/specialfunctions.h"
#include "outputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <third_party/Eigen/Dense>
#include <QRegularExpression>
#include <cmath>
#include <deque>


QString OutputParameter::mCombinedUncertaintyHeaderString = \
    "Combined uncertainty:";
QString OutputParameter::mConfidenceString = "confidence";
QString OutputParameter::mErrorString = "error";
QString OutputParameter::mFormulaString = "formula";
QString OutputParameter::mMonteCarloHeaderString = "Monte Carlo simulation:";
QString OutputParameter::mMonteCarloString = "monteCarlo";
QString OutputParameter::mOutputParametersHeaderString = "Output parameters:";
QString OutputParameter::mUncertaintyComponentsHeaderString = \
    "Uncertainty components ";

ModelControl<OutputParameter *> OutputParameter::mOutputModel = \
    ModelControl<OutputParameter *>();

QString OutputParameter::defaultName = "Y";

QStringList OutputParameter::headerLabels {
    "Output name",
    "Measurement function",
    "Unit",
    "Level of confidence"
};

QList<int> OutputParameter::columnWidths = { 120, 180, 55, 140 };

QStringList OutputParameter::resultLabels {
    "Output name",
    "Unit",
    "Output estimate",
    "Combined standard uncertainty",
    "Effective degrees of freedom",
    "Coverage factor",
    "Expanded uncertainty",
    "Level of confidence"
};

QList<int> OutputParameter::resultColumnWidths = {
    100,
    55,
    120,
    212,
    198,
    120,
    155,
    140
};

parser_t OutputParameter::parser = parser_t();


OutputParameter::OutputParameter( QObject *parent )
    :   Parameter { parent },
        mFormula {},
        mConfidence { 0.95 },
        mError {},
        mComponents {},
        mMonteCarlo { MonteCarlo( this ) },
        mMixedCopulaSampler { MixedCopulaSampler( this ) }
{
    setNominalValue( mDoubleNaN );
}


OutputParameter::OutputParameter( const OutputParameter &op )
    :   Parameter {
            op.parent(),
            op.getName(),
            op.getUnit(),
            op.getNominalValue(),
            op.getLocked(),
            op.getValid()
        },
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


OutputParameter::~OutputParameter(){}


OutputParameter& OutputParameter::operator= ( const OutputParameter &op ) {
    if ( this == &op ) {
        return *this;
    }

    setName( op.getName() );
    setUnit( op.getUnit() );
    setFormula( op.getFormula() );
    setNominalValue( op.getNominalValue() );
    setConfidence( op.getConfidence() );
    setError( op.getError() );
    setValid( op.getValid() );
    setComponents( op.getComponents() );
    setMonteCarlo( op.getMonteCarlo() );
    setLocked( op.getLocked() );
    setParent( op.parent() );
    setMixedCopulaSampler( op.getMixedCopulaSampler() );
    mMonteCarlo.setOutputParameter( this );
    mMixedCopulaSampler.setOutputParameter( this );

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


MixedCopulaSampler OutputParameter::getMixedCopulaSampler() const {
    return mMixedCopulaSampler;
}


MonteCarlo OutputParameter::getMonteCarlo() const {
    return mMonteCarlo;
}


OutputParameter * OutputParameter::addToModel( const bool &resetMonteCarlo ) {
    if ( validName( getName() ) ) {
        compile( resetMonteCarlo );
        return mOutputModel.appendRow( *this );
    }
    return nullptr;
}


QJsonObject OutputParameter::toJson() const {
    QJsonObject json {};
    json[ mNameString ] = getName();
    json[ mUnitString ] = getUnit();
    json[ mFormulaString ] = getFormula();
    json[ mConfidenceString] = getConfidence();
    json[ mMonteCarloString ] = mMonteCarlo.toJson();
    return json;
}


QList<UncertaintyComponent> OutputParameter::getComponents() const {
    return mComponents;
}


QList<double> OutputParameter::getHistogramValues() {
    return mMonteCarlo.getHistogramValues();
}


QString OutputParameter::componentsToString() const {
    // Add title and header labels
    QString componentsString {
        mUncertaintyComponentsHeaderString + getName() + ":"
    };
    QString result {};
    result += addQuotes( componentsString ) + endl;
    result += UncertaintyComponent::headerLabels.join(
                  StringUtils::csvSeparator
              ) +
              endl;

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
            result += endl;
        }
    }

    return result;
}


QString OutputParameter::getComponentContributionAsString(
    const int &row
) const {
    double contri { getComponentContribution( row ) };
    return contributionToPercentageString( contri );
}


QString OutputParameter::getContributionAsString(
    const int &row,
    const int &column
) const {
    double contri { getContribution( row, column )};
    return contributionToPercentageString( contri );
}


QString OutputParameter::getCorrelationContributionAsString(
    const int &row
) const {
    double contri { getCorrelationContribution( row ) };
    return contributionToPercentageString( contri );
}


QString OutputParameter::getTotalContributionAsString( const int &row ) const {
    double contri { getTotalContribution( row ) };
    return contributionToPercentageString( contri );;
}


QString OutputParameter::histogramToString() const {
    return mMonteCarlo.histogramToString();
}


QString OutputParameter::resultsToString() const {
    QStringList results { getResults( true ) };
    return results.join( StringUtils::csvSeparator );
}


QString OutputParameter::toString() const {
    QStringList results {};
    for ( int column { 0 }; column < staticColumnCount(); ++column ) {
        results.append( get( column, true ).toString() );
    }
    return results.join( StringUtils::csvSeparator );
}


QStringList OutputParameter::getMonteCarloResults( const bool &csvMode ) const {
    QStringList results {};
    results.fill( "", MonteCarlo::headerLabels.size() );
    if ( getValid() ) {
        results[ 0 ] = getName( csvMode );
        results[ 1 ] = getUnit( csvMode );
        results[ 2 ] = mMonteCarlo.getStatus( csvMode );
        if ( !getLocked() && mMonteCarlo.getValid() ) {
            results[ 3 ] = mMonteCarlo.getNumericalToleranceAsString();
            results[ 4 ] = mMonteCarlo.getMeanAsString( csvMode );
            results[ 5 ] = mMonteCarlo.getStdDeviationAsString( csvMode );
            results[ 6 ] = mMonteCarlo.getExpandedUncertaintyAsString(
                               csvMode
                           );
            results[ 7 ] = getConfidenceAsString();
        }
    }
    return results;
}


QStringList OutputParameter::getResults(const bool &csvMode ) const {
    QStringList results {};
    results.fill( "", resultLabels.size() );
    if ( getValid() ) {
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


QVariant OutputParameter::get( const int &column, const bool &csvMode ) const {
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


QVariant OutputParameter::getResult(
    const int &column,
    const bool &csvMode
) const {
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


QVariant OutputParameter::headerData( const int &column ) const {
    return staticHeaderData( column );
}


UncertaintyComponent * OutputParameter::getComponent( const int &row ) const {
    if ( row >= 0 && row < getNumberOfComponents() ) {
        return const_cast<UncertaintyComponent *> ( &mComponents[ row ] );
    }
    return nullptr;
}


bool OutputParameter::getMonteCarloValid() const {
    return mMonteCarlo.getValid();
}


bool OutputParameter::isInputParameterReferenced(
    InputParameter * const &inputParameter
) const {
    if ( inputParameter ) {
        for ( const UncertaintyComponent &component : mComponents ) {
            if ( component.getInputParameter() == inputParameter )  {
                return true;
            }
        }
    }
    return false;
}


bool OutputParameter::updateSelectedModelRow() {
    OutputParameter *selectedParameter { mOutputModel.getSelectedRow() };
    if ( selectedParameter && validName( getName(), true ) ) {
        if ( *selectedParameter != *this ) {
            // New parameter results will be different, so recompile and update.
            compile();
            mOutputModel.updateSelectedRow( *this );
        }
        else {
            // Parameter results will stay the same so only update name and
            // unit.
            selectedParameter->setName( getName() );
            selectedParameter->setUnit( getUnit() );
            mOutputModel.emitRowChanged( mOutputModel.selectedRow() );
        }
        return true;
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


double OutputParameter::getComponentContribution( const int &row ) const {
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


double OutputParameter::getContribution(
    const int &row,
    const int &column
) const {
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


double OutputParameter::getCorrelationContribution( const int &row ) const {
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


double OutputParameter::getTotalContribution( const int &row ) const {
    return getComponentContribution( row ) + getCorrelationContribution( row );
}


int OutputParameter::columnCount() const {
    return staticColumnCount();
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
    return mMaxInt;
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


void OutputParameter::resetResults( const bool &resetMonteCarlo ) {
    setNominalValue( mDoubleNaN );
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


void OutputParameter::set( const int &column, const QVariant &value ) {
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


void OutputParameter::setLocked( const bool &locked ) {
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


QString OutputParameter::getConfidenceAsString() const {
    return QString::number( mConfidence * 100., 'f', 1 ) + "%";
}


QString OutputParameter::getError( const bool &csvMode ) const {
    return csvMode ? addQuotes( mError ) : mError;
}


QString OutputParameter::getFormula( const bool &csvMode ) const {
    return csvMode ? addQuotes( mFormula ) : mFormula;
}


QString OutputParameter::getNominalValueAsString( const bool &csvMode ) const {
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


void OutputParameter::compile( const bool &resetMonteCarlo ) {
    resetResults( resetMonteCarlo );
    bool valid { true };
    expression_t expression {};

    // Make sure that the expression (i.e. measurement formula) is not empty.
    if ( mFormula.size() == 0 ) {
        setError( mEmptyExpressionString );
        valid = false;
    }

    if ( valid ) {
        // Try to compile the expression.
        static QRegularExpression re = QRegularExpression( mRegExErrorReplace );
        expression.register_symbol_table( InputParameter::symbolTable );
        if ( !parser.compile(mFormula.toStdWString(), expression ) ) {
            // Compilation error
            QString errorString {};
            errorString = QString::fromStdWString( parser.error() );
            errorString = errorString.mid( mErrorCodeLength );
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
            setError( mInvalidNominalString + getNominalValueAsString() );
            valid = false;
        }

        // Store the components.
        std::deque<symbol_t> variables {};
        parser.dec().symbols( variables );
        for ( auto &variable: variables ) {
            QString inputName { QString::fromStdWString( variable.first ) };
            InputParameter *inputParameter {
                InputParameter::getInputParameter( inputName )
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
            setError( mValidExpressionString + getVariables() );

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
                            Correlation::getCorrelation( paramI, paramJ )
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
                                c = 2. * std::sin( pi * c / 6. );
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
            setError( mNoInputParametersString );
        }
    }

    setValid( valid );
}


void OutputParameter::setConfidence( const double &confidence ) {
    mConfidence = confidence;
}


void OutputParameter::setFormula( const QString &formula ) {
    mFormula = formula.trimmed();
}


void OutputParameter::setToSelected() {
    OutputParameter * const parameter { mOutputModel.getSelectedRow() };
    if ( parameter ) {
        *this = *parameter;
    }
}


ModelControl<OutputParameter *> * OutputParameter::getOutputModel() {
    return &mOutputModel;
}


OutputParameter OutputParameter::fromJson(
    const QJsonObject &json,
    const bool &addToModel,
    QObject *parent
) {
    OutputParameter parameter { OutputParameter( parent ) };
    if ( const QJsonValue v = json[ mNameString ]; v.isString() ) {
        parameter.setName( v.toString() );
    }
    if ( const QJsonValue v = json[ mUnitString ]; v.isString() ) {
        parameter.setUnit( v.toString() );
    }
    if ( const QJsonValue v = json[ mFormulaString ]; v.isString() ) {
        parameter.setFormula( v.toString() );
    }
    if ( const QJsonValue v = json[ mConfidenceString ]; v.isDouble() ) {
        parameter.setConfidence( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mMonteCarloString ]; v.isObject() ) {
        MonteCarlo monteCarlo { MonteCarlo::fromJson( v.toObject() ) };
        parameter.setMonteCarlo( monteCarlo );
    }

    if ( addToModel ) {
        parameter.addToModel( false );
    }

    return parameter;
}


QJsonArray OutputParameter::parametersToJson() {
    QJsonArray paramArray {};
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        paramArray.append( parameter->toJson() );
    }
    return paramArray;
}


QString OutputParameter::parametersToString() {
    QString result {};
    // Add title, header labels and output parameter data.
    result += mOutputParametersHeaderString + endl;
    result += headerLabels.join( StringUtils::csvSeparator ) + endl;
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        if ( parameter) {
            result += parameter->toString() + endl;
        }
    }
    result += endl;

    // Add title, header labels and combined uncertainty for each valid output
    // parameter.
    result += mCombinedUncertaintyHeaderString + endl;
    result += resultLabels.join( StringUtils::csvSeparator ) + endl;
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        if ( parameter && parameter->getValid() ) {
            result += parameter->resultsToString() + endl;
        }
    }
    result += endl;

    // Add the components for each valid output parameter. This already includes
    // a title and header labels.
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        if ( parameter && parameter->getValid() ) {
            result += parameter->componentsToString() + endl;
        }
    }

    // Add title, header labels and Monte Carlo simulation results for each
    // valid output parameter.
    result += mMonteCarloHeaderString + endl;
    result += MonteCarlo::headerLabels.join( StringUtils::csvSeparator ) + endl;
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        if ( parameter && parameter->getValid() ) {
            QStringList mcResults { parameter->getMonteCarloResults( true ) };
            result += mcResults.join( StringUtils::csvSeparator ) + endl;
        }
    }
    result += endl;

    // Add title, header labels and histogram data for each valid output
    // parameter.
    for ( OutputParameter * &parameter : mOutputModel.getAllRows() ) {
        if ( parameter && parameter->getValid() ) {
            QString histogramString { parameter->histogramToString() };
            if ( histogramString.size() > 0 ) {
                result += histogramString + endl;
            }
        }
    }

    return result;
}


bool OutputParameter::validName(
    const QString &name,
    const bool &checkCurrentSelection
) {
    if ( checkCurrentSelection && mOutputModel.nameIsCurrent( name ) ) {
        return true;
    }
    return !mOutputModel.nameIsPresent( name ) && name.size() > 0;
}


QVariant OutputParameter::staticHeaderData( const int &column ) {
    if ( column >= 0 && column < staticColumnCount() ) {
        return headerLabels[ column ];
    }
    return QVariant();
}


int OutputParameter::staticColumnCount() {
    return headerLabels.size();
}


void OutputParameter::parametersFromJson(
    const QJsonArray &jsonArray,
    QObject *parent
) {
    for ( const QJsonValue &parameter : jsonArray ) {
        fromJson( parameter.toObject(), true, parent );
    }
}


void OutputParameter::setCollectVariables( const bool &collect ) {
    parser.dec().collect_variables() = collect;
}


QString OutputParameter::getCombinedStdUncertaintyAsString(
    const bool &csvMode
) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString(
        getCombinedStdUncertainty(),
        precision
    );
}


QString OutputParameter::getCoverageFactorAsString(
    const bool &csvMode
) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString( getCoverageFactor(), precision );
}


QString OutputParameter::getEffectiveDOFAsString() const {
    int DOF { getEffectiveDOF() };
    if ( DOF < mMaxInt ) {
        return QString::number( DOF, 'f', 0 );
    }
    return InputParameter::infiniteString;
}


QString OutputParameter::getExpandedUncertaintyAsString(
    const bool &csvMode
) const {
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
    return variableList.join( variableSeparator );
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
    // Connect to Monte Carlo object
    connect(
        &mMonteCarlo,
        &MonteCarlo::started,
        this,
        &OutputParameter::monteCarloStarted
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::finished,
        this,
        &OutputParameter::monteCarloFinished
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::monteCarloStatusChanged,
        this,
        &OutputParameter::monteCarloStatusChanged
    );
    connect(
        &mMonteCarlo,
        &MonteCarlo::convergenceFactorChanged,
        this,
        &OutputParameter::monteCarloConvergenceFactorChanged
    );
}
