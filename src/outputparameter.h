// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef OUTPUTPARAMETER_H
#define OUTPUTPARAMETER_H

#include "correlation.h"
#include "third_party/exprtk/exprtk.hpp"
#include "mixedcopulasampler.h"
#include "jsondiff.h"
#include "modelcontrol.hpp"
#include "montecarlo.h"
#include "uncertaintycomponent.h"
#include "parameter.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1StringView>
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
    OutputParameter( QObject *parent = nullptr );
    OutputParameter( const OutputParameter &op );

    OutputParameter& operator= ( const OutputParameter &op );
    bool operator== ( const OutputParameter &op ) const;
    bool operator!= ( const OutputParameter &op ) const;

    DataType dataType() const override;
    MixedCopulaSampler getMixedCopulaSampler() const;
    MonteCarlo getMonteCarlo() const;
    OutputParameter *appendToModel();
    QJsonObject toJson() const override;
    QList<UncertaintyComponent> getComponents() const;
    QList<double> getHistogramValues() const;
    QString componentsToCSVString() const;
    QString getComponentContributionAsString( int row ) const;
    QString getContributionAsString( int row, int column ) const;
    QString getCorrelationContributionAsString( int row ) const;
    QString getTotalContributionAsString( int row ) const;
    QString histogramToCSVString() const;
    QString resultsToCSVString() const;
    QString toCSVString() const;
    QStringList getMonteCarloResults( bool csvMode = false ) const;
    QStringList getResults( bool csvMode = false ) const;
    QVariant get( int column, bool csvMode = false ) const override;
    QVariant getResult( int column, bool csvMode = false ) const;
    QVariant headerData( int column ) const override;
    UncertaintyComponent *getComponent( int row ) const;
    bool getMonteCarloValid() const;
    bool isInputParameterReferenced( const QUuid &id ) const;
    double getCombinedStdUncertainty() const;
    double getComponentContribution( int row ) const;
    double getContribution( int row, int column ) const;
    double getCorrelationContribution( int row ) const;
    double getCoverageFactor() const;
    double getExpandedUncertainty() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getMonteCarloConvergenceFactor() const;
    double getTotalContribution( int row ) const;
    int columnCount() const override;
    int getEffectiveDOF() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    int getNumberOfComponents() const;
    std::wstring getFormulaStdWString() const;
    void addComponent( const UncertaintyComponent &component );
    void clearComponents();
    void resetResults( bool resetMonteCarlo = true );
    void resetSymbolValues();
    void set( int column, const QVariant &value ) override;
    void setComponents( const QList<UncertaintyComponent> &components );
    void setError( const QString &error = "" );
    void setLocked( bool locked ) override;
    void setMixedCopulaSampler( const MixedCopulaSampler &mixedCopulaSampler );
    void setMonteCarlo( const MonteCarlo &monteCarlo );
    void setRandomSymbolValues();
    void startMonteCarlo();
    void stopMonteCarlo();
    void updateFromJson( const QJsonObject &json ) override;

    Q_INVOKABLE QString getConfidenceAsString() const;
    Q_INVOKABLE QString getError( bool csvMode = false ) const;
    Q_INVOKABLE QString getFormula( bool csvMode = false ) const;
    Q_INVOKABLE QString getNominalValueAsString( bool csvMode = false ) const;
    Q_INVOKABLE double getConfidence() const;
    Q_INVOKABLE void compile( bool resetMonteCarlo = true );
    Q_INVOKABLE void setConfidence( double confidence );
    Q_INVOKABLE void setFormula( const QString &formula );
    Q_INVOKABLE void setToSelected();

    static ModelControl<OutputParameter *> *getOutputModel();
    static OutputParameter fromJson(
        const QJsonObject &json,
        bool appendToModel = true,
        QObject *parent = nullptr
    );
    static OutputParameter *getByName( const QString &name );
    static OutputParameter *getById( const QUuid &id );
    static OutputParameter *getSelected();
    static QJsonArray parametersToJson();
    static QJsonObject currentJson( const QUuid &id );
    static const QList<OutputParameter *> &getAll();
    static QString parametersToCSVString();
    static QUuid getSelectedId();
    static bool remove( const QUuid &id );
    static bool update( const QUuid &id, OutputParameter *parameter );
    static bool validName(
        const QString &name,
        bool checkCurrentSelection = false
    );
    static int getRowIndex( const QUuid &id );
    static int getSelectedRow();
    static void applyDiff( const JsonDiff &diff );
    static void clearModel();
    static void parametersFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );
    static void onDisplayPrecisionChanged();
    static void recompileAllExpressions( bool resetMonteCarlo = false );
    static void setCollectVariables( bool collect );
    static void setSelectionLocked( bool locked );

    static constexpr QLatin1StringView defaultName { "Y" };
    inline static const QList<int> columnWidths { 120, 180, 55, 140 };
    inline static const QList<int> resultColumnWidths {
        100,
        55,
        120,
        212,
        198,
        120,
        155,
        140
    };
    inline static const QStringList headerLabels {
        "Output name",
        "Measurement function",
        "Unit",
        "Level of confidence"
    };
    inline static const QStringList resultLabels {
        "Output name",
        "Unit",
        "Output estimate",
        "Combined standard uncertainty",
        "Effective degrees of freedom",
        "Coverage factor",
        "Expanded uncertainty",
        "Level of confidence"
    };
    inline static parser_t parser {};

signals:
    void lockedChanged();
    void monteCarloConvergenceFactorChanged();
    void monteCarloFinished();
    void monteCarloStarted();
    void monteCarloStatusChanged();

private:
    OutputParameter *insertIntoModel( int row );
    QString getCombinedStdUncertaintyAsString( bool csvMode = false ) const;
    QString getCoverageFactorAsString( bool csvMode = false ) const;
    QString getEffectiveDOFAsString() const;
    QString getExpandedUncertaintyAsString( bool csvMode = false ) const;
    QString getVariables() const;
    bool allComponentsNormal() const;
    void createConnections();

    MixedCopulaSampler mMixedCopulaSampler;
    MonteCarlo mMonteCarlo;
    QList<UncertaintyComponent> mComponents;
    QString mError;
    QString mFormula;
    double mConfidence;

    static ModelControl<OutputParameter *> mOutputModel;

    static constexpr QLatin1StringView sEmptyExpressionString {
        "Measurement function is empty"
    };
    static constexpr QLatin1StringView sInvalidNominalString {
        "Nominal output value is invalid: "
    };
    static constexpr QLatin1StringView sNoInputParametersString {
        "No input parameters detected"
    };
    static constexpr QLatin1StringView sRegExErrorReplace { "!" };
    static constexpr QLatin1StringView sValidExpressionString {
        "Input parameters detected: "
    };
    static constexpr QLatin1StringView sCombinedUncertaintyHeaderString {
        "Combined uncertainty:"
    };
    static constexpr QLatin1StringView sConfidenceString { "confidence" };
    static constexpr QLatin1StringView sFormulaString { "formula" };
    static constexpr QLatin1StringView sMonteCarloHeaderString {
        "Monte Carlo simulation:"
    };
    static constexpr QLatin1StringView sMonteCarloString { "monteCarlo" };
    static constexpr QLatin1StringView sOutputParametersHeaderString {
        "Output parameters:"
    };
    static constexpr QLatin1StringView sUncertaintyComponentsHeaderString {
        "Uncertainty components "
    };
    static constexpr int sErrorCodeLength { 9 };
};

#endif // OUTPUTPARAMETER_H
