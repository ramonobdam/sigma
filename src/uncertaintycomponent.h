// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNCERTAINTYCOMPONENT_H
#define UNCERTAINTYCOMPONENT_H

#include "distribution.h"
#include "inputparameter.h"
#include <QList>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariant>

// UncertaintyComponent objects are used to store the pointers to the
// InputParameters detected in the measurement formula of an OutputParameter. It
// also provides methods used to calculate the sensitivity of the
// OutputParameter with respect to this component.
class UncertaintyComponent {

public:
    UncertaintyComponent();
    UncertaintyComponent( InputParameter *parameter );
    UncertaintyComponent( const UncertaintyComponent &cor );

    UncertaintyComponent & operator= ( const UncertaintyComponent &cor );

    Distribution::Type getDistribution() const;
    InputParameter *getInputParameter() const;
    QList<UncertaintyComponent *> getCorrelatedComponents() const;
    QList<double> getEvaluationValues() const;
    QString getDOFAsString() const;
    QString getDistributionAsString() const;
    QString getName( bool csvMode = false ) const;
    QString getUnit( bool csvMode = false ) const;
    QUuid getInputParameterId() const;
    QVariant get( int column, bool csvMode = false ) const;
    Distribution::InvCDF getInvCDF() const;
    bool getDOFInfinite() const;
    double getComponentValue() const;
    double getCorrelationValue() const;
    double getNominalValue() const;
    double getSensitivity() const;
    double getStdUncertainty() const;
    double getSymbolValue() const;
    double getWelchSatterthwaiteTerms() const;
    int getDOF() const;
    void addCorrelatedComponent( UncertaintyComponent *component );
    void calculateSensitivity( double stepSize, const QList<double> &values );
    void resetSymbolValue();
    void setCorrelatedComponents(
        const QList<UncertaintyComponent *> &components
    );
    void setInputParameter( InputParameter *parameter );
    void setInputParameterId( const QUuid &id );
    void setSentitivity( double sensitivity );
    void setSymbolValue( double value );

    inline static const QStringList headerLabels {
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
    inline static const QList<int> columnWidths {
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

private:
    InputParameter *mInputParameter;
    QList<UncertaintyComponent *> mCorrelatedComponents;
    QUuid mInputParameterId;
    double mSensitivity;

    static constexpr double sStepSize { 1e-6 };
};

#endif // UNCERTAINTYCOMPONENT_H
