// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "outputparameter.h"
#include "settings.h"
#include "uncertaintycalculation.h"
#include "undostack.h"
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QtNumeric>
#include <cmath>

// Integration-style unit tests for UncertaintyCalculation: the Orchestration
// layer class that ties the Core layer (InputParameter, OutputParameter,
// Correlation) together with project save/restore ('.sig' JSON files), CSV
// export and the undo/redo workflow.
class tst_uncertaintycalculation : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void addInputAndOutputParameter_compilesAndIsSelected();
    void addInputParameter_undoRedoRestoresModel();

    void addCorrelation_cascadesToOutputsAndIsUndoable();
    void addInputParameter_cascadesToOutputsAndIsUndoable();
    void removeCorrelation_cascadesToOutputsAndIsUndoable();
    void removeInputParameter_cascadesToCorrelationsAndOutputsAndIsUndoable();
    void updateCorrelation_cascadesToOutputsAndIsUndoable();
    void updateInputParameter_cascadesToOutputsAndIsUndoable();

    void projectJson_roundTripInMemory();
    void saveAndLoadProject_roundTripThroughFile();
    void loadProject_invalidUrl_fails();

    void saveCSV_writesExpectedSections();

    void runMonteCarlo_updatesSelectedOutputAndIsUndoable();

    void newProject_clearsEverything();

    void projectFromJson_addsNewUnit();
};


void tst_uncertaintycalculation::initTestCase() {
    // Give QSettings (used by the default ApplicationSettings passed to
    // UncertaintyCalculation) a dedicated organization/application name so it
    // does not warn about missing ones. None of these tests call
    // ApplicationSettings::load()/save(), so no real user configuration is
    // ever touched.
    QCoreApplication::setOrganizationName( "SigmaTests" );
    QCoreApplication::setApplicationName( "SigmaTests" );

    OutputParameter::setCollectVariables( true );

    // Keep Monte Carlo simulations fast in tests
    QVERIFY( Settings::setMonteCarloBatchSize( 3000 ) );
    QVERIFY( Settings::setMonteCarloDigits( 1 ) );
    QVERIFY( Settings::setMonteCarloMaxNumOfBatches( 300 ) );
}


void tst_uncertaintycalculation::cleanup() {
    UndoStack::instance().clear();
    OutputParameter::clearModel();
    Correlation::clearModel();
    InputParameter::clearModel();
}


void tst_uncertaintycalculation::addInputAndOutputParameter_compilesAndIsSelected(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 10. );
    x1.setStdUncertainty( 1. );
    calc.addInputParameter( &x1 );

    InputParameter x2 {};
    x2.setName( "X2" );
    x2.setNominalValue( 20. );
    x2.setStdUncertainty( 2. );
    calc.addInputParameter( &x2 );

    QCOMPARE( InputParameter::getAll().size(), 2 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    y1.setConfidence( 0.95 );
    calc.addOutputParameter( &y1 );

    QCOMPARE( OutputParameter::getAll().size(), 1 );
    OutputParameter *added { OutputParameter::getByName( "Y1" ) };
    QVERIFY( added != nullptr );
    QVERIFY( added->getValid() );
    QVERIFY( qAbs( added->getNominalValue() - 30. ) < 1e-9 );

    // appendToModel() (used internally) automatically selects the new row
    QCOMPARE( OutputParameter::getSelected(), added );
    // 'unsavedChanges' is exposed as a Q_PROPERTY with a private getter
    QVERIFY( calc.property( "unsavedChanges" ).toBool() );
}


void tst_uncertaintycalculation::addInputParameter_undoRedoRestoresModel() {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QVERIFY( UndoStack::instance().canUndo() );
    QVERIFY( !UndoStack::instance().canRedo() );

    calc.undo();
    QCOMPARE( InputParameter::getAll().size(), 0 );
    QVERIFY( !UndoStack::instance().canUndo() );
    QVERIFY( UndoStack::instance().canRedo() );

    calc.redo();
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QCOMPARE( InputParameter::getAll().first()->getName(), QString( "X1" ) );
}


void tst_uncertaintycalculation::addCorrelation_cascadesToOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    InputParameter *paramX1 { InputParameter::getByName( "X1" ) };

    InputParameter x2 {};
    x2.setName( "X2" );
    calc.addInputParameter( &x2 );
    InputParameter *paramX2 { InputParameter::getByName( "X2" ) };

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );
    OutputParameter *addedOutputParam { OutputParameter::getByName( "Y1" ) };
    QVERIFY( addedOutputParam );
    QVERIFY( addedOutputParam->getValid() );
    double originalCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };

    Correlation correlation {};
    correlation.setInputParameterA( paramX1 );
    correlation.setInputParameterB( paramX2 );
    correlation.setCorrelation( 0.5 );
    calc.addCorrelation( &correlation );
    QCOMPARE( Correlation::getAll().size(), 1 );

    // The OutputParameter should be recompiled and the combined uncertainty
    // changed due to the new Correlation
    QVERIFY( addedOutputParam->getValid() );
    double newCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };
    QVERIFY( std::abs(
                    newCombinedStdUncertainty -
                    originalCombinedStdUncertainty
             ) > 1e-9
    );

    // Undo must delete the Correlation, and — via
    // UncertaintyCalculation::onTransactionApplied() recompiling all
    // OutputParameters after every undo/redo — Y1 becomes valid again too
    calc.undo();
    QCOMPARE( Correlation::getAll().size(), 0 );
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        originalCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );

    // Redo must restore the removed correlation state
    calc.redo();
    QCOMPARE( Correlation::getAll().size(), 1 );
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        newCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );
}


void tst_uncertaintycalculation::addInputParameter_cascadesToOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );

    // Add OutputParameter with invalid formula due to the missing
    // InputParameter X2
    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );
    OutputParameter *addedOutputParam { OutputParameter::getByName( "Y1" ) };
    QVERIFY( addedOutputParam );
    // The OutputParameter should be invalid due to missing X2
    QVERIFY( !addedOutputParam->getValid() );

    // Add missing InputParameter X2
    InputParameter x2 {};
    x2.setName( "X2" );
    calc.addInputParameter( &x2 );
    QCOMPARE( InputParameter::getAll().size(), 2 );
    QVERIFY( InputParameter::getByName( "X2" ) );

    // The OutputParameter should be recompiled and valid now
    QVERIFY( addedOutputParam->getValid() );

    // Undo should remove X2 again and make Y1 invalid
    calc.undo();
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QVERIFY( !InputParameter::getByName( "X2" ) );
    QVERIFY( !addedOutputParam->getValid() );

    // Redo should add X2 again and make Y1 valid
    calc.redo();
    QCOMPARE( InputParameter::getAll().size(), 2 );
    QVERIFY( InputParameter::getByName( "X2" ) );
    QVERIFY( addedOutputParam->getValid() );
}


void tst_uncertaintycalculation::removeCorrelation_cascadesToOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    InputParameter *paramX1 { InputParameter::getByName( "X1" ) };

    InputParameter x2 {};
    x2.setName( "X2" );
    calc.addInputParameter( &x2 );
    InputParameter *paramX2 { InputParameter::getByName( "X2" ) };

    Correlation correlation {};
    correlation.setInputParameterA( paramX1 );
    correlation.setInputParameterB( paramX2 );
    correlation.setCorrelation( 0.5 );
    calc.addCorrelation( &correlation );
    QCOMPARE( Correlation::getAll().size(), 1 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );
    OutputParameter *addedOutputParam { OutputParameter::getByName( "Y1" ) };
    QVERIFY( addedOutputParam );
    QVERIFY( addedOutputParam->getValid() );
    double originalCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };

    // Select correlation between X1 and X2 and remove it
    Correlation *addedCorr {
        Correlation::getCorrelation( paramX1->getId(), paramX2->getId() )
    };
    QVERIFY( addedCorr );
    Correlation::getCorrelationModel()->selectRow(
        Correlation::getRowIndex( addedCorr->getId() )
    );
    calc.removeCorrelation();
    QCOMPARE( Correlation::getAll().size(), 0 );

    // The OutputParameter should be recompiled and the combined uncertainty
    // changed because of the removal of the Correlation
    QVERIFY( addedOutputParam->getValid() );
    double newCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };
    QVERIFY( std::abs(
                newCombinedStdUncertainty -
                originalCombinedStdUncertainty
             ) > 1e-9
    );

    // Undo must restore the Correlation, and — via
    // UncertaintyCalculation::onTransactionApplied() recompiling all
    // OutputParameters after every undo/redo — Y1 becomes valid again too
    calc.undo();
    QCOMPARE( Correlation::getAll().size(), 1 );
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        originalCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );

    // Redo must restore the removed correlation state
    calc.redo();
    QCOMPARE( Correlation::getAll().size(), 0 );
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        newCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );
}

void tst_uncertaintycalculation::removeInputParameter_cascadesToCorrelationsAndOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    InputParameter *paramX1 { InputParameter::getByName( "X1" ) };

    InputParameter x2 {};
    x2.setName( "X2" );
    calc.addInputParameter( &x2 );
    InputParameter *paramX2 { InputParameter::getByName( "X2" ) };

    Correlation correlation {};
    correlation.setInputParameterA( paramX1 );
    correlation.setInputParameterB( paramX2 );
    correlation.setCorrelation( 0.5 );
    calc.addCorrelation( &correlation );
    QCOMPARE( Correlation::getAll().size(), 1 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );
    QVERIFY( OutputParameter::getByName( "Y1" )->getValid() );

    // Select X1 and remove it
    InputParameter::getInputModel()->selectRow(
        InputParameter::getRowIndex( paramX1->getId() )
    );
    calc.removeInputParameter();

    QCOMPARE( InputParameter::getAll().size(), 1 );
    QCOMPARE( InputParameter::getAll().first()->getName(), QString( "X2" ) );
    // The correlation referencing the deleted InputParameter must be gone
    QCOMPARE( Correlation::getAll().size(), 0 );
    // The formula referencing the deleted InputParameter can no longer
    // compile
    QVERIFY( !OutputParameter::getByName( "Y1" )->getValid() );

    // Undo must restore the deleted InputParameter, and — via
    // UncertaintyCalculation::onTransactionApplied() recompiling all
    // OutputParameters after every undo/redo — Y1 becomes valid again too
    calc.undo();
    QCOMPARE( InputParameter::getAll().size(), 2 );
    QVERIFY( OutputParameter::getByName( "Y1" )->getValid() );
    QCOMPARE( Correlation::getAll().size(), 1 );
}

void tst_uncertaintycalculation::updateCorrelation_cascadesToOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    InputParameter *paramX1 { InputParameter::getByName( "X1" ) };

    InputParameter x2 {};
    x2.setName( "X2" );
    calc.addInputParameter( &x2 );
    InputParameter *paramX2 { InputParameter::getByName( "X2" ) };

    Correlation correlation {};
    correlation.setInputParameterA( paramX1 );
    correlation.setInputParameterB( paramX2 );
    correlation.setCorrelation( 0.5 );
    calc.addCorrelation( &correlation );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );
    OutputParameter *addedOutputParam { OutputParameter::getByName( "Y1" ) };
    QVERIFY( addedOutputParam );
    QVERIFY( addedOutputParam->getValid() );
    double originalCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };

    // Select correlation between X1 and X2 and update it
    Correlation *addedCorr {
        Correlation::getCorrelation( paramX1->getId(), paramX2->getId() )
    };
    QVERIFY( addedCorr );
    Correlation::getCorrelationModel()->selectRow(
        Correlation::getRowIndex( addedCorr->getId() )
    );
    Correlation updatedCorr { correlation };
    updatedCorr.setCorrelation( -0.5 );
    calc.updateCorrelation( &updatedCorr );

    // The OutputParameter should be recompiled and the combined uncertainty
    // changed because of the updated correlation
    QVERIFY( addedOutputParam->getValid() );
    double newCombinedStdUncertainty {
        addedOutputParam->getCombinedStdUncertainty()
    };
    QVERIFY( std::abs(
                newCombinedStdUncertainty -
                originalCombinedStdUncertainty
             ) > 1e-9
    );

    // Undo must restore the original correlation, and — via
    // UncertaintyCalculation::onTransactionApplied() recompiling all
    // OutputParameters after every undo/redo — Y1's combined standard
    // uncertainty is back to its original value
    calc.undo();
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        originalCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );

    // Redo must restore the updated correlation and the new combined standard
    // uncertainty
    calc.redo();
    QVERIFY( addedOutputParam->getValid() );
    QCOMPARE(
        newCombinedStdUncertainty,
        addedOutputParam->getCombinedStdUncertainty()
    );
}

void tst_uncertaintycalculation::updateInputParameter_cascadesToOutputsAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    InputParameter *paramX1 { InputParameter::getByName( "X1" ) };

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1" );
    calc.addOutputParameter( &y1 );
    OutputParameter *addedOutputParam { OutputParameter::getByName( "Y1" ) };
    QVERIFY( addedOutputParam->getValid() );

    // Select X1 and update its name
    InputParameter::getInputModel()->selectRow(
        InputParameter::getRowIndex( paramX1->getId() )
    );
    InputParameter updatedX1 { x1 };
    updatedX1.setName( "X2" );
    calc.updateInputParameter( &updatedX1 );

    // The OutputParameter Y1 must now be invalid because X1 is missing
    QVERIFY( !addedOutputParam->getValid() );

    // Undo must restore the InputParameter, and — via
    // UncertaintyCalculation::onTransactionApplied() recompiling all
    // OutputParameters after every undo/redo — Y1 becomes valid again too
    calc.undo();
    QVERIFY( addedOutputParam->getValid() );

    // Redo must make Y1 invalid again
    calc.redo();
    QVERIFY( !addedOutputParam->getValid() );
}


void tst_uncertaintycalculation::projectJson_roundTripInMemory() {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 1. );
    x1.setStdUncertainty( 0.1 );
    calc.addInputParameter( &x1 );

    InputParameter x2 {};
    x2.setName( "X2" );
    x2.setNominalValue( 2. );
    x2.setStdUncertainty( 0.2 );
    calc.addInputParameter( &x2 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1 + X2" );
    calc.addOutputParameter( &y1 );

    QJsonObject json { calc.projectToJson() };
    QVERIFY( json[ "inputParameters" ].toArray().size() == 2 );
    QVERIFY( json[ "outputParameters" ].toArray().size() == 1 );

    double expectedCombinedUncertainty {
        OutputParameter::getByName( "Y1" )->getCombinedStdUncertainty()
    };

    calc.newProject();
    QCOMPARE( InputParameter::getAll().size(), 0 );
    QCOMPARE( OutputParameter::getAll().size(), 0 );

    calc.projectFromJson( json );

    QCOMPARE( InputParameter::getAll().size(), 2 );
    QCOMPARE( OutputParameter::getAll().size(), 1 );
    OutputParameter *restored { OutputParameter::getByName( "Y1" ) };
    QVERIFY( restored != nullptr );
    QVERIFY( restored->getValid() );
    QVERIFY(
        qAbs(
            restored->getCombinedStdUncertainty() - expectedCombinedUncertainty
        ) < 1e-9
    );

    // projectFromJson() parents the newly created objects to 'calc' (so that
    // a long-lived UncertaintyCalculation instance, as used by the real
    // application, owns them for its entire lifetime). Since 'calc' here is
    // a local test variable, explicitly clear the models — and thereby
    // properly delete those objects — while 'calc' is still alive, instead
    // of leaving that to 'calc's destructor (whose child cleanup would race
    // with cleanup()'s own InputParameter::clearModel() etc. calls).
    calc.newProject();
}


void tst_uncertaintycalculation::saveAndLoadProject_roundTripThroughFile() {
    QTemporaryDir tempDir {};
    QVERIFY( tempDir.isValid() );
    QString path { tempDir.filePath( "project.sig" ) };
    QUrl url { QUrl::fromLocalFile( path ) };

    {
        UncertaintyCalculation calc {};

        InputParameter x1 {};
        x1.setName( "X1" );
        x1.setNominalValue( 5. );
        x1.setStdUncertainty( 0.5 );
        calc.addInputParameter( &x1 );

        OutputParameter y1 {};
        y1.setName( "Y1" );
        y1.setUnit( "mm" );
        y1.setFormula( "2 * X1" );
        y1.setConfidence( 0.93 );
        calc.addOutputParameter( &y1 );

        QVERIFY( calc.saveProject( url ) );
        QVERIFY( QFile::exists( path ) );
        QCOMPARE( calc.getProjectFileName(), QString( "project.sig" ) );
    }

    // Reset the (static) Core layer state to simulate a fresh application
    // instance before loading
    OutputParameter::clearModel();
    Correlation::clearModel();
    InputParameter::clearModel();
    UndoStack::instance().clear();

    UncertaintyCalculation calc {};
    QVERIFY( calc.loadProject( url ) );

    QCOMPARE( InputParameter::getAll().size(), 1 );
    InputParameter *x1 { InputParameter::getByName( "X1" ) };
    QVERIFY( x1 != nullptr );
    QCOMPARE( x1->getNominalValue(), 5. );
    QCOMPARE( x1->getStdUncertainty(), 0.5 );

    OutputParameter *y1 { OutputParameter::getByName( "Y1" ) };
    QVERIFY( y1 != nullptr );
    QCOMPARE( y1->getUnit(), QString( "mm" ) );
    QCOMPARE( y1->getFormula(), QString( "2 * X1" ) );
    QCOMPARE( y1->getConfidence(), 0.93 );
    QVERIFY( y1->getValid() );
    QVERIFY( qAbs( y1->getNominalValue() - 10. ) < 1e-9 );

    // Loading a project clears the undo history
    QVERIFY( !UndoStack::instance().canUndo() );

    // loadProject() (via projectFromJson()) parents the newly created
    // objects to 'calc'. Clear the models explicitly while 'calc' is still
    // alive, rather than relying on its destructor to cascade-delete them
    // (see the comment in projectJson_roundTripInMemory() for details).
    calc.newProject();
}


void tst_uncertaintycalculation::loadProject_invalidUrl_fails() {
    UncertaintyCalculation calc {};
    QVERIFY( !calc.loadProject( QUrl( "not-a-local-file" ) ) );
}


void tst_uncertaintycalculation::saveCSV_writesExpectedSections() {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 1. );
    x1.setStdUncertainty( 0.1 );
    calc.addInputParameter( &x1 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1" );
    calc.addOutputParameter( &y1 );

    QTemporaryDir tempDir {};
    QVERIFY( tempDir.isValid() );
    QString path { tempDir.filePath( "export.csv" ) };
    QVERIFY( calc.saveCSV( QUrl::fromLocalFile( path ) ) );

    QFile csvFile { path };
    QVERIFY( csvFile.open( QIODevice::ReadOnly | QIODevice::Text ) );
    QString content { QString::fromUtf8( csvFile.readAll() ) };

    QVERIFY( content.contains( "Sigma version:" ) );
    QVERIFY( content.contains( "Input parameters:" ) );
    QVERIFY( content.contains( "Output parameters:" ) );
    QVERIFY( content.contains( "Combined uncertainty:" ) );
    QVERIFY( content.contains( "\"X1\"" ) );
    QVERIFY( content.contains( "\"Y1\"" ) );
}


void tst_uncertaintycalculation::runMonteCarlo_updatesSelectedOutputAndIsUndoable(
) {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 0. );
    x1.setStdUncertainty( 1. );
    calc.addInputParameter( &x1 );

    OutputParameter y1 {};
    y1.setName( "Y1" );
    y1.setFormula( "X1" );
    y1.setConfidence( 0.95 );
    calc.addOutputParameter( &y1 );

    // addOutputParameter() auto-selects the new row
    OutputParameter *selected { OutputParameter::getSelected() };
    QVERIFY( selected != nullptr );
    QVERIFY( !selected->getMonteCarloValid() );

    QSignalSpy finishedSpy { &calc, &UncertaintyCalculation::monteCarloFinished };
    calc.runMonteCarlo();
    QVERIFY( finishedSpy.wait( 30000 ) );

    QVERIFY( selected->getMonteCarloValid() );
    QVERIFY( qAbs( selected->getMonteCarlo().getMean() ) < 0.2 );
    QVERIFY( qAbs( selected->getMonteCarlo().getStdDeviation() - 1. ) < 0.2 );

    // The Monte Carlo simulation result must be undoable
    QVERIFY( UndoStack::instance().canUndo() );
    calc.undo();
    QVERIFY( !selected->getMonteCarloValid() );
}


void tst_uncertaintycalculation::newProject_clearsEverything() {
    UncertaintyCalculation calc {};

    InputParameter x1 {};
    x1.setName( "X1" );
    calc.addInputParameter( &x1 );
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QVERIFY( UndoStack::instance().canUndo() );

    calc.newProject();

    QCOMPARE( InputParameter::getAll().size(), 0 );
    QVERIFY( !UndoStack::instance().canUndo() );
    QVERIFY( !UndoStack::instance().canRedo() );
}


void tst_uncertaintycalculation::projectFromJson_addsNewUnit() {
    UncertaintyCalculation calc1 {};
    QStringList defaultUnits { calc1.getUnits() };

    // An existing unit should not be added to the list
    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setUnit( "m" );  // included in the default units list
    calc1.addInputParameter( &x1 );
    QCOMPARE( InputParameter::getAll().size(), 1 );

    // An new unit should be added to the list
    InputParameter x2 {};
    x2.setName( "X2" );
    x2.setUnit( "not-existing-unit" );  // included in the default units list
    calc1.addInputParameter( &x2 );
    QCOMPARE( InputParameter::getAll().size(), 2 );

    QJsonObject json { calc1.projectToJson() };

    // Restore the project from json
    UncertaintyCalculation calc2 {};
    calc2.projectFromJson( json );
    // New units list should contain 'not-existing-unit'
    QCOMPARE( calc2.getUnits().size(), defaultUnits.size() + 1 );
    QVERIFY( calc2.getUnits().contains( "not-existing-unit" ) );

    // projectFromJson() parents the newly created objects to 'calc'. Clear the
    // models explicitly while 'calc' is stil alive, rather than relying on its
    // destructor to cascade-delete them (see the comment in
    // projectJson_roundTripInMemory() for details).
    calc1.newProject();
    calc2.newProject();
}


QTEST_MAIN( tst_uncertaintycalculation )
#include "tst_uncertaintycalculation.moc"
