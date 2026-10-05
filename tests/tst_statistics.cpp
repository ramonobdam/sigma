// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "statistics.h"
#include <QTest>
#include <QtNumeric>
#include <cmath>
#include <numeric>

// Unit tests for the Statistics class: mean/standard deviation, coverage
// interval bounds and the histogram used to display Monte Carlo simulation
// results.
class tst_statistics : public QObject {
    Q_OBJECT

private slots:
    void defaultConstructed_hasNoSamples();
    void addSample_singleSample();
    void addSamples_meanAndStdDev();
    void clearSamples_resetsToDefaults();

    void coverageBounds_fullCoverageEqualsMinMax();
    void coverageBounds_zeroCoverageCollapsesToOnePoint();
    void setP_invalidatesBoundsButNotStdDevs();

    void histogram_probabilitiesSumToOne();
    void histogram_allEqualSamples();

    void getSamples_returnsAllAddedSamples();
};


void tst_statistics::defaultConstructed_hasNoSamples() {
    Statistics stat {};
    QCOMPARE( stat.getNumberOfSamples(), 0 );
    QCOMPARE( stat.getMean(), 0. );
    QCOMPARE( stat.getStdDev(), 0. );
    QCOMPARE( stat.getStdDevOfTheMean(), 0. );
    QCOMPARE( stat.getLowerBound(), 0. );
    QCOMPARE( stat.getHigherBound(), 0. );

    // Default histogram bounds, as set by resetHistogram()
    QCOMPARE( stat.getHistogramXMin(), -1. );
    QCOMPARE( stat.getHistogramXMax(), 1. );
    QCOMPARE( stat.getHistogramYMax(), 1. );
    QCOMPARE( stat.getHistogramLowerIndex(), -1 );
    QCOMPARE( stat.getHistogramValues().size(), 71 );
}


void tst_statistics::addSample_singleSample() {
    Statistics stat {};
    stat.addSample( 42. );
    QCOMPARE( stat.getNumberOfSamples(), 1 );
    QCOMPARE( stat.getMean(), 42. );
    // Standard deviation is undefined (0.) for n <= 1
    QCOMPARE( stat.getStdDev(), 0. );
    QCOMPARE( stat.getStdDevOfTheMean(), 0. );
}


void tst_statistics::addSamples_meanAndStdDev() {
    Statistics stat {};
    stat.addSamples( { 1., 2., 3., 4., 5. } );

    QCOMPARE( stat.getNumberOfSamples(), 5 );
    QVERIFY( qAbs( stat.getMean() - 3. ) < 1e-12 );
    // Sample variance (n-1 denominator): (4+1+0+1+4)/4 = 2.5
    QVERIFY( qAbs( stat.getStdDev() - std::sqrt( 2.5 ) ) < 1e-12 );
    QVERIFY(
        qAbs( stat.getStdDevOfTheMean() - std::sqrt( 2.5 / 5. ) ) < 1e-12
    );

    // Adding more samples afterwards must update the (lazily calculated)
    // statistics
    stat.addSample( 100. );
    QCOMPARE( stat.getNumberOfSamples(), 6 );
    QVERIFY( stat.getMean() > 3. );
}


void tst_statistics::clearSamples_resetsToDefaults() {
    Statistics stat {};
    stat.addSamples( { 1., 2., 3. } );
    QCOMPARE( stat.getNumberOfSamples(), 3 );

    stat.clearSamples();
    QCOMPARE( stat.getNumberOfSamples(), 0 );
    QCOMPARE( stat.getMean(), 0. );
    QCOMPARE( stat.getStdDev(), 0. );
}


void tst_statistics::coverageBounds_fullCoverageEqualsMinMax() {
    Statistics stat { 1. };  // p = 1 -> 100% coverage
    stat.addSamples( { 5., 1., 4., 2., 3. } );

    QCOMPARE( stat.getLowerBound(), 1. );
    QCOMPARE( stat.getHigherBound(), 5. );
}


void tst_statistics::coverageBounds_zeroCoverageCollapsesToOnePoint() {
    Statistics stat { 0. };  // p = 0 -> the interval collapses
    stat.addSamples( { 1., 2., 3., 4., 5. } );

    // With zero coverage, the lower and higher bound must coincide
    QCOMPARE( stat.getLowerBound(), stat.getHigherBound() );
    QVERIFY( stat.getLowerBound() - 3. < 1e-12 );
}


void tst_statistics::setP_invalidatesBoundsButNotStdDevs() {
    Statistics stat { 1. };
    stat.addSamples( { 1., 2., 3., 4., 5. } );
    QCOMPARE( stat.getLowerBound(), 1. );
    QCOMPARE( stat.getHigherBound(), 5. );

    stat.setP( 0. );
    QCOMPARE( stat.getP(), 0. );
    QCOMPARE( stat.getLowerBound(), stat.getHigherBound() );

    // The mean is unaffected by changing p
    QVERIFY( qAbs( stat.getMean() - 3. ) < 1e-12 );
}


void tst_statistics::histogram_probabilitiesSumToOne() {
    Statistics stat {};
    std::vector<double> samples {};
    for ( int i { 0 }; i < 1000; ++i ) {
        samples.push_back( static_cast<double>( i ) );
    }
    stat.addSamples( samples );

    const QList<double> &values { stat.getHistogramValues() };
    QCOMPARE( values.size(), 71 );

    double sum { std::accumulate( values.begin(), values.end(), 0. ) };
    QVERIFY( qAbs( sum - 1. ) < 1e-9 );

    QCOMPARE( stat.getHistogramXMin(), 0. );
    QCOMPARE( stat.getHistogramXMax(), 999. );
    QVERIFY( stat.getHistogramYMax() > 0. );
    QVERIFY( stat.getHistogramYMax() <= 1. );
    QVERIFY( stat.getHistogramLowerIndex() >= -1 );
    QVERIFY( stat.getHistogramHigherIndex() <= 71 );
}


void tst_statistics::histogram_allEqualSamples() {
    Statistics stat {};
    stat.addSamples( { 7., 7., 7., 7. } );

    const QList<double> &values { stat.getHistogramValues() };
    QCOMPARE( values.size(), 71 );
    // All samples fall on the same value, so binSize is 0, and the special
    // case fills every bin with the same value 1/sNumBins
    for ( double value : values ) {
        QVERIFY( qAbs( value - 1. / 71. ) < 1e-12 );
    }
}


void tst_statistics::getSamples_returnsAllAddedSamples() {
    Statistics stat {};
    stat.addSample( 1. );
    stat.addSamples( { 2., 3. } );

    const std::vector<double> &samples { stat.getSamples() };
    QCOMPARE( static_cast<int>( samples.size() ), 3 );
    QVERIFY(
        std::find( samples.begin(), samples.end(), 1. ) != samples.end()
    );
    QVERIFY(
        std::find( samples.begin(), samples.end(), 2. ) != samples.end()
    );
    QVERIFY(
        std::find( samples.begin(), samples.end(), 3. ) != samples.end()
    );
}


QTEST_MAIN( tst_statistics )
#include "tst_statistics.moc"
