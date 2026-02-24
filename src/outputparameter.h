// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef OUTPUTPARAMETER_H
#define OUTPUTPARAMETER_H

#include "correlation.h"
#include "third_party/exprtk/exprtk.hpp"
#include "mixedcopulasampler.h"
#include "modelcontrol.hpp"
#include "montecarlo.h"
#include "uncertaintycomponent.h"
#include "parameter.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>
#include <string>

typedef exprtk::expression<double> expression_t;
typedef exprtk::parser<double> parser_t;

// Class that stores the data of an output parameter and defines the static
// ModelControl object that stores the output parameters. Combined uncertainty
// and uncertainty budget results (GUM JCGM:100) are calculated using the
// compile() method. The class contains MonteCarlo and MixedCopulaSampler
// objects to perform the Monte Carlo simulation (GUM JCGM:101), started via
// startMonteCarlo().
class OutputParameter: public Parameter {
    Q_OBJECT
    QML_ELEMENT

    typedef typename parser_t::dependent_entity_collector::symbol_t symbol_t;

public:
    OutputParameter( QObject* parent = nullptr );
    OutputParameter( const OutputParameter &op );
    ~OutputParameter();

    OutputParameter& operator= ( const OutputParameter &op );
    bool operator== ( const OutputParameter &op ) const;
    bool operator!= ( const OutputParameter &op ) const;

    MixedCopulaSampler getMixedCopulaSampler() const;
    MonteCarlo getMonteCarlo() const;
    QJsonObject toJson() const;
    QList<UncertaintyComponent> getComponents() const;
    QList<double> getHistogramValues();
    QString componentsToString() const;
    QString getComponentContributionAsString( const int &row ) const;
    QString getContributionAsString( const int &row, const int &column ) const;
    QString getCorrelationContributionAsString( const int &row ) const;
    QString getTotalContributionAsString( const int &row ) const;
    QString histogramToString() const;
    QString resultsToString() const;
    QString toString() const;
    QStringList getMonteCarloResults( const bool &csvMode = false ) const;
    QStringList getResults( const bool &csvMode = false ) const;
    QVariant get( const int &column, const bool &csvMode = false ) const;
    QVariant getResult( const int &column, const bool &csvMode = false ) const;
    UncertaintyComponent *getComponent( const int &row ) const;
    bool addToModel( const bool &resetMonteCarlo = true );
    bool getMonteCarloValid() const;
    bool isInputParameterReferenced( InputParameter *inputParameter ) const;
    bool updateSelectedModelRow();
    double getCombinedStdUncertainty() const;
    double getComponentContribution( const int &row ) const;
    double getContribution( const int &row, const int &column ) const;
    double getCorrelationContribution( const int &row ) const;
    double getCoverageFactor() const;
    double getExpandedUncertainty() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getMonteCarloConvergenceFactor() const;
    double getTotalContribution( const int &row ) const;
    int getEffectiveDOF() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    int getNumberOfComponents() const;
    std::wstring getFormulaStdWString() const;
    void addComponent( const UncertaintyComponent &component );
    void clearComponents();
    void resetResults( const bool &resetMonteCarlo = true );
    void resetSymbolValues();
    void set( const int &column, const QVariant &value );
    void setComponents( const QList<UncertaintyComponent> &components );
    void setError( const QString &error = "" );
    void setLocked( const bool &locked ) override;
    void setMixedCopulaSampler( const MixedCopulaSampler &mixedCopulaSampler );
    void setMonteCarlo( const MonteCarlo &monteCarlo );
    void setRandomSymbolValues();
    void startMonteCarlo();
    void stopMonteCarlo();

    Q_INVOKABLE QString getConfidenceAsString() const;
    Q_INVOKABLE QString getError( const bool &csvMode = false ) const;
    Q_INVOKABLE QString getFormula( const bool &csvMode = false ) const;
    Q_INVOKABLE QString getNominalValueAsString(
        const bool &csvMode = false
    ) const;
    Q_INVOKABLE double getConfidence() const;
    Q_INVOKABLE void compile( const bool &resetMonteCarlo = true );
    Q_INVOKABLE void setConfidence( const double &confidence );
    Q_INVOKABLE void setFormula( const QString &formula );

    static ModelControl<OutputParameter *> *getOutputModel();
    static OutputParameter fromJson(
        const QJsonObject &json,
        const bool &addToModel = true,
        QObject *parent = nullptr
    );
    static QJsonArray parametersToJson();
    static QString parametersToString();
    static bool validName(
        const QString &name,
        const bool &checkCurrentSelection = false
    );
    static void parametersFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );
    static void setCollectVariables( const bool &collect );

    static QList<int> columnWidths;
    static QList<int> resultColumnWidths;
    static QString defaultName;
    static QStringList headerLabels;
    static QStringList resultLabels;
    static parser_t parser;

signals:
    void lockedChanged();

private:
    QString getCombinedStdUncertaintyAsString(
        const bool &csvMode = false
    ) const;
    QString getCoverageFactorAsString( const bool &csvMode = false ) const;
    QString getEffectiveDOFAsString() const;
    QString getExpandedUncertaintyAsString( const bool &csvMode = false ) const;
    QString getVariables() const;
    bool allComponentsNormal() const;
    void createConnections();

    const QString mEmptyExpressionString { "Measurement function is empty" };
    const QString mInvalidNominalString { "Nominal output value is invalid: " };
    const QString mNoInputParametersString { "No input parameters detected" };
    const QString mRegExErrorReplace { "!" };
    const QString mValidExpressionString { "Input parameters detected: " };
    const QString variableSeparator { ", " };
    const double mDoubleNaN { std::numeric_limits<double>::quiet_NaN() };
    const int mErrorCodeLength { 9 };
    const int mMaxInt { std::numeric_limits<int>::max() };
    MixedCopulaSampler mMixedCopulaSampler;
    MonteCarlo mMonteCarlo;
    QList<UncertaintyComponent> mComponents;
    QString mError;
    QString mFormula;
    double mConfidence;

    static ModelControl<OutputParameter *> mOutputModel;
    static QString mCombinedUncertaintyHeaderString;
    static QString mConfidenceString;
    static QString mErrorString;
    static QString mFormulaString;
    static QString mMonteCarloHeaderString;
    static QString mMonteCarloString;
    static QString mOutputParametersHeaderString;
    static QString mUncertaintyComponentsHeaderString;
};

#endif // OUTPUTPARAMETER_H
