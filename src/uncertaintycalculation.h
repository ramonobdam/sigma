// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNCERTAINTYCALCULATION_H
#define UNCERTAINTYCALCULATION_H

#include "applicationsettings.h"
#include "budgetmodel.h"
#include "correlation.h"
#include "diffutil.h"
#include "inputparameter.h"
#include "modelcontrol.hpp"
#include "outputparameter.h"
#include "resultsmodel.h"
#include "undohistorymodel.h"
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include <QJsonObject>
#include <QLatin1StringView>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QUrl>
#include <QUuid>

// The UncertaintyCalculation class orchestrates the different parts of the
// uncertainty calculation, wraps data manipulation actions in transactions for
// the undo/redo system, and interfaces with the QML UI layer
class UncertaintyCalculation : public QObject {
    Q_OBJECT

public:
    UncertaintyCalculation(
        QObject *parent = nullptr,
        ApplicationSettings *appSettings = new ApplicationSettings()
    );

    QJsonObject projectToJson() const;
    QString getProjectFileName() const;
    QStringList getOutputParameterNames() const;
    QStringList getUnits() const;
    bool getHistogramValid() const;
    bool selectOutputParameterByName( const QString &name );
    void projectFromJson( const QJsonObject &json );

    Q_INVOKABLE QList<int> getBudgetColumnWidths() const;
    Q_INVOKABLE QList<int> getCorrelationColumnWidths() const;
    Q_INVOKABLE QList<int> getInputColumnWidths() const;
    Q_INVOKABLE QList<int> getOutputColumnWidths() const;
    Q_INVOKABLE QList<int> getResultsColumnWidths() const;
    Q_INVOKABLE QString getMonteCarloHeader( int column ) const;
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
    Q_INVOKABLE BudgetModel *budgetItemModel();
    Q_INVOKABLE QItemSelectionModel *correlationSelectionModel();
    Q_INVOKABLE QItemSelectionModel *inputSelectionModel();
    Q_INVOKABLE QItemSelectionModel *outputSelectionModel();
    Q_INVOKABLE QAbstractTableModel *correlationItemModel();
    Q_INVOKABLE QAbstractTableModel *inputItemModel();
    Q_INVOKABLE QAbstractTableModel *outputItemModel();
    Q_INVOKABLE QStringListModel *distributionsModel();
    Q_INVOKABLE QStringListModel *unitsModel();
    Q_INVOKABLE ResultsModel *resultsItemModel();
    Q_INVOKABLE UndoHistoryModel *undoHistoryModel();
    Q_INVOKABLE void addCorrelation( const Correlation *correlation );
    Q_INVOKABLE void addInputParameter( const InputParameter *parameter );
    Q_INVOKABLE void addOutputParameter( const OutputParameter *parameter );
    Q_INVOKABLE void addUnit( const QString &name );
    Q_INVOKABLE void newProject();
    Q_INVOKABLE void removeCorrelation();
    Q_INVOKABLE void removeInputParameter();
    Q_INVOKABLE void removeOutputParameter();
    Q_INVOKABLE void restoreState( int index );
    Q_INVOKABLE void runMonteCarlo();
    Q_INVOKABLE void stopMonteCarlo();
    Q_INVOKABLE void updateCorrelation( const Correlation *correlation );
    Q_INVOKABLE void updateInputParameter( const InputParameter *parameter );
    Q_INVOKABLE void updateOutputParameter( const OutputParameter *parameter );
    Q_INVOKABLE void userClearProject();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    Q_PROPERTY(
        QList<double> histogramValues
        READ getHistogramValues
        NOTIFY histogramValuesChanged
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
        bool canRedo
        READ canRedo
        NOTIFY canRedoChanged
    )
    Q_PROPERTY(
        bool canUndo
        READ canUndo
        NOTIFY canUndoChanged
    )
    Q_PROPERTY(
        bool histogramValid
        READ getHistogramValid
        NOTIFY histogramValuesChanged
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
        NOTIFY histogramValuesChanged
    )
    Q_PROPERTY(
        double histogramXMin
        READ getHistogramXMin
        NOTIFY histogramValuesChanged
    )
    Q_PROPERTY(
        double histogramYMax
        READ getHistogramYMax
        NOTIFY histogramValuesChanged
    )
    Q_PROPERTY(
        double monteCarloConvergenceFactor
        READ getMonteCarloConvergenceFactor
        NOTIFY monteCarloConvergenceFactorChanged
    )
    Q_PROPERTY(
        int histogramHigherIndex
        READ getHistogramHigherIndex
        NOTIFY histogramValuesChanged
    )
    Q_PROPERTY(
        int histogramLowerIndex
        READ getHistogramLowerIndex
        NOTIFY histogramValuesChanged
    )
    Q_PROPERTY(
        int undoCursor
        READ getUndoCursor
        NOTIFY undoCursorChanged
    )

public slots:
    void lockItemSelectionModels();
    void onMonteCarloFinished();
    void onTransactionApplied();
    void unsavedChanges();

signals:
    void canUndoChanged();
    void canRedoChanged();
    void inputParameterChanged();
    void monteCarloConvergenceFactorChanged();
    void monteCarloFinished();
    void monteCarloResultsChanged();
    void monteCarloResultsListChanged();
    void histogramValuesChanged();
    void outputLockedChanged();
    void projectFilePathChanged();
    void resultsChanged();
    void undoCursorChanged();
    void unsavedChangesChanged();

private:
    QList<double> getHistogramValues() const;
    QString getInputName() const;
    QString getOutputName() const;
    QString getOutputUnit() const;
    QString outputParameterReferencesToString(
        const QStringList &references
    ) const;
    QString projectToCSVString() const;
    QStringList getDistributionStrings() const;
    QStringList getInputParameterReferences(
        const InputParameter *inputParameter
    ) const;
    QStringList getMonteCarloResults() const;
    QUrl getProjectFilePath() const;
    bool canRedo() const;
    bool canUndo() const;
    bool getOutputLocked() const;
    bool getOutputValid() const;
    bool getUnsavedChanges() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getMonteCarloConvergenceFactor() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    int getUndoCursor() const;
    void clearProject( bool unsavedChanges = true );
    void connectToOutputParameter( const OutputParameter *parameter);
    void createConnections();
    void emitAllResultsChanged();
    void recompileExpressions(
        DiffUtil &diffUtil,
        const bool recompileInvalidExpressions,
        const QUuid inputParamAId = QUuid(),
        const QUuid inputParamBId = QUuid()
    );
    void registerDiffUtilDataTypeMethods();
    void resetDisplay();
    void resetProjectFilePath();
    void setProjectFilePath( const QUrl &projectFilePath );
    void setUnsavedChanges(  bool unsavedChanges );
    void updateUnits();

    static constexpr QLatin1StringView sCorrelationsString {
        "correlations"
    };
    static constexpr QLatin1StringView sCSVFailedString {
        "CSV file '%1' could not be saved."
    };
    static constexpr QLatin1StringView sInputParametersString {
        "inputParameters"
    };
    static constexpr QLatin1StringView sLoadFailedString {
        "Project file '%1' could not be loaded."
    };
    static constexpr QLatin1StringView sOutputParametersString {
        "outputParameters"
    };
    static constexpr QLatin1StringView sSaveFailedString {
        "Project file '%1' could not be saved."
    };
    static constexpr QLatin1StringView sVersionString { "Sigma version" };
    static constexpr int sMaxNameAttempts { 100 };
    inline static const QStringList sDefaultUnits {
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

    ApplicationSettings *mAppSettings;
    BudgetModel mBudgetModel;
    // Persistent DiffUtil for Monte Carlo — begin/commit in different methods:
    DiffUtil mDiffUtil;
    QStringList mUnits;
    QStringListModel mDistributionModel;
    QStringListModel mUnitsModel;
    QUrl mProjectFilePath;
    ResultsModel mResultsModel;
    UndoHistoryModel mUndoHistoryModel;
    bool mUnsavedChanges;
};

#endif // UNCERTAINTYCALCULATION_H
