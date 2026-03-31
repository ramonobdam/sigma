// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNCERTAINTYCOMPONENT_H
#define UNCERTAINTYCOMPONENT_H

#include "distribution.h"
#include "inputparameter.h"
#include "settings.h"
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>

// UncertaintyComponent objects are used to store the references to the
// InputParameters detected in the measurement formula of an OutputParameter. It
// also provides methods used to calculate the sensitivity of the
// OutputParameter with respect to this component.
class UncertaintyComponent : private Settings {

public:
    UncertaintyComponent();
    UncertaintyComponent( InputParameter *parameter);
    UncertaintyComponent( const UncertaintyComponent &cor );
    ~UncertaintyComponent();

    UncertaintyComponent& operator= ( const UncertaintyComponent &cor );

    Distribution::Type getDistribution() const;
    InputParameter *getInputParameter() const;
    QList<UncertaintyComponent *> getCorrelatedComponents() const;
    QList<double> getEvaluationValues() const;
    QString getDOFAsString() const;
    QString getDistributionAsString() const;
    QString getName( const bool &csvMode = false ) const;
    QString getUnit( const bool &csvMode = false ) const;
    QVariant get(const int &column, const bool &csvMode = false ) const;
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
    void calculateSensitivity(
        const double &stepSize,
        const QList<double> &values
    );
    void resetSymbolValue();
    void setCorrelatedComponents(
        const QList<UncertaintyComponent *> &components
    );
    void setInputParameter( InputParameter *parameter );
    void setSentitivity( const double &sensitivity );
    void setSymbolValue( const double &value );

    static QStringList headerLabels;
    static QList<int> columnWidths;

private:
    const double mStepSize { 1e-6 };
    InputParameter *mInputParameter;
    QList<UncertaintyComponent *> mCorrelatedComponents;
    double mSensitivity;
};

#endif // UNCERTAINTYCOMPONENT_H
