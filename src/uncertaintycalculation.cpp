// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "datatype.h"
#include "montecarlo.h"
#include "settings.h"
#include "stringutils.h"
#include "uncertaintycalculation.h"
#include "undostack.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QIODeviceBase>
#include <QJsonDocument>
#include <QJsonValue>
#include <QList>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>


UncertaintyCalculation::UncertaintyCalculation(
    QObject *parent,
    ApplicationSettings *appSettings
)   :   QObject { parent },
        mUnitsModel { QStringListModel( this ) },
        mDistributionModel { QStringListModel( this ) },
        mBudgetModel { BudgetModel( this ) },
        mResultsModel { ResultsModel ( this) },
        mUnits { sDefaultUnits },
        mProjectFilePath {},
        mUnsavedChanges {},
        mDiffUtil {},
        mAppSettings { appSettings }
{
    mUnitsModel.setStringList( mUnits );
    mUnitsModel.sort( 0 );

    mDistributionModel.setStringList( getDistributionStrings() );

    InputParameter::addConstantsToSymbolTable();
    OutputParameter::setCollectVariables( true );

    createConnections();

    registerDiffUtilDataTypeMethods();
}


QStringList UncertaintyCalculation::getUnits() const {
    return mUnits;
}


QList<int> UncertaintyCalculation::getBudgetColumnWidths() const {
    return UncertaintyComponent::columnWidths;
}


QList<int> UncertaintyCalculation::getCorrelationColumnWidths() const {
    return Correlation::columnWidths;
}


QList<int> UncertaintyCalculation::getInputColumnWidths() const {
    return InputParameter::columnWidths;
}


QList<int> UncertaintyCalculation::getOutputColumnWidths() const {
    return OutputParameter::columnWidths;
}


QList<int> UncertaintyCalculation::getResultsColumnWidths() const {
    return OutputParameter::resultColumnWidths;
}


QString UncertaintyCalculation::getMonteCarloHeader( int column ) const {
    if ( column >= 0 && column < MonteCarlo::headerLabels.size() ) {
        return MonteCarlo::headerLabels[ column ];
    }
    return "";
}


QString UncertaintyCalculation::getNewInputParameterName() const {
    QString newName {};
    int num { 0 };
    do {
        num++;
        newName = InputParameter::defaultName + QString::number( num );
    } while ( !validInputName( newName ) && num < sMaxNameAttempts );
    return newName;
}


QString UncertaintyCalculation::getNewOutputParameterName() const {
    QString newName {};
    int num { 0 };
    do {
        num++;
        newName = OutputParameter::defaultName + QString::number( num );
    } while ( !validOutputName( newName ) && num < sMaxNameAttempts );
    return newName;
}


QString UncertaintyCalculation::getSelectedCorrelationReferences() const {
    // Return a string that lists the OutputParameters that are referenced by
    // both InputParameters of the selected correlation
    QString string {};
    const Correlation *correlation { Correlation::getSelected() };
    if ( correlation ) {
        InputParameter *paramA { correlation->getInputParameterA() };
        InputParameter *paramB { correlation->getInputParameterB() };
        QStringList referencesA { getInputParameterReferences( paramA ) };
        QStringList referencesB { getInputParameterReferences( paramB ) };
        QStringList referencesBoth {};
        for ( QString &nameA : referencesA ) {
            if ( referencesB.contains( nameA ) ) {
                referencesBoth.append( nameA );
            }
        }
        string = outputParameterReferencesToString( referencesBoth );
    }
    return string;
}


QString UncertaintyCalculation::getSelectedInputParameterReferences(
) const {
    // Return a string that lists the OutputParameters that are referencing the
    // selected InputParameter
    const InputParameter *inputParameter { InputParameter::getSelected() };
    QStringList references { getInputParameterReferences( inputParameter ) };
    return outputParameterReferencesToString( references );
}


bool UncertaintyCalculation::inputParameterIsConstant(
    const QString &name
) const {
    return InputParameter::inputParameterIsConstant( name );
}


bool UncertaintyCalculation::loadProject( const QUrl &url ) {
    QString path { url.toLocalFile() };
    QFile loadFile( path );
    if ( !url.isLocalFile() || !loadFile.open( QIODevice::ReadOnly ) ) {
        qDebug() << QString( sLoadFailedString ).arg( path );
        resetProjectFilePath();
        return false;
    }
    QByteArray projectData = loadFile.readAll();
    QJsonDocument projectDoc( QJsonDocument::fromJson( projectData ) );
    clearProject( false );  // Currently open project should not be saved
    projectFromJson( projectDoc.object() );
    setProjectFilePath( url );
    setUnsavedChanges( false );

    // Remove transactions from the previous project
    UndoStack::instance().clear();

    return true;
}


bool UncertaintyCalculation::saveCSV( const QUrl &url ) {
    QString path { url.toLocalFile() };
    QSaveFile csvFile( path );
    if (
        !url.isLocalFile() ||
        !csvFile.open( QIODeviceBase::WriteOnly | QIODeviceBase::Text )
    ) {
        qDebug() << QString( sCSVFailedString ).arg( path );
        return false;
    }
    QTextStream out( &csvFile );
    out << projectToCSVString();
    csvFile.commit();
    return true;
}


bool UncertaintyCalculation::saveProject() {
    QUrl url { getProjectFilePath() };
    if ( url.isLocalFile() ) {
        return saveProject( url );
    }
    return false;
}


bool UncertaintyCalculation::saveProject( const QUrl &url ) {
    QString path { url.toLocalFile() };
    QSaveFile saveFile( path );
    if (
        !url.isLocalFile() ||
        !saveFile.open( QIODeviceBase::WriteOnly | QIODeviceBase::Text )
    ) {
        qDebug() << QString( sSaveFailedString ).arg( path );
        resetProjectFilePath();
        return false;
    }
    QJsonObject project { projectToJson() };
    saveFile.write( QJsonDocument( project ).toJson() );
    saveFile.commit();
    setProjectFilePath( url );
    setUnsavedChanges( false );
    return true;
}


bool UncertaintyCalculation::validInputName(
    const QString &name,
    const bool &checkCurrentSelection
) const {
    return InputParameter::validName( name, checkCurrentSelection );
}


bool UncertaintyCalculation::validOutputName(
    const QString &name,
    const bool &checkCurrentSelection
) const {
    return OutputParameter::validName( name, checkCurrentSelection );
}


const BudgetModel * UncertaintyCalculation::budgetItemModel() const {
    return &mBudgetModel;
}


const QItemSelectionModel * UncertaintyCalculation::correlationSelectionModel(
) const {
    return Correlation::getCorrelationModel()->selectionModel();
}


const QItemSelectionModel * UncertaintyCalculation::inputSelectionModel(
) const {
    return InputParameter::getInputModel()->selectionModel();
}


const QItemSelectionModel * UncertaintyCalculation::outputSelectionModel(
) const {
    return OutputParameter::getOutputModel()->selectionModel();
}


const QObject * UncertaintyCalculation::correlationItemModel() const {
    return Correlation::getCorrelationModel()->itemModel();
}


const QObject * UncertaintyCalculation::inputItemModel() const {
    return InputParameter::getInputModel()->itemModel();
}


const QObject * UncertaintyCalculation::outputItemModel() const {
    return OutputParameter::getOutputModel()->itemModel();
}


const QStringListModel * UncertaintyCalculation::distributionsModel() const {
    return &mDistributionModel;
}


const QStringListModel * UncertaintyCalculation::unitsModel() const {
    return &mUnitsModel;
}


const ResultsModel * UncertaintyCalculation::resultsItemModel() const {
    return &mResultsModel;
}


void UncertaintyCalculation::addCorrelation( const Correlation *correlation ) {
    if ( correlation ) {
        Correlation newCorrelation { *correlation };
        newCorrelation.resetId();   // Fresh Id in the model
        Correlation *addedCorrelation { newCorrelation.appendToModel() };
        if ( addedCorrelation ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshotOfNewObject( addedCorrelation );

            // Recompile expressions that use both correlated InputParameters
            // when the new correlation is not zero
            if ( addedCorrelation->getCorrelation() != 0. ) {
                recompileExpressions(
                    diffUtil,
                    false,
                    addedCorrelation->getInputParameterAId(),
                    addedCorrelation->getInputParameterBId()
                );
            }

            diffUtil.commitChanges(
                "add correlation between " +
                addedCorrelation->getInputParameterNameA() +
                " & " +
                addedCorrelation->getInputParameterNameB()
            );

            setUnsavedChanges( true );
        }
    }
}


void UncertaintyCalculation::addInputParameter(
    const InputParameter *parameter
) {
    if ( parameter ) {
        InputParameter newParameter { *parameter };
        newParameter.resetId();   // Fresh Id in the model
        InputParameter *addedParameter { newParameter.appendToModel() };
        if ( addedParameter ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshotOfNewObject( addedParameter );

            recompileExpressions( diffUtil, true );

            diffUtil.commitChanges(
                "add input parameter " + addedParameter->getName()
            );

            setUnsavedChanges( true );
        }
    }
}


void UncertaintyCalculation::addOutputParameter(
    const OutputParameter *parameter
) {
    if ( parameter ) {
        OutputParameter newParameter { *parameter };
        // Fresh Id in the model, compile expression and unclock
        newParameter.resetId();
        newParameter.compile();
        newParameter.setLocked( false );
        OutputParameter *addedParameter { newParameter.appendToModel() };
        if ( addedParameter ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshotOfNewObject( addedParameter );

            diffUtil.commitChanges(
                "add output parameter " + addedParameter->getName()
            );

            connectToOutputParameter( addedParameter );

            setUnsavedChanges( true );
        }
    }
}


void UncertaintyCalculation::addUnit( const QString &name ) {
    if ( !mUnits.contains( name ) ) {
        mUnits.append( name );
        mUnitsModel.setStringList( mUnits );
        mUnitsModel.sort( 0 );
    }
}


void UncertaintyCalculation::newProject() {
    clearProject();
    resetProjectFilePath();

    // Remove transactions from the previous project
    UndoStack::instance().clear();
}


void UncertaintyCalculation::removeCorrelation() {
    QUuid id { Correlation::getSelectedId() };
    if ( !id.isNull() ) {
        Correlation *correlation { Correlation::getById( id ) };
        if ( correlation ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshot( correlation );

            // Store needed data before removal
            QUuid paramAId { correlation->getInputParameterAId() };
            QUuid paramBId { correlation->getInputParameterBId() };
            const double correlationCoeff { correlation->getCorrelation() };
            const QString nameA { correlation->getInputParameterNameA() };
            const QString nameB { correlation->getInputParameterNameB() };

            if ( Correlation::remove( id ) ) {
                // Recompile the expressions that referenced the InputParameters
                // of the deleted correlation when their correlation was not
                // zero
                if ( correlationCoeff != 0. ) {
                    recompileExpressions( diffUtil, false, paramAId, paramBId );
                }

                diffUtil.commitChanges(
                    "delete correlation between " + nameA + " & " + nameB
                );

                setUnsavedChanges( true );
            }
        }
    }
}


void UncertaintyCalculation::removeInputParameter() {
    QUuid id { InputParameter::getSelectedId() };
    if ( !id.isNull() ) {
        InputParameter *parameter { InputParameter::getById( id ) };
        if ( parameter ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshot( parameter );

            // Store needed data before removal
            const QString name { parameter->getName() };
            const QUuid id { parameter->getId() };
            if ( InputParameter::remove( id ) ) {
                // Remove correlations that reference the deleted InputParameter
                const QList<Correlation *> &correlations {
                    Correlation::getCorrelationsForInputParameter( id )
                };
                for ( Correlation *correlation: correlations ) {
                    diffUtil.takeSnapshot( correlation );
                    Correlation::remove( correlation->getId() );
                }

                // Recompile the expressions that reference the deleted
                // InputParameter
                recompileExpressions( diffUtil, false, id );

                diffUtil.commitChanges( "delete input parameter " + name );

                setUnsavedChanges( true );
            }
        }
    }
}


void UncertaintyCalculation::removeOutputParameter() {
    QUuid id { OutputParameter::getSelectedId() };
    if ( !id.isNull() ) {
        OutputParameter *parameter { OutputParameter::getById( id ) };
        if ( parameter ) {
            DiffUtil diffUtil {};
            diffUtil.takeSnapshot( parameter );

            // Store needed data before removal
            const QString name { parameter->getName() };
            if ( OutputParameter::remove( id ) ) {
                diffUtil.commitChanges( "delete output parameter " + name );

                emitAllResultsChanged();

                setUnsavedChanges( true );
            }
        }
    }
}


void UncertaintyCalculation::runMonteCarlo() {
    OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && !parameter->getLocked() ) {
        mDiffUtil.takeSnapshot( parameter );
        parameter->startMonteCarlo();
        // Note that commitChanges() is called in onMonteCarloFinished()
    }
}


void UncertaintyCalculation::stopMonteCarlo() {
    OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getLocked() ) {
        parameter->stopMonteCarlo();
        // Note that commitChanges() is called in onMonteCarloFinished()
    }
}


void UncertaintyCalculation::updateCorrelation(
    const Correlation *correlation
) {
    QUuid id { Correlation::getSelectedId() };
    Correlation *originalCorrelation { Correlation::getById( id ) };
    if ( correlation && originalCorrelation ) {
        Correlation newCorrelation { *correlation };
        newCorrelation.setId( id );   // Keep the existing id in the model

        bool recompile { true };
        if ( *originalCorrelation == newCorrelation ) {
            // Existing and new Correlation are equal numerically, no need to
            // recompile.
            recompile = false;
        }

        // Store the needed data before updating
        QUuid originalParamAId { originalCorrelation->getInputParameterAId() };
        QUuid originalParamBId { originalCorrelation->getInputParameterBId() };
        QString originalNameA { originalCorrelation->getInputParameterNameA() };
        QString originalNameB { originalCorrelation->getInputParameterNameB() };

        DiffUtil diffUtil {};
        diffUtil.takeSnapshot( originalCorrelation );

        Correlation::update( id, &newCorrelation );

        if ( recompile ) {
            // Recompile to check the original InputParameters
            recompileExpressions(
                diffUtil,
                false,
                originalParamAId,
                originalParamBId
            );
            // Recompile to check the new InputParameters
            recompileExpressions(
                diffUtil,
                false,
                newCorrelation.getInputParameterAId(),
                newCorrelation.getInputParameterBId()
            );
        }

        diffUtil.commitChanges(
            "update correlation between " +
            originalNameA +
            " & " +
            originalNameB
        );

        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::updateInputParameter(
    const InputParameter *parameter
) {
    QUuid id { InputParameter::getSelectedId() };
    InputParameter *originalParameter { InputParameter::getById( id ) };
    if ( parameter && originalParameter ) {
        InputParameter newParameter { *parameter };
        newParameter.setId( id );   // Keep the existing id in the model

        bool recompile { true };
        if ( *originalParameter == newParameter ) {
            // Existing and new InputParameter are equal numerically, no need to
            // recompile the OutputParameters.
            recompile = false;
        }

        // Store the needed data before updating
        QString originalName { originalParameter->getName() };

        DiffUtil diffUtil {};
        diffUtil.takeSnapshot( originalParameter );

        InputParameter::update( id, &newParameter);

        if ( recompile ) {
            // All invalid expressions are recompiled plus any that reference
            // the updated InputParameter
            recompileExpressions( diffUtil, true, id );
        }

        diffUtil.commitChanges( "update input parameter " + originalName );

        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::updateOutputParameter(
    const OutputParameter *parameter
) {
    QUuid id { OutputParameter::getSelectedId() };
    OutputParameter *originalParameter { OutputParameter::getById( id ) };
    if ( parameter && originalParameter ) {
        OutputParameter newParameter { *parameter };
        newParameter.setId( id );   // Keep the existing id in the model

        if ( *originalParameter != newParameter ) {
            // Existing and new OutputParameter are not equal numerically,
            // use the new one and recompile
            newParameter.compile();
        }
        else {
            // Results are identical, use originalParameter with new name and
            // new unit to keep any available Monte Carlo results
            newParameter = *originalParameter;
            newParameter.setName( parameter->getName() );
            newParameter.setUnit( parameter->getUnit() );
        }

        // Store the needed data before updating
        QString originalName { originalParameter->getName() };

        DiffUtil diffUtil {};
        diffUtil.takeSnapshot( originalParameter );

        OutputParameter::update( id, &newParameter);
        emitAllResultsChanged();

        diffUtil.commitChanges( "update output parameter " + originalName );

        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::userClearProject() {
    // The project is cleared by the user and can be undone

    // Take snapshot and remove all objects. Note that the objects are removed
    // individually in this case to maintain the row order on undo.
    DiffUtil diffUtil {};

    for ( const InputParameter *inputParameter : InputParameter::getAll() ) {
        diffUtil.takeSnapshot( inputParameter );
        InputParameter::remove( inputParameter->getId() );
    }

    for ( const OutputParameter *outputParameter : OutputParameter::getAll() ) {
        diffUtil.takeSnapshot( outputParameter );
        OutputParameter::remove( outputParameter->getId() );
    }

    for ( const Correlation *correlation : Correlation::getAll() ) {
        diffUtil.takeSnapshot( correlation );
        Correlation::remove( correlation->getId() );
    }

    diffUtil.commitChanges( "clear project" );

    setUnsavedChanges( true );
}


void UncertaintyCalculation::undo() {
    if ( !getOutputLocked() ) {
        UndoStack::instance().undo();
    }
}


void UncertaintyCalculation::redo() {
    if ( !getOutputLocked() ) {
        UndoStack::instance().redo();
    }
}


void UncertaintyCalculation::lockItemSelectionModels() {
    bool locked { getOutputLocked() };
    InputParameter::setSelectionLocked( locked );
    OutputParameter::setSelectionLocked( locked );
    Correlation::setSelectionLocked( locked );
    emit outputLockedChanged();
}


void UncertaintyCalculation::onMonteCarloFinished() {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    QString name { parameter ? " " + parameter->getName() : "" };
    mDiffUtil.commitChanges( "run monte carlo" + name );
}


void UncertaintyCalculation::onTransactionApplied() {
    // Connect to restored OutputParameters
    const QList<OutputParameter *> &parameters { OutputParameter::getAll() };
    for ( OutputParameter *parameter : parameters ) {
        connectToOutputParameter( parameter );
    }

    // Re-establish live pointers in Correlations from stored Ids
    Correlation::reconnectAllCorrelations();

    // Recompile all OutputParameters without resetting Monte Carlo results
    OutputParameter::recompileAllExpressions( false );

    // Notify the UI that the data of this object has changed
    emitAllResultsChanged();
    emit inputParameterChanged();

    setUnsavedChanges( true );
}


void UncertaintyCalculation::unsavedChanges() {
    setUnsavedChanges( true );
}


QJsonObject UncertaintyCalculation::projectToJson() const {
    QJsonObject json {};
    json[ sVersionString ] = QCoreApplication::applicationVersion();
    json[ sInputParametersString ] = InputParameter::parametersToJson();
    json[ sCorrelationsString ] = Correlation::correlationsToJson();
    json[ sOutputParametersString ] = OutputParameter::parametersToJson();
    return json;
}


QList<double> UncertaintyCalculation::getHistogramValues() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramValues();
    }
    return QList<double> {};
}


QString UncertaintyCalculation::getInputName() const
{
    const InputParameter *parameter { InputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getName();
    }
    return "";
}


QString UncertaintyCalculation::getOutputName() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getName();
    }
    return "";
}


QString UncertaintyCalculation::getOutputUnit() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getUnit();
    }
    return "";
}


QString UncertaintyCalculation::getProjectFileName() const {
    return mProjectFilePath.fileName();
}


QString UncertaintyCalculation::outputParameterReferencesToString(
    const QStringList &references
) const {
    // Convert a QStringList of OutputParameter names to a string
    QString string {};
    if ( references.size() > 0 ) {
        string = "output parameter";
    }
    if ( references.size() > 1 ) {
        string += "s";
    }
    for ( int i { 0 }; i < references.size(); ++ i ) {
        string += " " + references[ i ];
        if ( i < references.size() - 2 ) {
            string += ",";
        }
        else if ( i == references.size() - 2 ) {
            string += " and";
        }
    }
    return string;
}


QString UncertaintyCalculation::projectToCSVString() const {
    QString result {};
    // Add version number
    result += "Sigma version: " +
              QCoreApplication::applicationVersion() +
              StringUtils::endl;

    // Add project title
    QString project { getProjectFileName() };
    if ( project.size() > 0 ) {
        result += StringUtils::addQuotes( "Project: " + project ) +
                  StringUtils::endl;
    }

    // Add input, correlation and output parameter data
    result += StringUtils::endl;
    result += InputParameter::parametersToCSVString() + StringUtils::endl;
    if ( Correlation::getAll().size() > 0 ) {
        result += Correlation::correlationsToCSVString() + StringUtils::endl;
    }
    result += OutputParameter::parametersToCSVString();

    return result;
}


QStringList UncertaintyCalculation::getDistributionStrings() const {
    QStringList distributionList {};
    QList<Distribution::Type> distributions {
        Distribution::getDistributions()
    };
    for ( Distribution::Type &distribution : distributions ) {
        distributionList.append(
            Distribution::distributionToString( distribution )
        );
    }
    return distributionList;
}


QStringList UncertaintyCalculation::getInputParameterReferences(
    const InputParameter *inputParameter
) const {
    // Return a list of OutputParameter names that reference this
    // InputParameter
    QStringList references {};
    if ( inputParameter ) {
        for ( const OutputParameter *outputParam : OutputParameter::getAll() ) {
            if (
                outputParam &&
                outputParam->isInputParameterReferenced(
                    inputParameter->getId()
                )
            ) {
                references.append( outputParam->getName() );
            }
        }
    }
    return references;
}


QStringList UncertaintyCalculation::getMonteCarloResults() const {
    QStringList results {};
    results.fill( "", MonteCarlo::headerLabels.size() );
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getMonteCarloResults();
    }
    return results;
}


QUrl UncertaintyCalculation::getProjectFilePath() const {
    return mProjectFilePath;
}


bool UncertaintyCalculation::canRedo() const {
    return UndoStack::instance().canRedo();
}


bool UncertaintyCalculation::canUndo() const {
    return UndoStack::instance().canUndo();
}


bool UncertaintyCalculation::getHistogramValid() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() && !parameter->getLocked() ) {
        return parameter->getMonteCarloValid();
    }
    return false;
}


bool UncertaintyCalculation::getOutputLocked() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getLocked();
    }
    return false;
}


bool UncertaintyCalculation::getOutputValid() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter ) {
        return parameter->getValid();
    }
    return false;
}


bool UncertaintyCalculation::getUnsavedChanges() const {
    return mUnsavedChanges;
}


double UncertaintyCalculation::getHistogramXMax() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramXMax();
    }
    return 1.;
}


double UncertaintyCalculation::getHistogramXMin() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramXMin();
    }
    return -1.;
}


double UncertaintyCalculation::getHistogramYMax() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramYMax();
    }
    return 1.;
}


double UncertaintyCalculation::getMonteCarloConvergenceFactor() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getMonteCarloConvergenceFactor();
    }
    return 0.;
}


int UncertaintyCalculation::getHistogramHigherIndex() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramHigherIndex();
    }
    return -1;
}


int UncertaintyCalculation::getHistogramLowerIndex() const {
    const OutputParameter *parameter { OutputParameter::getSelected() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramLowerIndex();
    }
    return -1;
}


void UncertaintyCalculation::clearProject( bool unsavedChanges ) {
    OutputParameter::clearModel();
    Correlation::clearModel();
    InputParameter::clearModel();
    emit inputParameterChanged();
    setUnsavedChanges( unsavedChanges );
}


void UncertaintyCalculation::connectToOutputParameter(
    const OutputParameter *parameter
){
    // Connect the signals of the OutputParameter to this object's signals and
    // slots
    if ( parameter ) {
        connect(
            parameter,
            &OutputParameter::monteCarloStarted,
            this,
            &UncertaintyCalculation::histogramValuesChanged,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloFinished,
            this,
            &UncertaintyCalculation::histogramValuesChanged,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloStatusChanged,
            this,
            &UncertaintyCalculation::monteCarloResultsListChanged,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloConvergenceFactorChanged,
            this,
            &UncertaintyCalculation::monteCarloConvergenceFactorChanged,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::lockedChanged,
            this,
            &UncertaintyCalculation::lockItemSelectionModels,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloStarted,
            this,
            &UncertaintyCalculation::unsavedChanges,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloFinished,
            this,
            &UncertaintyCalculation::unsavedChanges,
            Qt::UniqueConnection
        );
        connect(
            parameter,
            &OutputParameter::monteCarloFinished,
            this,
            &UncertaintyCalculation::onMonteCarloFinished,
            Qt::UniqueConnection
        );
    }
}


void UncertaintyCalculation::createConnections() {
    // Create connections to the input and output model
    connect(
        InputParameter::getInputModel()->selectionModel(),
        &QItemSelectionModel::currentRowChanged,
        this,
        &UncertaintyCalculation::inputParameterChanged
    );
    connect(
        OutputParameter::getOutputModel()->selectionModel(),
        &QItemSelectionModel::currentRowChanged,
        this,
        &UncertaintyCalculation::emitAllResultsChanged
    );

    // Create connections to the undostack
    connect(
        &UndoStack::instance(),
        &UndoStack::transactionApplied,
        this,
        &UncertaintyCalculation::onTransactionApplied
    );
    connect(
        &UndoStack::instance(),
        &UndoStack::canUndoChanged,
        this,
        &UncertaintyCalculation::canUndoChanged
    );
    connect(
        &UndoStack::instance(),
        &UndoStack::canRedoChanged,
        this,
        &UncertaintyCalculation::canRedoChanged
    );

    // Create connection to the application settings
    connect(
        mAppSettings,
        &ApplicationSettings::displayPrecisionChanged,
        this,
        &UncertaintyCalculation::resetDisplay
    );
}


void UncertaintyCalculation::emitAllResultsChanged() {
    // Notify that properties are updated
    emit resultsChanged();              // outputName, outputUnit, outputValid
    emit monteCarloResultsListChanged();        // monteCarloResultsList
    emit histogramValuesChanged();              // histogram related properties
    emit monteCarloConvergenceFactorChanged();  // monteCarloConvergenceFactor
}


void UncertaintyCalculation::projectFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ sInputParametersString ]; v.isArray() ) {
        const QJsonArray paramArray { v.toArray() };
        InputParameter::parametersFromJson( paramArray, this );
    }

    if ( const QJsonValue v = json[ sCorrelationsString ]; v.isArray() ) {
        const QJsonArray correlationsArray { v.toArray() };
        Correlation::correlationsFromJson( correlationsArray, this );
    }

    if ( const QJsonValue v = json[ sOutputParametersString ]; v.isArray() ) {
        const QJsonArray paramArray { v.toArray() };
        OutputParameter::parametersFromJson( paramArray, this );

        // Create connections between the new OutputParameters and this object's
        // signals and slots
        for ( const OutputParameter *param : OutputParameter::getAll() ) {
            connectToOutputParameter( param );
        }
    }

    // Connect the new Correlation objects to their InputParameters
    Correlation::reconnectAllCorrelations();

    updateUnits();

    emitAllResultsChanged();
}


void UncertaintyCalculation::recompileExpressions(
    DiffUtil &diffUtil,
    const bool recompileInvalidExpressions,
    const QUuid inputParamAId,
    const QUuid inputParamBId
) {
    // Recompile invalid expressions, when recompileInvalidExpressions = true,
    // or expressions that reference inputParamAId (when inputParamBId.isNull()
    // ) or both inputParamAId and inputParamBId. So valid parameters that do
    // not reference the InputParameter(s) are left untouched.
    // The passed DiffUtil will be used to take snapshots of the before state of
    // recompiled parameters.
    for ( OutputParameter *outputParameter : OutputParameter::getAll() ) {
        if ( outputParameter && (
            ( recompileInvalidExpressions && !outputParameter->getValid() ) ||
                (
                    outputParameter->isInputParameterReferenced( inputParamAId ) &&
                    inputParamBId.isNull()
                ) ||
                (
                    outputParameter->isInputParameterReferenced( inputParamAId ) &&
                    outputParameter->isInputParameterReferenced( inputParamBId )
                )
            )
        ) {
            diffUtil.takeSnapshot( outputParameter );

            OutputParameter updatedOutputParameter { *outputParameter };
            updatedOutputParameter.compile();

            OutputParameter::update(
                outputParameter->getId(),
                &updatedOutputParameter
            );

            emitAllResultsChanged();
        }
    }
}


void UncertaintyCalculation::registerDiffUtilDataTypeMethods() {
    // Register the methods needed for applying transactions for each Data type
    // with DiffUtil
    DiffUtil::registerApplyDiff(
        DataType::InputParameter,
        &InputParameter::applyDiff
    );
    DiffUtil::registerApplyDiff(
        DataType::OutputParameter,
        &OutputParameter::applyDiff
    );
    DiffUtil::registerApplyDiff(
        DataType::Correlation,
        &Correlation::applyDiff
    );

    DiffUtil::registerCurrentJson(
        DataType::InputParameter,
        &InputParameter::currentJson
    );
    DiffUtil::registerCurrentJson(
        DataType::OutputParameter,
        &OutputParameter::currentJson
        );
    DiffUtil::registerCurrentJson(
        DataType::Correlation,
        &Correlation::currentJson
    );

    DiffUtil::registerGetRowIndex(
        DataType::InputParameter,
        &InputParameter::getRowIndex
    );
    DiffUtil::registerGetRowIndex(
        DataType::OutputParameter,
        &OutputParameter::getRowIndex
    );
    DiffUtil::registerGetRowIndex(
        DataType::Correlation,
        &Correlation::getRowIndex
    );
}


void UncertaintyCalculation::resetDisplay() {
    InputParameter::onDisplayPrecisionChanged();
    OutputParameter::onDisplayPrecisionChanged();
    Correlation::onDisplayPrecisionChanged();
    emitAllResultsChanged();
}


void UncertaintyCalculation::resetProjectFilePath() {
    mProjectFilePath.clear();
    emit projectFilePathChanged();
    Settings::setLastProjectFilePath( mProjectFilePath );
}


void UncertaintyCalculation::setProjectFilePath( const QUrl &projectFilePath ) {
    mProjectFilePath = projectFilePath;
    emit projectFilePathChanged();
    Settings::setLastProjectFilePath( mProjectFilePath );
}


void UncertaintyCalculation::setUnsavedChanges( bool unsavedChanges ) {
    if ( unsavedChanges && Settings::getAutoSaveProject() ) {
        // Auto Save the changes.
        saveProject();
        return;
    }
    mUnsavedChanges = unsavedChanges;
    emit unsavedChangesChanged();
}


void UncertaintyCalculation::updateUnits() {
    // Add units of the parameters to the list of units.
    for ( const InputParameter *parameter : InputParameter::getAll() ) {
        addUnit( parameter->getUnit() );
    }
    for ( const OutputParameter *parameter : OutputParameter::getAll() ) {
        addUnit( parameter->getUnit() );
    }
}
