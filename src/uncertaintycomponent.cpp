// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "settings.h"
#include "stringutils.h"
#include "uncertaintycomponent.h"
#include <cmath>


QStringList UncertaintyComponent::headerLabels = {
    "Input name",
    "Unit",
    "Input estimate",
    "Standard uncertainty",
    "Distribution",
    "Degrees of freedom",
    "Sensitivity",
    "Component",
    "Variance %",
    "Correlation %",
    "Total %"
};

QList<int> UncertaintyComponent::columnWidths = {
    100,
    55,
    105,
    147,
    92,
    142,
    92,
    90,
    91,
    102,
    75
};


UncertaintyComponent::UncertaintyComponent()
    :   mInputParameter {},
        mSensitivity { 0. },
        mCorrelatedComponents { QList<UncertaintyComponent *>() }
{}


UncertaintyComponent::UncertaintyComponent( InputParameter *parameter )
    :   mInputParameter { parameter },
        mSensitivity { 0. },
        mCorrelatedComponents { QList<UncertaintyComponent *>() }
{}


UncertaintyComponent::UncertaintyComponent( const UncertaintyComponent &cor )
    :   mInputParameter { cor.getInputParameter() },
        mSensitivity { cor.getSensitivity() },
        mCorrelatedComponents { cor.getCorrelatedComponents() }
{}


UncertaintyComponent::~UncertaintyComponent(){}


UncertaintyComponent& UncertaintyComponent::operator= (
    const UncertaintyComponent &cor
) {
    if ( this == &cor ) {
        return *this;
    }

    setInputParameter( cor.getInputParameter() );
    setSentitivity( cor.getSensitivity() );
    setCorrelatedComponents( cor.getCorrelatedComponents() );

    return *this;
}


Distribution::Type UncertaintyComponent::getDistribution() const {
    if ( mInputParameter ) {
        return mInputParameter->getDistribution();
    }
    return Distribution::Type::none;
}


InputParameter * UncertaintyComponent::getInputParameter() const {
    return mInputParameter;
}


QList<UncertaintyComponent *> UncertaintyComponent::getCorrelatedComponents(
) const {
    return mCorrelatedComponents;
}


QList<double> UncertaintyComponent::getEvaluationValues() const {
    QList<double> values {};
    InputParameter *parameter { getInputParameter() };
    if ( parameter ) {
        const double nominalValue { parameter->getNominalValue() };
        // stepSize is equal to:
        // abs(nominalValue * mStepSize) when nominalValue is not 0, otherwise
        // mStepSize
        double stepSize { std::abs( nominalValue * mStepSize ) };
        stepSize = stepSize == 0 ? mStepSize : stepSize;
        values.append( nominalValue + 2. * stepSize );
        values.append( nominalValue + 1. * stepSize );
        values.append( nominalValue - 1. * stepSize );
        values.append( nominalValue - 2. * stepSize );
    }
    return values;
}


QString UncertaintyComponent::getDOFAsString() const {
    if ( mInputParameter ) {
        return mInputParameter->getDOFAsString();
    }
    return "";
}


QString UncertaintyComponent::getDistributionAsString() const {
    if ( mInputParameter ) {
        return mInputParameter->getDistributionAsString();
    }
    return "";
}


QString UncertaintyComponent::getName( const bool &csvMode ) const {
    if ( mInputParameter ) {
        return mInputParameter->getName( csvMode );
    }
    return "";
}


QString UncertaintyComponent::getUnit( const bool &csvMode ) const {
    if ( mInputParameter ) {
        return mInputParameter->getUnit( csvMode );
    }
    return "";
}


QVariant UncertaintyComponent::get(
    const int &column,
    const bool &csvMode
) const {
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
        case 6:
            return QVariant(
                StringUtils::doubleToString( getSensitivity(), precision )
            );
        case 7:
            return QVariant(
                StringUtils::doubleToString( getComponentValue(), precision )
            );
        default:
            return QVariant();
    }
}


Distribution::InvCDF UncertaintyComponent::getInvCDF() const {
    if ( mInputParameter ) {
        return mInputParameter->getInvCDF();
    }
    return Distribution::invCDFConstant( 0. );
}


bool UncertaintyComponent::getDOFInfinite() const {
    if ( mInputParameter ) {
        return mInputParameter->getDOFInfinite();
    }
    return true;
}


double UncertaintyComponent::getComponentValue() const {
    // When the distribution is none, the component value is zero because it
    // does not contribute to the combined uncertainty
    if ( getDistribution() != Distribution::Type::none ) {
        return std::abs( mSensitivity * getStdUncertainty() );
    }
    return 0.;
}


double UncertaintyComponent::getCorrelationValue() const {
    if (
        mInputParameter &&
        mCorrelatedComponents.size() > 0 &&
        // Correlation to a constant has no meaning:
        getDistribution() != Distribution::Type::none
    ) {
        double value {};
        for ( UncertaintyComponent *component : mCorrelatedComponents ) {
            double stdUncertaintyCor { component->getStdUncertainty() };
            double sensitivityCor { component->getSensitivity() };
            Correlation *correlation {
                Correlation::getCorrelation(
                    mInputParameter,
                    component->getInputParameter()
                )
            };
            if ( correlation ) {
                value += sensitivityCor *
                         getSensitivity() *
                         stdUncertaintyCor *
                         getStdUncertainty() *
                         correlation->getCorrelation();
            }
        }
        return value;
    }
    return 0.;
}


double UncertaintyComponent::getNominalValue() const {
    if ( mInputParameter ) {
        return mInputParameter->getNominalValue();
    }
    return 0.;
}


double UncertaintyComponent::getSensitivity() const {
    return mSensitivity;
}


double UncertaintyComponent::getStdUncertainty() const {
    if ( mInputParameter ) {
        return mInputParameter->getStdUncertainty();
    }
    return 0.;
}


double UncertaintyComponent::getSymbolValue() const {
    if ( mInputParameter ) {
        return *mInputParameter->getSymbolPtr();
    }
    return 0.;
}


double UncertaintyComponent::getWelchSatterthwaiteTerms() const {
    // Returns the denominator terms of the Welch-Satterthwaite formula for this
    // component
    double value {};
    if (
        mInputParameter &&
        !getDOFInfinite() &&
        getDistribution() != Distribution::Type::none
    ) {
        double u_i { getStdUncertainty() };
        double c_i { getSensitivity() };
        double v_i { static_cast<double> ( getDOF() ) };

        // Variance term
        value = std::pow( c_i * u_i, 4 ) / v_i;

        // Correlation terms
        for ( UncertaintyComponent *component : mCorrelatedComponents ) {
            // The terms for components with infinite DOF are zero
            if ( !component->getDOFInfinite() ) {
                double u_j { component->getStdUncertainty() };
                double c_j { component->getSensitivity() };
                int v_j { component->getDOF() };
                Correlation *correlation {
                    Correlation::getCorrelation(
                        mInputParameter,
                        component->getInputParameter()
                    )
                };
                if ( correlation ) {
                    double r_ij { correlation->getCorrelation() };
                    double term {};
                    // First term
                    term = std::pow( r_ij * c_i * c_j * u_i * u_j, 2 );
                    term *= ( ( v_i + v_j + 0.5) / ( v_i * v_j) ) / 2.;
                    value += term;

                    // Second term
                    term = r_ij * c_i * c_j * u_i * u_j;
                    term *= (
                        std::pow( c_i * u_i, 2 ) / v_i +
                        std::pow( c_j * u_j, 2 ) / v_j
                    );
                    value += term;
                }
            }
        }
    }
    return value;
}



int UncertaintyComponent::getDOF() const {
    if ( mInputParameter ) {
        return mInputParameter->getDOF();
    }
    return 1;
}


void UncertaintyComponent::addCorrelatedComponent(
    UncertaintyComponent *component
) {
    if ( component && !mCorrelatedComponents.contains( component ) ) {
        mCorrelatedComponents.append( component );
    }
}


void UncertaintyComponent::calculateSensitivity(
    const double &stepSize,
    const QList<double> &values
) {
    // Five point midpoint approximation formula for first derivative
    if ( values.size() == 4 ) {
        setSentitivity(
            ( -values[ 0 ] + 8 * values[ 1 ] - 8 * values[ 2 ] + values[ 3 ] ) /
            ( 12. * stepSize )
        );
    }
}


void UncertaintyComponent::resetSymbolValue() {
    if ( mInputParameter ) {
        mInputParameter->resetSymbolValue();
    }
}


void UncertaintyComponent::setCorrelatedComponents(
    const QList<UncertaintyComponent *> &components
    ) {
    mCorrelatedComponents = components;
}


void UncertaintyComponent::setInputParameter( InputParameter *parameter ) {
    mInputParameter = parameter;
}


void UncertaintyComponent::setSentitivity( const double &sensitivity ) {
    mSensitivity = sensitivity;
}


void UncertaintyComponent::setSymbolValue( const double &value ) {
    if ( mInputParameter ) {
        mInputParameter->setSymbolValue(value);
    }
}
