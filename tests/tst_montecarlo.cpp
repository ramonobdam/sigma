// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "inputparameter.h"
#include "montecarlo.h"
#include "outputparameter.h"
#include "settings.h"
#include <QList>
#include <QStringList>
#include <QTest>
#include <QtNumeric>
#include <cmath>

// Unit tests for MonteCarlo: performs the Monte Carlo simulation (GUM
// JCGM 101:2008) for an OutputParameter and stores its results (mean,
// standard deviation, coverage interval, histogram). run() is called
// directly (instead of via start()/QThread) so the algorithm runs
// synchronously and deterministically on the test thread.
class tst_montecarlo : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void defaultConstructed_hasEmptyResults();
    void resetResults_restoresDefaults();

    void run_additiveNormalModel_convergesToAnalyticStatistics();
    void run_requestStop_stopsWithoutConverging();
    void run_withoutOutputParameter_doesNothing();

    void jsonRoundTrip();
    void copyConstructor_copiesAllFields();
};


void tst_montecarlo::initTestCase() {
    OutputParameter::setCollectVariables( true );

    // Use small settings so the simulation converges quickly in a unit test
    QVERIFY( Settings::setMonteCarloBatchSize( 3000 ) );
    QVERIFY( Settings::setMonteCarloDigits( 1 ) );
    QVERIFY( Settings::setMonteCarloMaxNumOfBatches( 300 ) );
}


void tst_montecarlo::cleanup() {
    OutputParameter::clearModel();
    InputParameter::clearModel();
}


void tst_montecarlo::defaultConstructed_hasEmptyResults() {
    MonteCarlo mc {};
    QCOMPARE( mc.getMean(), 0. );
    QCOMPARE( mc.getStdDeviation(), 0. );
    QVERIFY( !mc.getValid() );
    QCOMPARE( mc.getOutputParameter(), nullptr );
}


void tst_montecarlo::resetResults_restoresDefaults() {
    MonteCarlo mc {};
    mc.setMean( 5. );
    mc.setStdDeviation( 2. );
    mc.setValid( true );

    mc.resetResults();
    QCOMPARE( mc.getMean(), 0. );
    QCOMPARE( mc.getStdDeviation(), 0. );
    QVERIFY( !mc.getValid() );
    QCOMPARE(
        mc.getStatus(),
        QString( "Click 'Start' to run Monte Carlo simulation" )
    );
}


void tst_montecarlo::run_additiveNormalModel_convergesToAnalyticStatistics() {
    QStringList names { "X1", "X2", "X3", "X4" };
    for ( const QString &name : names ) {
        InputParameter parameter {};
        parameter.setName( name );
        parameter.setNominalValue( 0. );
        parameter.setStdUncertainty( 1. );
        parameter.setDistribution( Distribution::Type::normal );
        parameter.appendToModel();
    }

    OutputParameter output {};
    output.setFormula( names.join( " + ") );
    output.setConfidence( 0.95 );
    output.compile();
    QVERIFY( output.getValid() );

    // Analytic combined standard uncertainty: sqrt(4 * 1^2) = 2
    double expected { std::sqrt( names.size() * std::pow( 1., 2. ) ) };
    QVERIFY( qAbs( output.getCombinedStdUncertainty() - expected ) < 1e-9 );

    MonteCarlo mc { &output };
    mc.run();

    QVERIFY( mc.getValid() );
    QVERIFY( mc.getStatus().contains( "Converged" ) );
    QVERIFY( qAbs( mc.getMean() ) < 0.1 );
    QVERIFY( qAbs( mc.getStdDeviation() - 2. ) < 0.1 );
    QVERIFY( mc.getLowerBound() < mc.getMean() );
    QVERIFY( mc.getHigherBound() > mc.getMean() );
    QCOMPARE( mc.getHistogramValues().size(), 71 );

    // The OutputParameter must be unlocked again once the simulation
    // finished
    QVERIFY( !output.getLocked() );
}


void tst_montecarlo::run_requestStop_stopsWithoutConverging() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    parameter.setStdUncertainty( 1. );
    parameter.appendToModel();

    OutputParameter output {};
    output.setFormula( "X1" );
    output.compile();
    QVERIFY( output.getValid() );

    MonteCarlo mc { &output };
    mc.setRequestStop( true );
    mc.run();

    QVERIFY( !mc.getValid() );
    QVERIFY( mc.getStatus().contains( "Stopped" ) );
    // The stop request is cleared again once handled
    QVERIFY( !mc.getRequestStop() );
}


void tst_montecarlo::run_withoutOutputParameter_doesNothing() {
    MonteCarlo mc { nullptr };
    mc.run();  // must not crash
    QVERIFY( !mc.getValid() );
}


void tst_montecarlo::jsonRoundTrip() {
    MonteCarlo mc {};
    mc.setNumericalTolerance( 0.05 );
    mc.setMean( 12.3 );
    mc.setStdDeviation( 1.1 );
    mc.setLowerBound( 10. );
    mc.setHigherBound( 14. );
    mc.setStatus( "Converged after 100 function evaluations" );
    mc.setValid( true );
    mc.setHistogramXMin( -5. );
    mc.setHistogramXMax( 5. );
    mc.setHistogramYMax( 0.4 );
    mc.setHistogramLowerIndex( 3 );
    mc.setHistogramHigherIndex( 60 );
    mc.setHistogramValues( { 0.1, 0.2, 0.3, 0.4 } );

    QJsonObject json { mc.toJson() };
    MonteCarlo restored { MonteCarlo::fromJson( json ) };

    QCOMPARE( restored.getNumericalTolerance(), 0.05 );
    QCOMPARE( restored.getMean(), 12.3 );
    QCOMPARE( restored.getStdDeviation(), 1.1 );
    QCOMPARE( restored.getLowerBound(), 10. );
    QCOMPARE( restored.getHigherBound(), 14. );
    QCOMPARE(
        restored.getStatus(),
        QString( "Converged after 100 function evaluations" )
    );
    QVERIFY( restored.getValid() );
    QCOMPARE( restored.getHistogramXMin(), -5. );
    QCOMPARE( restored.getHistogramXMax(), 5. );
    QCOMPARE( restored.getHistogramYMax(), 0.4 );
    QCOMPARE( restored.getHistogramLowerIndex(), 3 );
    QCOMPARE( restored.getHistogramHigherIndex(), 60 );
    QCOMPARE(
        restored.getHistogramValues(),
        QList<double>( { 0.1, 0.2, 0.3, 0.4 } )
    );
}


void tst_montecarlo::copyConstructor_copiesAllFields() {
    MonteCarlo original {};
    original.setNumericalTolerance( 0.05 );
    original.setMean( 12.3 );
    original.setStdDeviation( 1.1 );
    original.setLowerBound( 10. );
    original.setHigherBound( 14. );
    original.setStatus( "Converged after 100 function evaluations" );
    original.setValid( true );
    original.setHistogramXMin( -5. );
    original.setHistogramXMax( 5. );
    original.setHistogramYMax( 0.4 );
    original.setHistogramLowerIndex( 3 );
    original.setHistogramHigherIndex( 60 );
    original.setHistogramValues( { 0.1, 0.2, 0.3, 0.4 } );

    MonteCarlo copy { original };
    QCOMPARE( copy.getNumericalTolerance(), 0.05 );
    QCOMPARE( copy.getMean(), 12.3 );
    QCOMPARE( copy.getStdDeviation(), 1.1 );
    QCOMPARE( copy.getLowerBound(), 10. );
    QCOMPARE( copy.getHigherBound(), 14. );
    QCOMPARE( copy.getStatus(), "Converged after 100 function evaluations" );
    QVERIFY( copy.getValid() );
    QCOMPARE( copy.getHistogramXMin(), -5. );
    QCOMPARE( copy.getHistogramXMax(), 5. );
    QCOMPARE( copy.getHistogramYMax(), 0.4 );
    QCOMPARE( copy.getHistogramLowerIndex(), 3 );
    QCOMPARE( copy.getHistogramHigherIndex(), 60 );
    QCOMPARE(
        copy.getHistogramValues(),
        QList<double>( { 0.1, 0.2, 0.3, 0.4 } )
    );

    MonteCarlo assigned {};
    assigned = original;
    QCOMPARE( assigned.getNumericalTolerance(), 0.05 );
    QCOMPARE( assigned.getMean(), 12.3 );
    QCOMPARE( assigned.getStdDeviation(), 1.1 );
    QCOMPARE( assigned.getLowerBound(), 10. );
    QCOMPARE( assigned.getHigherBound(), 14. );
    QCOMPARE(
        assigned.getStatus(),
        "Converged after 100 function evaluations"
    );
    QVERIFY( assigned.getValid() );
    QCOMPARE( assigned.getHistogramXMin(), -5. );
    QCOMPARE( assigned.getHistogramXMax(), 5. );
    QCOMPARE( assigned.getHistogramYMax(), 0.4 );
    QCOMPARE( assigned.getHistogramLowerIndex(), 3 );
    QCOMPARE( assigned.getHistogramHigherIndex(), 60 );
    QCOMPARE(
        assigned.getHistogramValues(),
        QList<double>( { 0.1, 0.2, 0.3, 0.4 } )
    );

}


QTEST_MAIN( tst_montecarlo )
#include "tst_montecarlo.moc"
