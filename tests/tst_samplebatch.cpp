// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "outputparameter.h"
#include "samplebatch.h"
#include "statistics.h"
#include <QRegularExpression>
#include <QTest>
#include <QtNumeric>
#include <cmath>

// Unit tests for SampleBatch: generates one batch of Monte Carlo simulation
// samples (GUM JCGM 101:2008) for an OutputParameter's measurement function,
// drawing correlated input values via the Gaussian copula (see
// MixedCopulaSampler).
class tst_samplebatch : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void generateSamples_singleNormalVariable_matchesAnalyticStatistics();
    void generateSamples_isReproducibleWithSameSeed();
    void generateSamples_differentSeedsProduceDifferentSamples();
    void generateSamples_storeInputValues_storesOneRowPerSample();

    void generateSamples_invalidOutput_reportsError();

    void generateSamples_correlatedInputs_matchesCombinedUncertainty();

    void generateSamples_withoutOutputParameter_fails();

    void setBatchSize_belowOneIsClamped();
};


void tst_samplebatch::initTestCase() {
    // ExprTk must be told to collect the referenced variables so that
    // OutputParameter::compile() can build the uncertainty components
    OutputParameter::setCollectVariables( true );
}


void tst_samplebatch::cleanup() {
    OutputParameter::clearModel();
    Correlation::clearModel();
    InputParameter::clearModel();
}


void tst_samplebatch::generateSamples_singleNormalVariable_matchesAnalyticStatistics(
) {
    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 10. );
    x1.setStdUncertainty( 2. );
    x1.setDistribution( Distribution::Type::normal );
    x1.appendToModel();

    OutputParameter output {};
    output.setFormula( "X1" );
    output.compile();
    QVERIFY( output.getValid() );

    const int batchSize { 20000 };
    SampleBatch batch { &output, batchSize, 12345 };
    QVERIFY( batch.generateSamples() );
    QVERIFY( batch.getError().isEmpty() );

    const std::vector<double> &samples { batch.getSamples() };
    QCOMPARE( static_cast<int>( samples.size() ), batchSize );

    Statistics stat {};
    stat.addSamples( samples );
    QVERIFY( qAbs( stat.getMean() - 10. ) < 0.1 );
    QVERIFY( qAbs( stat.getStdDev() - 2. ) < 0.1 );
}


void tst_samplebatch::generateSamples_isReproducibleWithSameSeed() {
    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setStdUncertainty( 1. );
    x1.appendToModel();

    OutputParameter output {};
    output.setFormula( "X1" );
    output.compile();

    SampleBatch batchA { &output, 100, 777 };
    QVERIFY( batchA.generateSamples() );

    SampleBatch batchB { &output, 100, 777 };
    QVERIFY( batchB.generateSamples() );

    QCOMPARE( batchA.getSamples(), batchB.getSamples() );
}


void tst_samplebatch::generateSamples_differentSeedsProduceDifferentSamples() {
    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setStdUncertainty( 1. );
    x1.appendToModel();

    OutputParameter output {};
    output.setFormula( "X1" );
    output.compile();

    SampleBatch batchA { &output, 100, 1 };
    QVERIFY( batchA.generateSamples() );

    SampleBatch batchB { &output, 100, 2 };
    QVERIFY( batchB.generateSamples() );

    QVERIFY( batchA.getSamples() != batchB.getSamples() );
}


void tst_samplebatch::generateSamples_storeInputValues_storesOneRowPerSample() {
    for ( const QString &name : { "X1", "X2" } ) {
        InputParameter parameter {};
        parameter.setName( name );
        parameter.setStdUncertainty( 1. );
        parameter.appendToModel();
    }

    OutputParameter output {};
    output.setFormula( "X1 + 2 * X2" );
    output.compile();
    QVERIFY( output.getValid() );

    // The input values are stored in the order of the components
    const QList<UncertaintyComponent> components { output.getComponents() };
    QCOMPARE( components.size(), 2 );
    const int iX1 { components[ 0 ].getName() == "X1" ? 0 : 1 };
    const int iX2 { 1 - iX1 };

    const int batchSize { 100 };
    SampleBatch batch { &output, batchSize, 42 };
    QVERIFY( batch.generateSamples( true ) );
    QCOMPARE( batch.getNumberOfInputs(), 2 );

    const std::vector<double> &inputs { batch.getInputValues() };
    const std::vector<double> &outputs { batch.getSamples() };
    QCOMPARE( static_cast<int>( outputs.size() ), batchSize );
    QCOMPARE( static_cast<int>( inputs.size() ), 2 * batchSize );
    for ( int i { 0 }; i < batchSize; ++i ) {
        const double x1 { inputs[ 2 * i + iX1 ] };
        const double x2 { inputs[ 2 * i + iX2 ] };
        QVERIFY( qAbs( outputs[ i ] - ( x1 + 2 * x2 ) ) < 1e-12 );
    }

    // Storing the input values doesn't change the samples, and without
    // storing no input values are kept
    SampleBatch batchWithoutInputs { &output, batchSize, 42 };
    QVERIFY( batchWithoutInputs.generateSamples() );
    QCOMPARE( batchWithoutInputs.getSamples(), outputs );
    QVERIFY( batchWithoutInputs.getInputValues().empty() );
}


void tst_samplebatch::generateSamples_invalidOutput_reportsError() {
    // The nominal value (1.) keeps compile() itself valid, but a standard
    // uncertainty this large compared to the nominal value means about half
    // of the randomly sampled values for X1 will be negative, making
    // sqrt(X1) NaN for those samples
    InputParameter x1 {};
    x1.setName( "X1" );
    x1.setNominalValue( 1. );
    x1.setStdUncertainty( 100. );
    x1.setDistribution( Distribution::Type::normal );
    x1.appendToModel();

    OutputParameter output {};
    output.setFormula( "sqrt(X1)" );
    output.compile();
    QVERIFY( output.getValid() );

    SampleBatch batch { &output, 100, 1 };
    QVERIFY( !batch.generateSamples() );
    QVERIFY( batch.getError().startsWith(
        "Invalid output value for input parameter values:"
    ) );
}


void tst_samplebatch::generateSamples_correlatedInputs_matchesCombinedUncertainty(
) {
    InputParameter *x1 {};
    InputParameter *x2 {};
    {
        InputParameter a {};
        a.setName( "X1" );
        a.setStdUncertainty( 1. );
        x1 = a.appendToModel();
        QVERIFY( x1 );

        InputParameter b {};
        b.setName( "X2" );
        b.setStdUncertainty( 1. );
        x2 = b.appendToModel();
        QVERIFY( x2 );
    }

    Correlation correlation { nullptr, x1->getId(), x2->getId(), 0.5 };
    QVERIFY( correlation.appendToModel() );

    OutputParameter output {};
    output.setFormula( "X1 + X2" );
    output.compile();
    QVERIFY( output.getValid() );

    // Analytic combined standard uncertainty (GUM JCGM 100:2008): sqrt(3)
    double analyticStdDev { output.getCombinedStdUncertainty() };
    QVERIFY( qAbs( analyticStdDev - std::sqrt( 3. ) ) < 1e-6 );

    SampleBatch batch { &output, 200000, 99 };
    QVERIFY( batch.generateSamples() );

    Statistics stat {};
    stat.addSamples( batch.getSamples() );
    // The Monte Carlo standard deviation of a large batch must be close to
    // the analytic combined standard uncertainty
    QVERIFY( qAbs( stat.getStdDev() - analyticStdDev ) < 0.01 );
    QVERIFY( qAbs( stat.getMean() ) < 0.01 );
}


void tst_samplebatch::generateSamples_withoutOutputParameter_fails() {
    SampleBatch batch { nullptr, 100, 1 };
    QVERIFY( !batch.generateSamples() );
    QCOMPARE( batch.getError(), QString( "No samples could be generated" ) );
}


void tst_samplebatch::setBatchSize_belowOneIsClamped() {
    // The minimum batch size is 1
    QTest::ignoreMessage(
        QtCriticalMsg,
        QRegularExpression( "Invalid batch size of '0' is set to '1'" )
    );
    SampleBatch batch { nullptr, 0, 1 };
    QCOMPARE( batch.getBatchSize(), 1 );

    QTest::ignoreMessage(
        QtCriticalMsg,
        QRegularExpression( "Invalid batch size of '-1' is set to '1'" )
    );
    batch.setBatchSize( -1 );
    QCOMPARE( batch.getBatchSize(), 1 );

    batch.setBatchSize( 9999 );
    QCOMPARE( batch.getBatchSize(), 9999 );

}


QTEST_MAIN( tst_samplebatch )
#include "tst_samplebatch.moc"
