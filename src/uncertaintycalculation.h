// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNCERTAINTYCALCULATION_H
#define UNCERTAINTYCALCULATION_H

#include "budgetmodel.h"
#include "correlation.h"
#include "inputparameter.h"
#include "modelcontrol.hpp"
#include "outputparameter.h"
#include "resultsmodel.h"
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

// The UncertaintyCalculation class orchestrates the different parts of the
// uncertainty calculation and interfaces with the QML UI layer
class UncertaintyCalculation : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    UncertaintyCalculation( QObject *parent = nullptr );
    ~UncertaintyCalculation();

    Correlation *getSelectedCorrelation() const;
    InputParameter *getSelectedInputParameter() const;
    OutputParameter *getSelectedOutputParameter() const;
    QStringList getDistributionStrings() const;
    QStringList getUnits() const;

    Q_INVOKABLE QList<int> getBudgetColumnWidths() const;
    Q_INVOKABLE QList<int> getCorrelationColumnWidths() const;
    Q_INVOKABLE QList<int> getInputColumnWidths() const;
    Q_INVOKABLE QList<int> getOutputColumnWidths() const;
    Q_INVOKABLE QList<int> getResultsColumnWidths() const;
    Q_INVOKABLE QString getMonteCarloHeader( const int &column ) const;
    Q_INVOKABLE QString getNewInputParameterName() const;
    Q_INVOKABLE QString getNewOutputParameterName() const;
    Q_INVOKABLE QString getSelectedCorrelationReferences() const;
    Q_INVOKABLE QString getSelectedInputParameterReferences() const;
    Q_INVOKABLE bool inputParameterIsConstant( const QString &name ) const;
    Q_INVOKABLE bool loadProject( const QUrl &url );
    Q_INVOKABLE bool saveCSV( const QUrl &url );
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool saveProject( const QUrl &url );
    Q_INVOKABLE bool validInputName(
        const QString &name,
        const bool &checkCurrentSelection = false
    ) const;
    Q_INVOKABLE bool validOutputName(
        const QString &name,
        const bool &checkCurrentSelection = false
    ) const;
    Q_INVOKABLE const BudgetModel *budgetItemModel() const;
    Q_INVOKABLE const QItemSelectionModel *correlationSelectionModel() const;
    Q_INVOKABLE const QItemSelectionModel *inputSelectionModel() const;
    Q_INVOKABLE const QItemSelectionModel *outputSelectionModel() const;
    Q_INVOKABLE const QObject *correlationItemModel() const;
    Q_INVOKABLE const QObject *inputItemModel() const;
    Q_INVOKABLE const QObject *outputItemModel() const;
    Q_INVOKABLE const QStringListModel *distributionsModel() const;
    Q_INVOKABLE const QStringListModel *unitsModel() const;
    Q_INVOKABLE const ResultsModel *resultsItemModel() const;
    Q_INVOKABLE void addCorrelation( Correlation *correlation );
    Q_INVOKABLE void addInputParameter( InputParameter *parameter );
    Q_INVOKABLE void addOutputParameter( OutputParameter *parameter );
    Q_INVOKABLE void addUnit( const QString &name );
    Q_INVOKABLE void clearProject( const bool &unsavedChanges = true );
    Q_INVOKABLE void newProject();
    Q_INVOKABLE void removeCorrelation();
    Q_INVOKABLE void removeInputParameter();
    Q_INVOKABLE void removeOutputParameter();
    Q_INVOKABLE void resetDisplay();
    Q_INVOKABLE void runMonteCarlo();
    Q_INVOKABLE void stopMonteCarlo();
    Q_INVOKABLE void updateCorrelation( Correlation *correlation );
    Q_INVOKABLE void updateInputParameter( InputParameter *parameter );
    Q_INVOKABLE void updateOutputParameter( OutputParameter *parameter );

    Q_PROPERTY(
        QList<double> histogramValues
        READ getHistogramValues
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        QString inputName
        READ getInputName
        NOTIFY inputParameterChanged
    )
    Q_PROPERTY(
        QString outputName
        READ getOutputName
        NOTIFY resultsChanged
    )
    Q_PROPERTY(
        QString outputUnit
        READ getOutputUnit
        NOTIFY resultsChanged
    )
    Q_PROPERTY(
        QString projectFileName
        READ getProjectFileName
        NOTIFY projectFilePathChanged
    )
    Q_PROPERTY(
        QStringList monteCarloResultsList
        READ getMonteCarloResults
        NOTIFY monteCarloResultsListChanged
    )
    Q_PROPERTY(
        QUrl projectFilePath
        READ getProjectFilePath
        NOTIFY projectFilePathChanged
    )
    Q_PROPERTY(
        bool histogramValid
        READ getHistogramValid
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        bool outputLocked
        READ getOutputLocked
        NOTIFY outputLockedChanged
    )
    Q_PROPERTY(
        bool outputValid
        READ getOutputValid
        NOTIFY resultsChanged
    )
    Q_PROPERTY(
        bool unsavedChanges
        READ getUnsavedChanges
        NOTIFY unsavedChangesChanged
    )
    Q_PROPERTY(
        double histogramXMax
        READ getHistogramXMax
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        double histogramXMin
        READ getHistogramXMin
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        double histogramYMax
        READ getHistogramYMax
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        double monteCarloConvergenceFactor
        READ getMonteCarloConvergenceFactor
        NOTIFY monteCarloConvergenceFactorChanged
    )
    Q_PROPERTY(
        int histogramHigherIndex
        READ getHistogramHigherIndex
        NOTIFY monteCarloValuesChanged
    )
    Q_PROPERTY(
        int histogramLowerIndex
        READ getHistogramLowerIndex
        NOTIFY monteCarloValuesChanged
    )

public slots:
    void lockItemSelectionModels();
    void setOutputRow();
    void unsavedChanges();

signals:
    void inputParameterChanged();
    void monteCarloConvergenceFactorChanged();
    void monteCarloResultsChanged();
    void monteCarloResultsListChanged();
    void monteCarloValuesChanged();
    void outputLockedChanged();
    void projectFilePathChanged();
    void resultsChanged();
    void unsavedChangesChanged();

private:
    QJsonObject projectToJson() const;
    QList<double> getHistogramValues() const;
    QString getInputName() const;
    QString getOutputName() const;
    QString getOutputUnit() const;
    QString getProjectFileName() const;
    QString outputParameterReferencesToString(
        const QStringList &references
    ) const;
    QString projectToCSVString() const;
    QStringList getInputParameterReferences(
        InputParameter * const &inputParameter
    ) const;
    QStringList getMonteCarloResults() const;
    QUrl getProjectFilePath() const;
    bool getHistogramValid() const;
    bool getOutputLocked() const;
    bool getOutputValid() const;
    bool getUnsavedChanges() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getMonteCarloConvergenceFactor() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    void connectToOutputParameter( const OutputParameter *parameter);
    void emitAllResultsChanged();
    void emitOutputModelsAboutToBeReset();
    void emitOutputModelsReset();
    void parametersFromJson( const QJsonObject &json );
    void recompileExpressions(
        const bool recompileInvalidExpressions = false,
        InputParameter *inputParameterA = nullptr,
        InputParameter *inputParameterB = nullptr
    );
    void resetProjectFilePath();
    void setProjectFilePath( const QUrl &projectFilePath );
    void setUnsavedChanges( const bool &unsavedChanges );
    void updateUnits();

    const QString mCorrelationsString { "correlations" };
    const QString mCSVFailedString { "CSV file '%1' could not be saved." };
    const QString mInputParametersString { "inputParameters" };
    const QString mLoadFailedString {
        "Project file '%1' could not be loaded."
    };
    const QString mOutputParametersString { "outputParameters" };
    const QString mSaveFailedString { "Project file '%1' could not be saved." };
    const QString mVersionString { "Sigma version" };
    const QStringList mDefaultUnits {
        "A",
        "°F",
        "cd",
        "h",
        "Hz",
        "J",
        "K",
        "L",
        "N",
        "Nm",
        "Pa",
        "V",
        "W",
        "deg",
        "g",
        "gram",
        "km/h",
        "kN",
        "kg",
        "kW",
        "m",
        "m/s",
        "m/s²",
        "m²",
        "m³",
        "mm",
        "Ω",
        "rad",
        "s",
        "°C",
        "%"
    };
    const int maxNameAttempts { 100 };
    BudgetModel mBudgetModel;
    ModelControl<Correlation *> *mCorrelationModel;
    ModelControl<InputParameter *> *mInputParametersModel;
    ModelControl<OutputParameter *> *mOutputParametersModel;
    QStringList mUnits;
    QStringListModel mDistributionModel;
    QStringListModel mUnitsModel;
    QUrl mProjectFilePath;
    ResultsModel mResultsModel;
    bool mUnsavedChanges;
    int mOutputRow;
};

#endif // UNCERTAINTYCALCULATION_H
