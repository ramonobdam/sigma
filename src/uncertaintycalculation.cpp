// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "montecarlo.h"
#include "uncertaintycalculation.h"
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QIODeviceBase>
#include <QJsonDocument>
#include <QJsonValue>
#include <QList>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>


UncertaintyCalculation::UncertaintyCalculation( QObject *parent )
    :   QObject { parent },
        mCorrelationModel { Correlation::getCorrelationModel() },
        mInputParametersModel { InputParameter::getInputModel() },
        mOutputParametersModel { OutputParameter::getOutputModel() },
        mUnitsModel { QStringListModel( this ) },
        mDistributionModel { QStringListModel( this ) },
        mBudgetModel { BudgetModel( this ) },
        mResultsModel { ResultsModel ( this) },
        mUnits { mDefaultUnits },
        mProjectFilePath {},
        mUnsavedChanges {},
        mOutputRow { -1 }
{
    mUnitsModel.setStringList( mUnits );
    mUnitsModel.sort( 0 );

    mDistributionModel.setStringList( getDistributionStrings() );

    InputParameter::addConstantsToSymbolTable();
    OutputParameter::setCollectVariables( true );

    mBudgetModel.setOutputParameterModel( mOutputParametersModel );
    mResultsModel.setOutputParameterModel( mOutputParametersModel );

    // Create connections to the input and output model.
    connect(
        mOutputParametersModel->selectionModel(),
        &QItemSelectionModel::currentRowChanged,
        this,
        &UncertaintyCalculation::setOutputRow
    );
    connect(
        mInputParametersModel->selectionModel(),
        &QItemSelectionModel::currentRowChanged,
        this,
        &UncertaintyCalculation::inputParameterChanged
    );
}


UncertaintyCalculation::~UncertaintyCalculation(){}


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


QStringList UncertaintyCalculation::getUnits() const {
    return mUnits;
}


InputParameter * const UncertaintyCalculation::getSelectedInputParameter(
) const {
    if ( mInputParametersModel ) {
        return mInputParametersModel->getSelectedRow();
    }
    return nullptr;
}


OutputParameter * const UncertaintyCalculation::getSelectedOutputParameter(
) const {
    if ( mOutputParametersModel ) {
        return mOutputParametersModel->getSelectedRow();
    }
    return nullptr;
}


const BudgetModel * UncertaintyCalculation::budgetItemModel() const {
    return &mBudgetModel;
}


Correlation * const UncertaintyCalculation::getSelectedCorrelation() const {
    if ( mCorrelationModel ) {
        return mCorrelationModel->getSelectedRow();
    }
    return nullptr;
}


const QItemSelectionModel * UncertaintyCalculation::correlationSelectionModel(
) const {
    return mCorrelationModel->selectionModel();
}


const QItemSelectionModel * UncertaintyCalculation::inputSelectionModel(
) const {
    return mInputParametersModel->selectionModel();
}


const QItemSelectionModel * UncertaintyCalculation::outputSelectionModel(
) const {
    return mOutputParametersModel->selectionModel();
}


const QList<int> UncertaintyCalculation::getBudgetColumnWidths() const {
    return UncertaintyComponent::columnWidths;
}


const QList<int> UncertaintyCalculation::getCorrelationColumnWidths() const {
    return Correlation::columnWidths;
}


const QList<int> UncertaintyCalculation::getInputColumnWidths() const {
    return InputParameter::columnWidths;
}


const QList<int> UncertaintyCalculation::getOutputColumnWidths() const {
    return OutputParameter::columnWidths;
}


const QList<int> UncertaintyCalculation::getResultsColumnWidths() const {
    return OutputParameter::resultColumnWidths;
}


const QObject * UncertaintyCalculation::correlationItemModel() const {
    return mCorrelationModel->itemModel();
}


const QObject * UncertaintyCalculation::inputItemModel() const {
    return mInputParametersModel->itemModel();
}


const QObject * UncertaintyCalculation::outputItemModel() const {
    return mOutputParametersModel->itemModel();
}


const QString UncertaintyCalculation::getMonteCarloHeader(
    const int &column
) const {
    if ( column >= 0 && column < MonteCarlo::headerLabels.size() ) {
        return MonteCarlo::headerLabels[ column ];
    }
    return "";
}


const QString UncertaintyCalculation::getNewInputParameterName() const {
    QString newName {};
    int num { 0 };
    do {
        num++;
        newName = InputParameter::defaultName + QString::number( num );
    } while ( !validInputName( newName ) && num < maxNameAttempts );
    return newName;
}


const QString UncertaintyCalculation::getNewOutputParameterName() const {
    QString newName {};
    int num { 0 };
    do {
        num++;
        newName = OutputParameter::defaultName + QString::number( num );
    } while ( !validOutputName( newName ) && num < maxNameAttempts );
    return newName;
}


const QString UncertaintyCalculation::getSelectedCorrelationReferences() const {
    // Return a string that lists the OutputParameters that are referenced by
    // both InputParameters of the selected correlation
    QString string {};
    Correlation * const correlation { getSelectedCorrelation() };
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


const QString UncertaintyCalculation::getSelectedInputParameterReferences(
) const {
    // Return a string that lists the OutputParameters that are referenced by
    // the selected InputParameter
    InputParameter * const inputParameter { getSelectedInputParameter() };
    QStringList references { getInputParameterReferences( inputParameter ) };
    return outputParameterReferencesToString( references );
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

bool UncertaintyCalculation::inputParameterIsConstant(
    const QString &name
) const {
    return InputParameter::inputParameterIsConstant( name );
}


bool UncertaintyCalculation::loadProject( const QUrl &url ) {
    QString path { url.toLocalFile() };
    QFile loadFile( path );
    if ( !url.isLocalFile() || !loadFile.open( QIODevice::ReadOnly ) ) {
        qDebug() << QString( mLoadFailedString ).arg( path );
        resetProjectFilePath();
        return false;
    }
    QByteArray projectData = loadFile.readAll();
    QJsonDocument projectDoc( QJsonDocument::fromJson( projectData ) );
    clearProject( false );  // Currently open project should not be saved
    parametersFromJson( projectDoc.object() );
    setProjectFilePath( url );
    setUnsavedChanges( false );
    return true;
}


bool UncertaintyCalculation::saveCSV( const QUrl &url ) {
    QString path { url.toLocalFile() };
    QSaveFile csvFile( path );
    if (
        !url.isLocalFile() ||
        !csvFile.open( QIODeviceBase::WriteOnly | QIODeviceBase::Text )
    ) {
        qDebug() << QString( mCSVFailedString ).arg( path );
        return false;
    }
    QTextStream out( &csvFile );
    out << projectToString();
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
        qDebug() << QString( mSaveFailedString ).arg( path );
        resetProjectFilePath();
        return false;
    }
    QJsonObject project { parametersToJson() };
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


void UncertaintyCalculation::addCorrelation( Correlation *correlation ) {
    if ( correlation && correlation->addToModel() ) {
        // Recompile expressions that use both correlated InputParameters
        recompileExpressions(
            false,
            correlation->getInputParameterA(),
            correlation->getInputParameterB()
        );
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::addInputParameter( InputParameter *parameter ) {
    if ( parameter && parameter->addToModel() ) {
        recompileExpressions( true );
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::addOutputParameter( OutputParameter* parameter ) {
    if ( parameter && parameter->addToModel() ) {
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::addUnit( const QString &name ) {
    if ( !mUnits.contains( name ) ) {
        mUnits.append( name );
        mUnitsModel.setStringList( mUnits );
        mUnitsModel.sort( 0 );
    }
}


void UncertaintyCalculation::clearProject( const bool &unsavedChanges ) {
    emitOutputModelsAboutToBeReset();
    mOutputParametersModel->clear();
    mCorrelationModel->clear();
    mInputParametersModel->clear();
    emit inputParameterChanged();
    InputParameter::clearSymbolTable();
    InputParameter::addConstantsToSymbolTable();
    emitOutputModelsReset();
    setOutputRow();
    setUnsavedChanges( unsavedChanges );
}


void UncertaintyCalculation::newProject() {
    clearProject();
    resetProjectFilePath();
}


void UncertaintyCalculation::removeCorrelation() {
    Correlation * const correlation { getSelectedCorrelation() };
    if ( correlation ) {
        // Store referenced InputParameters and correlation coefficient
        InputParameter *paramA { correlation->getInputParameterA() };
        InputParameter *paramB { correlation->getInputParameterB() };
        double correlationCoeff { correlation->getCorrelation() };
        if ( Correlation::removeSelectedModelRow() ) {
            // Recompile the expressions that referenced the InputParameters of
            // the deleted correlation when their correlation was not zero
            if ( correlationCoeff != 0 ) {
                recompileExpressions( false, paramA, paramB );
            }
            setUnsavedChanges( true );
        }
    }
}


void UncertaintyCalculation::removeInputParameter() {
    InputParameter *deletedParameter {
        InputParameter::removeSelectedModelRow()
    };
    if ( deletedParameter ) {
        recompileExpressions( false, deletedParameter );
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::removeOutputParameter() {
    if ( mOutputParametersModel->removeSelectedRow() ) {
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::resetDisplay() {
    mInputParametersModel->emitAllDataChanged();
    mOutputParametersModel->emitAllDataChanged();
    mBudgetModel.emitAllDataChanged();
    mResultsModel.emitAllDataChanged();
    emitAllResultsChanged();
}


void UncertaintyCalculation::runMonteCarlo() {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        parameter->startMonteCarlo();
    }
}


void UncertaintyCalculation::stopMonteCarlo() {
    OutputParameter *parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        parameter->stopMonteCarlo();
    }
}


void UncertaintyCalculation::updateCorrelation( Correlation *correlation ) {
    if ( correlation ) {
        bool recompile { true };
        Correlation * const selectedCorrelation { getSelectedCorrelation() };
        // Store the InputParameters that are currently referenced
        InputParameter *originalParamA { correlation->getInputParameterA() };
        InputParameter *originalParamB { correlation->getInputParameterB() };
        if ( selectedCorrelation && *selectedCorrelation == *correlation ) {
            // Existing and new Correlation are equal numerically, no need to
            // recompile.
            recompile = false;
        }
        emitOutputModelsAboutToBeReset();
        Correlation *updatedCorrelation {
            correlation->updateSelectedModelRow()
        };
        emitOutputModelsReset();
        emit resultsChanged();
        if ( updatedCorrelation && recompile ) {
            // Recompile to check the original InputParameters
            recompileExpressions( false, originalParamA, originalParamB );
            // Recompile to check the new InputParameters
            recompileExpressions(
                false,
                updatedCorrelation->getInputParameterA(),
                updatedCorrelation->getInputParameterB()
            );
        }
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::updateInputParameter( InputParameter *parameter) {
    if ( parameter ) {
        bool recompile { true };
        const InputParameter *selectedInputParameter {
            getSelectedInputParameter()
        };
        if ( selectedInputParameter && *selectedInputParameter == *parameter ) {
            // Existing and new InputParameter are equal numerically, no need to
            // recompile.
            recompile = false;
        }
        emitOutputModelsAboutToBeReset();
        InputParameter *updatedParameter {
            parameter->updateSelectedModelRow()
        };
        emitOutputModelsReset();
        emit resultsChanged();
        if ( updatedParameter && recompile ) {
            recompileExpressions( true, updatedParameter );
        }
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::updateOutputParameter(
    OutputParameter *parameter
) {
    if ( parameter ) {
        emitOutputModelsAboutToBeReset();
        parameter->updateSelectedModelRow();
        emitOutputModelsReset();
        emitAllResultsChanged();
        setUnsavedChanges( true );
    }
}


void UncertaintyCalculation::lockItemSelectionModels() {
    bool locked { getOutputLocked() };
    mInputParametersModel->setSelectionLocked( locked );
    mOutputParametersModel->setSelectionLocked( locked );
    emit outputLockedChanged();
}


void UncertaintyCalculation::setOutputRow() {
    const int row { mOutputParametersModel->selectedRow() };
    if ( row != mOutputRow ) {
        mOutputRow = row;
        mBudgetModel.setOutputRow( row );
        mResultsModel.setOutputRow( row );
        emitAllResultsChanged();
    }
}


void UncertaintyCalculation::unsavedChanges() {
    setUnsavedChanges( true );
}


QJsonObject UncertaintyCalculation::parametersToJson() const {
    QJsonObject json {};
    json[ mInputParametersString ] = InputParameter::parametersToJson();
    json[ mCorrelationsString ] = Correlation::correlationsToJson();
    json[ mOutputParametersString ] = OutputParameter::parametersToJson();
    return json;
}


QList<double> UncertaintyCalculation::getHistogramValues() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramValues();
    }
    return QList<double> {};
}


QString UncertaintyCalculation::getInputName() const
{
    const InputParameter *parameter { getSelectedInputParameter() };
    if ( parameter ) {
        return parameter->getName();
    }
    return "";
}


QString UncertaintyCalculation::getOutputName() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        return parameter->getName();
    }
    return "";
}


QString UncertaintyCalculation::getOutputUnit() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
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


QString UncertaintyCalculation::projectToString() const {
    QString result {};
    // Add project title
    QString project { getProjectFileName() };
    if ( project.size() > 0 ) {
        result += addQuotes( "Project: " + project ) + endl;
    }

    // Add input, correlation and output parameter data
    result += endl;
    result += InputParameter::parametersToString() + endl;
    if ( mCorrelationModel->rowCount() > 0 ) {
        result += Correlation::correlationsToString() + endl;
    }
    result += OutputParameter::parametersToString();

    return result;
}

QStringList UncertaintyCalculation::getInputParameterReferences(
    InputParameter * const &inputParameter
) const {
    // Return a list of OutputParameter names that references this
    // InputParameter
    QStringList references {};
    if ( inputParameter ) {
        int rows { mOutputParametersModel->rowCount() };
        for ( int row { 0 }; row < rows; ++row ) {
            OutputParameter *outputParameter {
                mOutputParametersModel->getRow( row )
            };
            if (
                outputParameter &&
                outputParameter->isInputParameterReferenced( inputParameter )
            ) {
                references.append( outputParameter->getName() );
            }
        }
    }
    return references;
}


QStringList UncertaintyCalculation::getMonteCarloResults() const {
    QStringList results {};
    results.fill( "", MonteCarlo::headerLabels.size() );
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        return parameter->getMonteCarloResults();
    }
    return results;
}


QUrl UncertaintyCalculation::getProjectFilePath() const {
    return mProjectFilePath;
}


bool UncertaintyCalculation::getHistogramValid() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() && !parameter->getLocked() ) {
        return parameter->getMonteCarloValid();
    }
    return false;
}


bool UncertaintyCalculation::getOutputLocked() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        return parameter->getLocked();
    }
    return false;
}


bool UncertaintyCalculation::getOutputValid() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter ) {
        return parameter->getValid();
    }
    return false;
}


bool UncertaintyCalculation::getUnsavedChanges() const {
    return mUnsavedChanges;
}


double UncertaintyCalculation::getHistogramXMax() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramXMax();
    }
    return 1.;
}


double UncertaintyCalculation::getHistogramXMin() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramXMin();
    }
    return -1.;
}


double UncertaintyCalculation::getHistogramYMax() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramYMax();
    }
    return 1.;
}


double UncertaintyCalculation::getMonteCarloConvergenceFactor() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getMonteCarloConvergenceFactor();
    }
    return 0.;
}


int UncertaintyCalculation::getHistogramHigherIndex() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramHigherIndex();
    }
    return -1;
}


int UncertaintyCalculation::getHistogramLowerIndex() const {
    OutputParameter * const parameter { getSelectedOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getHistogramLowerIndex();
    }
    return -1;
}


void UncertaintyCalculation::emitAllResultsChanged() {
    emit resultsChanged();
    emit monteCarloResultsListChanged();
    emit monteCarloValuesChanged();
    emit monteCarloConvergenceFactorChanged();
}


void UncertaintyCalculation::emitOutputModelsAboutToBeReset() {
    mBudgetModel.emitModelAboutToBeReset();
    mResultsModel.emitModelAboutToBeReset();
}


void UncertaintyCalculation::emitOutputModelsReset() {
    mBudgetModel.emitModelReset();
    mResultsModel.emitModelReset();
}


void UncertaintyCalculation::parametersFromJson( const QJsonObject &json ) {
    if ( const QJsonValue v = json[ mInputParametersString ]; v.isArray() ) {
        const QJsonArray paramArray { v.toArray() };
        InputParameter::parametersFromJson( paramArray, this );
    }

    if ( const QJsonValue v = json[ mCorrelationsString ]; v.isArray() ) {
        const QJsonArray correlationsArray { v.toArray() };
        Correlation::correlationsFromJson( correlationsArray, this );
    }

    if ( const QJsonValue v = json[ mOutputParametersString ]; v.isArray() ) {
        const QJsonArray paramArray { v.toArray() };
        OutputParameter::parametersFromJson( paramArray, this );
    }

    updateUnits();

    emitAllResultsChanged();
}


void UncertaintyCalculation::recompileExpressions(
    const bool recompileInvalidExpressions,
    InputParameter *inputParameterA,
    InputParameter *inputParameterB
) {
    // Recompile invalid expressions, when recompileInvalidExpressions = true,
    // or expressions that reference inputParameterA (when inputParameterB is
    // null_ptr) or both inputParameterA and inputParameterB. So valid
    // parameters that do not reference the inputParameter(s) are left
    // untouched.
    int rows { mOutputParametersModel->rowCount() };
    for ( int row { 0 }; row < rows; ++row ) {
        OutputParameter *outputParameter {
            mOutputParametersModel->getRow( row )
        };
        if ( outputParameter && (
            ( recompileInvalidExpressions && !outputParameter->getValid() ) ||
                (
                    outputParameter->isInputParameterReferenced( inputParameterA ) &&
                    !inputParameterB
                ) ||
                (
                    outputParameter->isInputParameterReferenced( inputParameterA ) &&
                    outputParameter->isInputParameterReferenced( inputParameterB )
                )
            )
        ) {
            emitOutputModelsAboutToBeReset();
            outputParameter->compile();
            mOutputParametersModel->emitRowChanged( row );
            emitOutputModelsReset();
            emitAllResultsChanged();
        }
    }
}


void UncertaintyCalculation::resetProjectFilePath() {
    mProjectFilePath.clear();
    emit projectFilePathChanged();
    setLastProjectFilePath( mProjectFilePath );
}


void UncertaintyCalculation::setProjectFilePath( const QUrl &projectFilePath ) {
    mProjectFilePath = projectFilePath;
    emit projectFilePathChanged();
    setLastProjectFilePath( mProjectFilePath );
}


void UncertaintyCalculation::setUnsavedChanges( const bool &unsavedChanges ) {
    if ( unsavedChanges && getAutoSaveProject() ) {
        // Auto Save the changes.
        saveProject();
        return;
    }
    mUnsavedChanges = unsavedChanges;
    emit unsavedChangesChanged();
}


void UncertaintyCalculation::updateUnits() {
    // Add units of the parameters to the list of units.
    for ( InputParameter * &parameter : mInputParametersModel->getAllRows() ) {
        addUnit( parameter->getUnit() );
    }
    for ( OutputParameter* &parameter : mOutputParametersModel->getAllRows() ) {
        addUnit( parameter->getUnit() );
    }
}
