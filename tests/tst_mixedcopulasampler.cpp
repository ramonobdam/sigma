// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include "mixedcopulasampler.hpp"
#include <QTest>
#include <cmath>
#include <random>
#include <vector>

// Unit tests for MixedCopulaSampler: generates correlated samples for mixed
// marginal distributions via a Gaussian copula, using its own uniform and
// standard normal generator (independent of the C++ standard library).
class tst_mixedcopulasampler : public QObject {
    Q_OBJECT

private slots:
    void uniform_isBuiltFrom53GeneratorBits();
    void uniform_isInOpenUnitInterval();
    void standardNormal_fixedSeed_givesPortableValues();
    void standardNormal_hasZeroMeanAndUnitStdDev();
    void setRandomSeed_restartsTheSequence();

    void normalVariable_isMeanPlusStdDevTimesNormal();
    void correlatedNormalVariables_haveTheLatentCorrelation();
    void invCDFVariable_isMappedViaTheUniformDistribution();

    void factorize_positiveDefinite_usesCholesky();
    void factorize_perfectCorrelation_isSupported();
    void factorize_inconsistentCorrelations_fails();
    void perfectCorrelation_samplesAreEqualOrOpposite();
    void invalidCorrelation_samplerIsNotReady();
};

using Sampler = MixedCopulaSampler<std::mt19937>;


void tst_mixedcopulasampler::uniform_isBuiltFrom53GeneratorBits() {
    // The uniform value consists of the upper 27 bits of the first and the
    // upper 26 bits of the second 32-bit generator output
    std::mt19937 generator { 12345u };
    const uint64_t a { static_cast<uint64_t>( generator() ) >> 5 };
    const uint64_t b { static_cast<uint64_t>( generator() ) >> 6 };
    const double expected {
        ( static_cast<double>( ( a << 26 ) | b ) + 0.5 ) / 9007199254740992.
    };

    Sampler sampler { 12345u };
    QCOMPARE( sampler.uniform(), expected );
    QCOMPARE( expected, 0.92961609281714797 );
}


void tst_mixedcopulasampler::uniform_isInOpenUnitInterval() {
    Sampler sampler { 1u };
    double sum { 0. };
    const int n { 100000 };
    for ( int i { 0 }; i < n; ++i ) {
        const double u { sampler.uniform() };
        QVERIFY( u > 0. );
        QVERIFY( u < 1. );
        sum += u;
    }
    QVERIFY( qAbs( sum / n - 0.5 ) < 0.005 );
}


void tst_mixedcopulasampler::standardNormal_fixedSeed_givesPortableValues() {
    // These values were obtained with MSVC (Windows) and GCC (Linux), which
    // give identical results. A difference indicates a platform dependency
    // (e.g. in std::log() or std::sqrt()), or a change of the algorithm.
    const std::vector<double> expected {
        0.47894333805754763,
        -0.20470765948471256,
        -0.55573030434749049,
        -0.51943871505673855,
        1.3934058329729904,
        1.9657805725027142,
        0.28174615283020105,
        0.092907876743717158
    };

    Sampler sampler { 12345u };
    for ( size_t i { 0 }; i < expected.size(); ++i ) {
        const double value { sampler.standardNormal() };
        QVERIFY2(
            qAbs( value - expected[ i ] ) <= 1e-15 * qAbs( expected[ i ] ),
            qPrintable(
                QString( "value %1: %2 (expected %3)" ).arg(
                    QString::number( i ),
                    QString::number( value, 'g', 17 ),
                    QString::number( expected[ i ], 'g', 17 )
                )
            )
        );
    }
}


void tst_mixedcopulasampler::standardNormal_hasZeroMeanAndUnitStdDev() {
    Sampler sampler { 7u };
    const int n { 1000000 };
    double sum { 0. };
    double sumOfSquares { 0. };
    for ( int i { 0 }; i < n; ++i ) {
        const double z { sampler.standardNormal() };
        sum += z;
        sumOfSquares += z * z;
    }
    const double mean { sum / n };
    const double variance { sumOfSquares / n - mean * mean };
    QVERIFY( qAbs( mean ) < 0.005 );
    QVERIFY( qAbs( std::sqrt( variance ) - 1. ) < 0.005 );
}


void tst_mixedcopulasampler::setRandomSeed_restartsTheSequence() {
    // Reseeding must also discard the spare normal value
    Sampler sampler { 3u };
    const double first { sampler.standardNormal() };
    sampler.setRandomSeed( 3u );
    QCOMPARE( sampler.standardNormal(), first );
}


void tst_mixedcopulasampler::normalVariable_isMeanPlusStdDevTimesNormal() {
    Sampler sampler { 11u };
    sampler.addNormalVariable( 10., 2. );
    sampler.setLatentCorrelation( Eigen::MatrixXd::Identity( 1, 1 ) );

    Sampler reference { 11u };
    std::vector<double> values( 1 );
    for ( int i { 0 }; i < 100; ++i ) {
        sampler.getRandomSymbolValues( values );
        QCOMPARE( values[ 0 ], 10. + 2. * reference.standardNormal() );
    }
}


void tst_mixedcopulasampler::correlatedNormalVariables_haveTheLatentCorrelation(
) {
    const double rho { 0.8 };
    Eigen::MatrixXd correlation { Eigen::MatrixXd::Identity( 2, 2 ) };
    correlation( 0, 1 ) = rho;
    correlation( 1, 0 ) = rho;

    Sampler sampler { 5u };
    sampler.addNormalVariable( 0., 1. );
    sampler.addNormalVariable( 0., 1. );
    sampler.setLatentCorrelation( correlation );

    std::vector<double> values( 2 );
    const int n { 200000 };
    double sumXY { 0. };
    double sumXX { 0. };
    double sumYY { 0. };
    for ( int i { 0 }; i < n; ++i ) {
        sampler.getRandomSymbolValues( values );
        sumXY += values[ 0 ] * values[ 1 ];
        sumXX += values[ 0 ] * values[ 0 ];
        sumYY += values[ 1 ] * values[ 1 ];
    }
    QVERIFY( qAbs( sumXY / std::sqrt( sumXX * sumYY ) - rho ) < 0.01 );
}


void tst_mixedcopulasampler::invCDFVariable_isMappedViaTheUniformDistribution(
) {
    // A uniform variable on [4, 6]: u = Phi( z ), x = 4 + 2u
    Sampler sampler { 13u };
    sampler.addVariable( Distribution::invCDFUniform( 5., 1. ) );
    sampler.setLatentCorrelation( Eigen::MatrixXd::Identity( 1, 1 ) );

    Sampler reference { 13u };
    std::vector<double> values( 1 );
    for ( int i { 0 }; i < 100; ++i ) {
        sampler.getRandomSymbolValues( values );
        const double u {
            Distribution::normalCDF( reference.standardNormal() )
        };
        QCOMPARE( values[ 0 ], 5. + 1. * ( 2. * u - 1. ) );
        QVERIFY( values[ 0 ] > 4. && values[ 0 ] < 6. );
    }
}


namespace {
    // 2x2 or 3x3 correlation matrix with the given off-diagonal values
    Eigen::MatrixXd correlationMatrix( double r12 ) {
        Eigen::MatrixXd rho { Eigen::MatrixXd::Identity( 2, 2 ) };
        rho( 0, 1 ) = rho( 1, 0 ) = r12;
        return rho;
    }

    Eigen::MatrixXd correlationMatrix( double r12, double r13, double r23 ) {
        Eigen::MatrixXd rho { Eigen::MatrixXd::Identity( 3, 3 ) };
        rho( 0, 1 ) = rho( 1, 0 ) = r12;
        rho( 0, 2 ) = rho( 2, 0 ) = r13;
        rho( 1, 2 ) = rho( 2, 1 ) = r23;
        return rho;
    }
}


void tst_mixedcopulasampler::factorize_positiveDefinite_usesCholesky() {
    // A positive definite matrix gives exactly the Cholesky factor, so the
    // results are the same as before the eigendecomposition fallback
    const Eigen::MatrixXd rho { correlationMatrix( 0.8, 0.3, 0.1 ) };
    Eigen::MatrixXd L {};
    QVERIFY( CorrelationMatrix::factorize( rho, L ) );
    const Eigen::MatrixXd cholesky { Eigen::LLT<Eigen::MatrixXd>( rho ).matrixL() };
    QVERIFY( L == cholesky );
}


void tst_mixedcopulasampler::factorize_perfectCorrelation_isSupported() {
    for ( double r : { 1., -1. } ) {
        const Eigen::MatrixXd rho { correlationMatrix( r ) };
        Eigen::MatrixXd L {};
        QVERIFY( CorrelationMatrix::factorize( rho, L ) );
        QVERIFY( ( L * L.transpose() - rho ).cwiseAbs().maxCoeff() < 1e-12 );
    }

    // A singular, consistent 3x3 matrix: r23 = 0.81 - 0.19 = 0.62
    const Eigen::MatrixXd rho { correlationMatrix( 0.9, 0.9, 0.62 ) };
    Eigen::MatrixXd L {};
    QVERIFY( CorrelationMatrix::factorize( rho, L ) );
    QVERIFY( ( L * L.transpose() - rho ).cwiseAbs().maxCoeff() < 1e-12 );
}


void tst_mixedcopulasampler::factorize_inconsistentCorrelations_fails() {
    QVERIFY( !CorrelationMatrix::isValid( correlationMatrix( 0.9, 0.9, -0.9 ) ) );
    QVERIFY( CorrelationMatrix::isValid( correlationMatrix( 0.9, 0.9, 0.9 ) ) );
    QVERIFY( CorrelationMatrix::isValid( Eigen::MatrixXd {} ) );
}


void tst_mixedcopulasampler::perfectCorrelation_samplesAreEqualOrOpposite() {
    for ( double r : { 1., -1. } ) {
        Sampler sampler { 17u };
        sampler.addNormalVariable( 0., 1. );
        sampler.addNormalVariable( 0., 1. );
        QVERIFY( sampler.setLatentCorrelation( correlationMatrix( r ) ) );
        QVERIFY( sampler.canSample( 2 ) );
        QVERIFY( !sampler.canSample( 3 ) );

        std::vector<double> values( 2 );
        double sumOfSquares { 0. };
        for ( int i { 0 }; i < 1000; ++i ) {
            QVERIFY( sampler.getRandomSymbolValues( values ) );
            QVERIFY( qAbs( values[ 1 ] - r * values[ 0 ] ) < 1e-12 );
            sumOfSquares += values[ 0 ] * values[ 0 ];
        }
        // The samples are not degenerate
        QVERIFY( sumOfSquares / 1000 > 0.5 );
    }
}


void tst_mixedcopulasampler::invalidCorrelation_samplerIsNotReady() {
    Sampler sampler { 19u };
    sampler.addNormalVariable( 0., 1. );
    sampler.addNormalVariable( 0., 1. );
    sampler.addNormalVariable( 0., 1. );
    QVERIFY(
        !sampler.setLatentCorrelation( correlationMatrix( 0.9, 0.9, -0.9 ) )
    );
    QVERIFY( !sampler.isReady() );
    QVERIFY( !sampler.canSample( 3 ) );

    // No values are generated (instead of silently keeping the old values)
    std::vector<double> values { 1., 2., 3. };
    QVERIFY( !sampler.getRandomSymbolValues( values ) );
    QCOMPARE( values, std::vector<double>( { 1., 2., 3. } ) );
}


QTEST_MAIN( tst_mixedcopulasampler )
#include "tst_mixedcopulasampler.moc"
