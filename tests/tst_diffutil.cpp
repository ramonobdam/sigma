// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "diffutil.h"
#include "inputparameter.h"
#include "undostack.h"
#include <QTest>

// Unit tests for the undo/redo engine: DiffUtil records before/after JSON
// snapshots of Data objects and turns them into Transactions, which are
// pushed onto the (singleton) UndoStack. UndoStack::undo()/redo() then
// dispatch each JsonDiff back to DiffUtil::applyDiff(), which is registered
// per DataType. This mirrors exactly how
// UncertaintyCalculation::registerDiffUtilDataTypeMethods() wires things up,
// using InputParameter as the concrete DataType under test.
class tst_diffutil : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void addObject_undoRemovesRedoRestores();
    void updateObject_undoRestoresPreviousValue();
    void deleteObject_undoReinsertsAtOriginalRow();

    void unchangedObject_producesNoTransaction();
    void abortChanges_discardsSnapshots();

    void singleTransaction_canGroupMultipleDiffs();

    void pushTransaction_truncatesRedoHistory();
    void restoreState_jumpsAcrossMultipleTransactions();
};


void tst_diffutil::initTestCase() {
    DiffUtil::registerApplyDiff(
        DataType::InputParameter, &InputParameter::applyDiff
    );
    DiffUtil::registerCurrentJson(
        DataType::InputParameter, &InputParameter::currentJson
    );
    DiffUtil::registerGetRowIndex(
        DataType::InputParameter, &InputParameter::getRowIndex
    );
}


void tst_diffutil::cleanup() {
    UndoStack::instance().clear();
    InputParameter::clearModel();
}


void tst_diffutil::addObject_undoRemovesRedoRestores() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    InputParameter *added { parameter.appendToModel() };
    QUuid id { added->getId() };

    DiffUtil diffUtil {};
    diffUtil.takeSnapshotOfNewObject( added );
    diffUtil.commitChanges( "Add X1" );

    QVERIFY( UndoStack::instance().canUndo() );
    QVERIFY( !UndoStack::instance().canRedo() );
    QCOMPARE( UndoStack::instance().getLabelAt( 0 ), QString( "Add X1" ) );

    UndoStack::instance().undo();
    QCOMPARE( InputParameter::getAll().size(), 0 );
    QVERIFY( !UndoStack::instance().canUndo() );
    QVERIFY( UndoStack::instance().canRedo() );

    UndoStack::instance().redo();
    QCOMPARE( InputParameter::getAll().size(), 1 );
    InputParameter *restored { InputParameter::getById( id ) };
    QVERIFY( restored != nullptr );
    QCOMPARE( restored->getName(), QString( "X1" ) );
}


void tst_diffutil::updateObject_undoRestoresPreviousValue() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    parameter.setNominalValue( 1. );
    InputParameter *added { parameter.appendToModel() };
    QUuid id { added->getId() };

    // The "add" is its own transaction, exactly like the application does
    DiffUtil addDiff {};
    addDiff.takeSnapshotOfNewObject( added );
    addDiff.commitChanges( "Add X1" );

    // Now update the nominal value, following the same before/mutate/commit
    // pattern used by UncertaintyCalculation::updateInputParameter()
    DiffUtil updateDiff {};
    updateDiff.takeSnapshot( added );

    InputParameter newState { *added };
    newState.setNominalValue( 99. );
    QVERIFY( InputParameter::update( id, &newState ) );

    updateDiff.commitChanges( "Update X1" );

    QCOMPARE( InputParameter::getById( id )->getNominalValue(), 99. );
    QCOMPARE( UndoStack::instance().getStackSize(), 2 );

    UndoStack::instance().undo();  // undo the update
    QCOMPARE( InputParameter::getById( id )->getNominalValue(), 1. );

    UndoStack::instance().redo();  // redo the update
    QCOMPARE( InputParameter::getById( id )->getNominalValue(), 99. );

    UndoStack::instance().undo();  // undo the update
    UndoStack::instance().undo();  // undo the add
    QCOMPARE( InputParameter::getAll().size(), 0 );
}


void tst_diffutil::deleteObject_undoReinsertsAtOriginalRow() {
    InputParameter first {};
    first.setName( "X1" );
    InputParameter *addedFirst { first.appendToModel() };

    InputParameter second {};
    second.setName( "X2" );
    InputParameter *addedSecond { second.appendToModel() };
    QUuid secondId { addedSecond->getId() };

    QCOMPARE( InputParameter::getAll().size(), 2 );

    DiffUtil diffUtil {};
    diffUtil.takeSnapshot( addedSecond );
    QVERIFY( InputParameter::remove( secondId ) );
    diffUtil.commitChanges( "Delete X2" );

    QCOMPARE( InputParameter::getAll().size(), 1 );

    UndoStack::instance().undo();
    QCOMPARE( InputParameter::getAll().size(), 2 );
    QCOMPARE( InputParameter::getRowIndex( secondId ), 1 );
    QCOMPARE( InputParameter::getById( secondId )->getName(), QString( "X2" ) );

    UndoStack::instance().redo();
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QCOMPARE( InputParameter::getById( secondId ), nullptr );

    Q_UNUSED( addedFirst )
}


void tst_diffutil::unchangedObject_producesNoTransaction() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    InputParameter *added { parameter.appendToModel() };

    DiffUtil diffUtil {};
    diffUtil.takeSnapshot( added );
    // No actual change is made
    diffUtil.commitChanges( "No-op" );

    QCOMPARE( UndoStack::instance().getStackSize(), 0 );
}


void tst_diffutil::abortChanges_discardsSnapshots() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    InputParameter *added { parameter.appendToModel() };

    DiffUtil diffUtil {};
    diffUtil.takeSnapshot( added );
    added->setNominalValue( 42. );
    diffUtil.abortChanges();
    diffUtil.commitChanges( "Should not be pushed" );

    QCOMPARE( UndoStack::instance().getStackSize(), 0 );
}


void tst_diffutil::singleTransaction_canGroupMultipleDiffs() {
    DiffUtil diffUtil {};

    InputParameter a {};
    a.setName( "X1" );
    InputParameter *addedA { a.appendToModel() };
    diffUtil.takeSnapshotOfNewObject( addedA );

    InputParameter b {};
    b.setName( "X2" );
    InputParameter *addedB { b.appendToModel() };
    diffUtil.takeSnapshotOfNewObject( addedB );

    diffUtil.commitChanges( "Add X1 and X2" );

    QCOMPARE( UndoStack::instance().getStackSize(), 1 );
    QCOMPARE( InputParameter::getAll().size(), 2 );

    // A single undo must revert both additions since they are part of the
    // same transaction
    UndoStack::instance().undo();
    QCOMPARE( InputParameter::getAll().size(), 0 );

    UndoStack::instance().redo();
    QCOMPARE( InputParameter::getAll().size(), 2 );
}


void tst_diffutil::pushTransaction_truncatesRedoHistory() {
    DiffUtil diffUtil1 {};
    InputParameter a {};
    a.setName( "X1" );
    InputParameter *addedA { a.appendToModel() };
    diffUtil1.takeSnapshotOfNewObject( addedA );
    diffUtil1.commitChanges( "Add X1" );

    UndoStack::instance().undo();
    QVERIFY( UndoStack::instance().canRedo() );

    // Pushing a new transaction while redo history exists must discard it
    DiffUtil diffUtil2 {};
    InputParameter b {};
    b.setName( "X2" );
    InputParameter *addedB { b.appendToModel() };
    diffUtil2.takeSnapshotOfNewObject( addedB );
    diffUtil2.commitChanges( "Add X2" );

    QVERIFY( !UndoStack::instance().canRedo() );
    QCOMPARE( UndoStack::instance().getStackSize(), 1 );
    QCOMPARE( UndoStack::instance().getLabelAt( 0 ), QString( "Add X2" ) );
}


void tst_diffutil::restoreState_jumpsAcrossMultipleTransactions() {
    QStringList names { "X1", "X2", "X3" };
    int numNames { static_cast<int>( names.size() ) };
    for ( const QString &name : names ) {
        DiffUtil diffUtil {};
        InputParameter parameter {};
        parameter.setName( name );
        InputParameter *added { parameter.appendToModel() };
        diffUtil.takeSnapshotOfNewObject( added );
        diffUtil.commitChanges( "Add " + name );
    }
    QCOMPARE( InputParameter::getAll().size(), numNames );
    QCOMPARE( UndoStack::instance().getCursor(), numNames );

    UndoStack::instance().restoreState( 1 );
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QCOMPARE( UndoStack::instance().getCursor(), 1 );

    UndoStack::instance().restoreState( numNames );
    QCOMPARE( InputParameter::getAll().size(), numNames );
    QCOMPARE( UndoStack::instance().getCursor(), numNames );

    UndoStack::instance().restoreState( 0 );
    QCOMPARE( InputParameter::getAll().size(), 0 );
}


QTEST_MAIN( tst_diffutil )
#include "tst_diffutil.moc"
