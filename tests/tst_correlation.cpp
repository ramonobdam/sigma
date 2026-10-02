// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include <QTest>

// Unit tests for Correlation: the class that stores the correlation
// coefficient between two InputParameters, used by the Monte Carlo simulation
// (via the Gaussian copula) and the analytic uncertainty budget calculation.
class tst_correlation : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void getValid_requiresTwoDistinctRegisteredParameters();
    void getValid_requiresCorrelationInRange();

    void jsonRoundTrip();

    void appendToModel_rejectsNonUniqueCorrelation();
    void getCorrelation_isOrderIndependent();
    void getCorrelationsForInputParameter_findsBothSides();

    void remove_removesFromModel();
    void update_requiresUniqueOrSameParameters();

    void reconnectInputParameters_restoresPointersFromIds();
    void applyDiff_addUpdateAndRemove();

private:
    InputParameter *mParamA;
    InputParameter *mParamB;
    InputParameter *mParamC;
};


void tst_correlation::init() {
    InputParameter a {};
    a.setName( "A" );
    mParamA = a.appendToModel();

    InputParameter b {};
    b.setName( "B" );
    mParamB = b.appendToModel();

    InputParameter c {};
    c.setName( "C" );
    mParamC = c.appendToModel();

    QVERIFY( mParamA && mParamB && mParamC );
}


void tst_correlation::cleanup() {
    Correlation::clearModel();
    InputParameter::clearModel();
}


void tst_correlation::getValid_requiresTwoDistinctRegisteredParameters() {
    Correlation valid { nullptr, mParamA, mParamB, 0.5 };
    QVERIFY( valid.getValid() );

    Correlation sameParameter { nullptr, mParamA, mParamA, 0.5 };
    QVERIFY( !sameParameter.getValid() );

    Correlation noParameterB { nullptr, mParamA, nullptr, 0.5 };
    QVERIFY( !noParameterB.getValid() );
}


void tst_correlation::getValid_requiresCorrelationInRange() {
    Correlation tooHigh { nullptr, mParamA, mParamB, 1.5 };
    QVERIFY( !tooHigh.getValid() );

    Correlation tooLow { nullptr, mParamA, mParamB, -1.5 };
    QVERIFY( !tooLow.getValid() );

    Correlation atBound { nullptr, mParamA, mParamB, 1. };
    QVERIFY( atBound.getValid() );
}


void tst_correlation::jsonRoundTrip() {
    Correlation original { nullptr, mParamA, mParamB, 0.42 };

    QJsonObject json { original.toJson() };
    QCOMPARE(
        json[ "IdInputParameterA" ].toString(),
        mParamA->getId().toString()
    );
    QCOMPARE(
        json[ "IdInputParameterB" ].toString(),
        mParamB->getId().toString()
    );
    QCOMPARE( json[ "correlation" ].toDouble(), original.getCorrelation() );

    Correlation restored { Correlation::fromJson( json, false ) };
    QCOMPARE( restored.getInputParameterAId(), mParamA->getId() );
    QCOMPARE( restored.getInputParameterBId(), mParamB->getId() );
    QCOMPARE( restored.getCorrelation(), original.getCorrelation() );
    QCOMPARE( restored.getId(), original.getId() );
}


void tst_correlation::appendToModel_rejectsNonUniqueCorrelation() {
    Correlation first { nullptr, mParamA, mParamB, 0.3 };
    QVERIFY( first.appendToModel() != nullptr );
    QCOMPARE( Correlation::getAll().size(), 1 );

    // Same InputParameters, reversed order -> still not unique
    Correlation duplicate { nullptr, mParamB, mParamA, 0.7 };
    QCOMPARE( duplicate.appendToModel(), nullptr );
    QCOMPARE( Correlation::getAll().size(), 1 );

    // A correlation between a different pair is fine
    Correlation other { nullptr, mParamA, mParamC, 0.1 };
    QVERIFY( other.appendToModel() != nullptr );
    QCOMPARE( Correlation::getAll().size(), 2 );
}


void tst_correlation::getCorrelation_isOrderIndependent() {
    Correlation correlation { nullptr, mParamA, mParamB, 0.55 };
    Correlation *added { correlation.appendToModel() };
    QVERIFY( added != nullptr );

    QCOMPARE(
        Correlation::getCorrelation( mParamA->getId(), mParamB->getId() ),
        added
    );
    QCOMPARE(
        Correlation::getCorrelation( mParamB->getId(), mParamA->getId() ),
        added
    );
    QCOMPARE(
        Correlation::getCorrelation( mParamA->getId(), mParamC->getId() ),
        nullptr
    );
}


void tst_correlation::getCorrelationsForInputParameter_findsBothSides() {
    Correlation ab { nullptr, mParamA, mParamB, 0.2 };
    ab.appendToModel();
    Correlation ac { nullptr, mParamA, mParamC, 0.3 };
    ac.appendToModel();

    QList<Correlation *> forA {
        Correlation::getCorrelationsForInputParameter( mParamA->getId() )
    };
    QCOMPARE( forA.size(), 2 );

    QList<Correlation *> forB {
        Correlation::getCorrelationsForInputParameter( mParamB->getId() )
    };
    QCOMPARE( forB.size(), 1 );

    QList<Correlation *> forNull {
        Correlation::getCorrelationsForInputParameter( QUuid() )
    };
    QCOMPARE( forNull.size(), 0 );
}


void tst_correlation::remove_removesFromModel() {
    Correlation correlation { nullptr, mParamA, mParamB, 0.5 };
    Correlation *added { correlation.appendToModel() };
    QUuid id { added->getId() };

    QVERIFY( Correlation::remove( id ) );
    QCOMPARE( Correlation::getAll().size(), 0 );
    QCOMPARE( Correlation::getById( id ), nullptr );
}


void tst_correlation::update_requiresUniqueOrSameParameters() {
    Correlation ab { nullptr, mParamA, mParamB, 0.5 };
    Correlation *addedAB { ab.appendToModel() };
    Correlation ac { nullptr, mParamA, mParamC, 0.1 };
    Correlation *addedAC { ac.appendToModel() };
    Q_UNUSED( addedAC )

    // Changing just the coefficient, same parameters -> allowed
    Correlation updatedAB { *addedAB };
    updatedAB.setCorrelation( 0.9 );
    QVERIFY( Correlation::update( addedAB->getId(), &updatedAB ) );
    QCOMPARE(
        Correlation::getById( addedAB->getId() )->getCorrelation(), 0.9
    );

    // Changing to a pair that already exists elsewhere in the model ->
    // rejected
    Correlation conflicting { nullptr, mParamA, mParamC, 0.2 };
    QVERIFY( !Correlation::update( addedAB->getId(), &conflicting ) );
}


void tst_correlation::reconnectInputParameters_restoresPointersFromIds() {
    // Simulates what happens after loading a project from JSON: correlations
    // are constructed from Ids only, and the InputParameter pointers need to
    // be reconnected afterwards.
    QJsonObject json {};
    json[ "IdInputParameterA" ] = mParamA->getId().toString();
    json[ "IdInputParameterB" ] = mParamB->getId().toString();
    json[ "correlation" ] = 0.6;

    Correlation restored { Correlation::fromJson( json, true ) };
    Q_UNUSED( restored )

    QCOMPARE( Correlation::getAll().size(), 1 );
    Correlation *stored { Correlation::getAll().first() };

    // updateFromJson() only stores the Ids — the InputParameter pointers are
    // still unresolved at this point
    QCOMPARE( stored->getInputParameterA(), nullptr );
    QCOMPARE( stored->getInputParameterB(), nullptr );

    // reconnectAllCorrelations() (called after loading a project) resolves
    // the pointers from the stored Ids
    Correlation::reconnectAllCorrelations();
    QCOMPARE( stored->getInputParameterA(), mParamA );
    QCOMPARE( stored->getInputParameterB(), mParamB );
}


void tst_correlation::applyDiff_addUpdateAndRemove() {
    Correlation correlation { nullptr, mParamA, mParamB, 0.25 };
    Correlation *added { correlation.appendToModel() };
    QUuid id { added->getId() };
    QJsonObject afterAdd { added->toJson() };

    // Simulate undo of an "add" transaction: before is empty, after is empty
    // -> the object must be removed
    Correlation::applyDiff( JsonDiff { id, DataType::Correlation, 0,
                                        QJsonObject {}, QJsonObject {} } );
    QCOMPARE( Correlation::getById( id ), nullptr );

    // Simulate redo of the "add" transaction: before empty, after contains
    // the full object state -> it must be re-inserted at the stored row
    Correlation::applyDiff( JsonDiff { id, DataType::Correlation, 0,
                                        QJsonObject {}, afterAdd } );
    Correlation *reAdded { Correlation::getById( id ) };
    QVERIFY( reAdded != nullptr );
    QCOMPARE( reAdded->getCorrelation(), 0.25 );

    // Simulate an update: only the changed key needs to be present
    QJsonObject patch {};
    patch[ "correlation" ] = 0.8;
    Correlation::applyDiff(
        JsonDiff { id, DataType::Correlation, 0, afterAdd, patch }
    );
    QCOMPARE( Correlation::getById( id )->getCorrelation(), 0.8 );
}


QTEST_MAIN( tst_correlation )
#include "tst_correlation.moc"
