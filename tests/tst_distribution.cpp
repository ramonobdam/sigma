// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include <QTest>
#include <QtNumeric>
#include <cmath>
#include <limits>
#include <vector>

// Unit tests for the Distribution namespace: the inverse CDFs used to draw
// samples for the Monte Carlo simulation and the distribution <-> string
// conversions used by the input parameter model and the .sig JSON format.
class tst_distribution : public QObject {
    Q_OBJECT

private slots:
    void distributionToString_roundTrip_data();
    void distributionToString_roundTrip();
    void stringToDistribution_invalidString();
    void getDistributions_containsAllTypes();

    void normalCDF_isSymmetricAroundZero();
    void normalCDF_lowerTailIsAccurate();
    void invCDFStandardNormal_boundsAndMedian();
    void invCDFStandardNormal_isMonotonicallyIncreasing();
    void invCDFStandardNormal_knownQuantiles_data();
    void invCDFStandardNormal_knownQuantiles();
    void invCDFStandardNormal_roundTripIncludingTails();
    void isNormal_normalAndInfiniteDofStudentsT();

    void invCDFNormal_medianEqualsMean();
    void invCDFUniform_boundsAndMedian();
    void invCDFTriangular_boundsAndMedian();
    void invCDFArcsine_boundsAndMedian();
    void invCDFConstant_alwaysReturnsMean();
    void invCDFStudentsT_medianEqualsMean();

    void getInvCDF_uniform_usesSqrt3HalfWidth();
    void getInvCDF_triangular_usesSqrt6HalfWidth();
    void getInvCDF_arcsine_usesSqrt2HalfWidth();
    void getInvCDF_studentst_infiniteDofEqualsNormal();
    void getInvCDF_studentst_usesStudentTQuantile();
    void getInvCDF_none_isConstant();

    void invCDFStandardStudentT_referenceValues_data();
    void invCDFStandardStudentT_referenceValues();
    void invCDFStandardStudentT_boundsAndSymmetry();
    void invCDFStandardStudentT_inverseOfCDF_data();
    void invCDFStandardStudentT_inverseOfCDF();
    void studentTCDF_referenceValues();
    void coverageFactor_referenceValues_data();
    void coverageFactor_referenceValues();
    void coverageFactor_limits();
    void coverageFactor_nonIntegerDOF_isBetweenIntegerDOF();
};


void tst_distribution::distributionToString_roundTrip_data() {
    QTest::addColumn<Distribution::Type>( "type" );
    QTest::addColumn<QString>( "text" );

    QTest::newRow( "normal" ) << Distribution::Type::normal << "normal";
    QTest::newRow( "uniform" ) << Distribution::Type::uniform << "uniform";
    QTest::newRow( "triangular" )
        << Distribution::Type::triangular << "triangular";
    QTest::newRow( "arcsine" ) << Distribution::Type::arcsine << "arcsine";
    QTest::newRow( "studentst" )
        << Distribution::Type::studentst << "student's t";
    QTest::newRow( "none" ) << Distribution::Type::none << "none";
}


void tst_distribution::distributionToString_roundTrip() {
    QFETCH( Distribution::Type, type );
    QFETCH( QString, text );

    QCOMPARE( Distribution::distributionToString( type ), text );

    Distribution::Type parsed {};
    QVERIFY( Distribution::stringToDistribution( text, parsed ) );
    QCOMPARE( parsed, type );
    QCOMPARE( Distribution::stringToDistribution( text ), type );

    // Leading/trailing whitespace must be tolerated, as it is when reading
    // from a .sig JSON file that may have been hand edited
    QVERIFY( Distribution::stringToDistribution( "  " + text + "  ", parsed ) );
    QCOMPARE( parsed, type );
}


void tst_distribution::stringToDistribution_invalidString() {
    Distribution::Type parsed { Distribution::Type::normal };
    QVERIFY(
        !Distribution::stringToDistribution( "not-a-distribution", parsed )
    );
    // On failure the type is reset to 'none'
    QCOMPARE( parsed, Distribution::Type::none );
}


void tst_distribution::getDistributions_containsAllTypes() {
    const QList<Distribution::Type> types { Distribution::getDistributions() };
    QCOMPARE( types.size(), 6 );
    QVERIFY( types.contains( Distribution::Type::normal ) );
    QVERIFY( types.contains( Distribution::Type::uniform ) );
    QVERIFY( types.contains( Distribution::Type::triangular ) );
    QVERIFY( types.contains( Distribution::Type::arcsine ) );
    QVERIFY( types.contains( Distribution::Type::studentst ) );
    QVERIFY( types.contains( Distribution::Type::none ) );
}


void tst_distribution::normalCDF_isSymmetricAroundZero() {
    QCOMPARE( Distribution::normalCDF( 0. ), 0.5 );
    QVERIFY( qFuzzyCompare( Distribution::normalCDF( 1. ) +
                             Distribution::normalCDF( -1. ), 1. ) );
    QVERIFY( Distribution::normalCDF( 2. ) > Distribution::normalCDF( 1. ) );
}


void tst_distribution::invCDFStandardNormal_boundsAndMedian() {
    QCOMPARE( Distribution::invCDFStandardNormal( 0. ),
              -std::numeric_limits<double>::infinity() );
    QCOMPARE( Distribution::invCDFStandardNormal( 1. ),
              std::numeric_limits<double>::infinity() );
    QVERIFY( qAbs( Distribution::invCDFStandardNormal( 0.5 ) ) < 1e-9 );

    // A known reference value: Phi^-1(0.975) ~= 1.959964
    QVERIFY( qAbs( Distribution::invCDFStandardNormal( 0.975 ) - 1.959964 ) <
             1e-4 );
}


void tst_distribution::invCDFStandardNormal_isMonotonicallyIncreasing() {
    double p { 0.01 };
    double step { 0.05 };
    double previous { Distribution::invCDFStandardNormal( p ) };
    while ( p < 1. ) {
        p += step;
        double current { Distribution::invCDFStandardNormal( p ) };
        QVERIFY( current > previous );
        previous = current;
    }
}


void tst_distribution::normalCDF_lowerTailIsAccurate() {
    const double phiMinus6 { Distribution::normalCDF( -6. ) };
    QVERIFY( qAbs( phiMinus6 / 9.865876450376946e-10 - 1. ) < 1e-12 );
    const double phiMinus8 { Distribution::normalCDF( -8. ) };
    QVERIFY( qAbs( phiMinus8 / 6.22096057427178e-16 - 1. ) < 1e-12 );
}


void tst_distribution::invCDFStandardNormal_knownQuantiles_data() {
    QTest::addColumn<double>( "p" );
    QTest::addColumn<double>( "z" );

    // Central region
    QTest::newRow( "0.5" ) << 0.5 << 0.;
    QTest::newRow( "0.1" ) << 0.1 << -1.2815515655446004;
    QTest::newRow( "0.9" ) << 0.9 << 1.2815515655446004;
    // Intermediate tails
    QTest::newRow( "0.05" ) << 0.05 << -1.6448536269514729;
    QTest::newRow( "0.025" ) << 0.025 << -1.959963984540054;
    QTest::newRow( "0.975" ) << 0.975 << 1.959963984540054;
    QTest::newRow( "1e-10" ) << 1e-10 << -6.361340902404056;
    // Far tail
    QTest::newRow( "1e-20" ) << 1e-20 << -9.262340089798408;
}


void tst_distribution::invCDFStandardNormal_knownQuantiles() {
    QFETCH( double, p );
    QFETCH( double, z );
    const double value { Distribution::invCDFStandardNormal( p ) };
    QVERIFY2(
        qAbs( value - z ) <= 1e-14 * qMax( qAbs( z ), 1. ),
        qPrintable(
            QString( "%1 (expected %2)" ).arg(
                QString::number( value, 'g', 17 ),
                QString::number( z, 'g', 17 )
            )
        )
    );
}


void tst_distribution::invCDFStandardNormal_roundTripIncludingTails() {
    // normalCDF( invCDFStandardNormal( p ) ) == p, from the far lower tail to
    // the median, covering all 3 regions of the AS241 algorithm
    std::vector<double> probabilities {};
    for ( int exponent { -300 }; exponent <= -1; ++exponent ) {
        probabilities.push_back( std::pow( 10., exponent ) );
    }
    for ( double p { 0.01 }; p < 0.5; p += 0.01 ) {
        probabilities.push_back( p );
    }

    for ( double p : probabilities ) {
        const double z { Distribution::invCDFStandardNormal( p ) };
        const double roundTrip { Distribution::normalCDF( z ) };
        QVERIFY2(
            qAbs( roundTrip / p - 1. ) < 1e-12,
            qPrintable(
                QString( "p = %1, z = %2, normalCDF( z ) = %3" ).arg(
                    QString::number( p, 'g', 17 ),
                    QString::number( z, 'g', 17 ),
                    QString::number( roundTrip, 'g', 17 )
                )
            )
        );

        // Symmetry around the median (only for p >= 0.01, since 1 - p can't
        // be represented accurately enough for smaller p)
        if ( p >= 0.01 ) {
            QVERIFY(
                qAbs( Distribution::invCDFStandardNormal( 1. - p ) + z ) <
                1e-12 * qMax( qAbs( z ), 1. )
            );
        }
    }
}


void tst_distribution::isNormal_normalAndInfiniteDofStudentsT() {
    using Distribution::Type;
    QVERIFY( Distribution::isNormal( Type::normal, false ) );
    QVERIFY( Distribution::isNormal( Type::normal, true ) );
    QVERIFY( Distribution::isNormal( Type::studentst, true ) );
    QVERIFY( !Distribution::isNormal( Type::studentst, false ) );
    QVERIFY( !Distribution::isNormal( Type::uniform, true ) );
    QVERIFY( !Distribution::isNormal( Type::none, true ) );
}


void tst_distribution::invCDFNormal_medianEqualsMean() {
    Distribution::InvCDF invCDF { Distribution::invCDFNormal( 10., 2. ) };
    QVERIFY( qAbs( invCDF( 0.5 ) - 10. ) < 1e-9 );
    // 1 standard deviation above the mean corresponds to Phi(1)
    double upper { invCDF( Distribution::normalCDF( 1. ) ) };
    QVERIFY( qAbs( upper - 12. ) < 1e-6 );
}


void tst_distribution::invCDFUniform_boundsAndMedian() {
    Distribution::InvCDF invCDF { Distribution::invCDFUniform( 5., 2. ) };
    QCOMPARE( invCDF( 0. ), 3. );
    QCOMPARE( invCDF( 1. ), 7. );
    QCOMPARE( invCDF( 0.5 ), 5. );
}


void tst_distribution::invCDFTriangular_boundsAndMedian() {
    Distribution::InvCDF invCDF { Distribution::invCDFTriangular( 0., 4. ) };
    QVERIFY( qAbs( invCDF( 0. ) - ( -4. ) ) < 1e-9 );
    QVERIFY( qAbs( invCDF( 1. ) - 4. ) < 1e-9 );
    QVERIFY( qAbs( invCDF( 0.5 ) - 0. ) < 1e-9 );
}


void tst_distribution::invCDFArcsine_boundsAndMedian() {
    Distribution::InvCDF invCDF { Distribution::invCDFArcsine( 1., 3. ) };
    QCOMPARE( invCDF( 0. ), -2. );  // mean - halfWidth
    QCOMPARE( invCDF( 1. ), 4. );   // mean + halfWidth
    QVERIFY( qAbs( invCDF( 0.5 ) - 1. ) < 1e-9 );  // median == mean
}


void tst_distribution::invCDFConstant_alwaysReturnsMean() {
    Distribution::InvCDF invCDF { Distribution::invCDFConstant( 42. ) };
    QCOMPARE( invCDF( 0. ), 42. );
    QCOMPARE( invCDF( 0.5 ), 42. );
    QCOMPARE( invCDF( 1. ), 42. );
}


void tst_distribution::invCDFStudentsT_medianEqualsMean() {
    Distribution::InvCDF invCDF { Distribution::invCDFStudentsT( 3., 1.5, 5 ) };
    QVERIFY( qAbs( invCDF( 0.5 ) - 3. ) < 1e-6 );
    // The t distribution is symmetric around the mean
    double lower { invCDF( 0.1 ) };
    double upper { invCDF( 0.9 ) };
    QVERIFY( qAbs( ( lower - 3. ) + ( upper - 3. ) ) < 1e-6 );
}


void tst_distribution::getInvCDF_uniform_usesSqrt3HalfWidth() {
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF( Distribution::Type::uniform, 0., 1., 1, true )
    };
    double halfWidth { std::sqrt( 3. ) };
    QVERIFY( qAbs( invCDF( 1. ) - halfWidth ) < 1e-9 );
    QVERIFY( qAbs( invCDF( 0. ) + halfWidth ) < 1e-9 );
}


void tst_distribution::getInvCDF_triangular_usesSqrt6HalfWidth() {
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF(
            Distribution::Type::triangular, 0., 1., 1, true
        )
    };
    double halfWidth { std::sqrt( 6. ) };
    QVERIFY( qAbs( invCDF( 1. ) - halfWidth ) < 1e-9 );
    QVERIFY( qAbs( invCDF( 0. ) + halfWidth ) < 1e-9 );
}


void tst_distribution::getInvCDF_arcsine_usesSqrt2HalfWidth() {
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF( Distribution::Type::arcsine, 0., 1., 1, true )
    };
    double halfWidth { std::sqrt( 2. ) };
    QVERIFY( qAbs( invCDF( 1. ) - halfWidth ) < 1e-9 );
    QVERIFY( qAbs( invCDF( 0. ) + halfWidth ) < 1e-9 );
}


void tst_distribution::getInvCDF_studentst_infiniteDofEqualsNormal() {
    // With infinite degrees of freedom, Student's t must behave exactly like
    // the normal distribution
    Distribution::InvCDF studentInf {
        Distribution::getInvCDF(
            Distribution::Type::studentst, 2., 1.5, 1, true
        )
    };
    Distribution::InvCDF normal { Distribution::invCDFNormal( 2., 1.5 ) };
    for ( double p : { 0.1, 0.3, 0.5, 0.7, 0.9 } ) {
        QVERIFY( qAbs( studentInf( p ) - normal( p ) ) < 1e-9 );
    }

    // With finite degrees of freedom, the tails must be wider (heavier) than
    // the normal distribution for the same standard uncertainty
    Distribution::InvCDF studentFinite {
        Distribution::getInvCDF(
            Distribution::Type::studentst, 2., 1.5, 3, false
        )
    };
    QVERIFY( studentFinite( 0.99 ) > normal( 0.99 ) );
}


namespace {
    // Relative accuracy required for the Student's t quantiles
    constexpr double relativeAccuracy { 1e-12 };

    bool relativeClose( double value, double reference, double tolerance ) {
        return std::abs( value - reference ) <=
               tolerance * std::abs( reference );
    }
}


void tst_distribution::getInvCDF_studentst_usesStudentTQuantile() {
    // The Monte Carlo sampling uses mean + stdDev * t-quantile
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF(
            Distribution::Type::studentst, 2., 1.5, 4, false
        )
    };
    QVERIFY(
        relativeClose(
            invCDF( 0.975 ), 2. + 1.5 * 2.7764451051977943, relativeAccuracy
        )
    );
    QVERIFY(
        relativeClose(
            invCDF( 0.025 ), 2. - 1.5 * 2.7764451051977943, relativeAccuracy
        )
    );

    for ( int dof : { 1, 2, 3, 10, 100 } ) {
        Distribution::InvCDF studentsT {
            Distribution::getInvCDF(
                Distribution::Type::studentst, -1., 0.5, dof, false
            )
        };
        for ( double p : { 1e-6, 0.01, 0.3, 0.5, 0.8, 0.999 } ) {
            QCOMPARE(
                studentsT( p ),
                -1. + 0.5 * Distribution::invCDFStandardStudentT( p, dof )
            );
        }
    }
}


void tst_distribution::getInvCDF_none_isConstant() {
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF( Distribution::Type::none, 7., 3., 1, true )
    };
    // The standard uncertainty is ignored for a constant ('none') parameter
    QCOMPARE( invCDF( 0. ), 7. );
    QCOMPARE( invCDF( 1. ), 7. );
}


void tst_distribution::invCDFStandardStudentT_referenceValues_data() {
    QTest::addColumn<double>( "p" );
    QTest::addColumn<double>( "dof" );
    QTest::addColumn<double>( "expected" );

    // Reference quantiles calculated with 113-bit floating point precision
    QTest::newRow( "dof 1" ) << 0.975 << 1. << 12.706204736174707;
    QTest::newRow( "dof 2" ) << 0.975 << 2. << 4.302652729749464;
    QTest::newRow( "dof 3" ) << 0.975 << 3. << 3.1824463052837095;
    QTest::newRow( "dof 4" ) << 0.975 << 4. << 2.7764451051977943;
    QTest::newRow( "dof 7" ) << 0.9 << 7. << 1.4149239276505084;
    QTest::newRow( "dof 10" ) << 0.975 << 10. << 2.2281388519862747;
    QTest::newRow( "dof 16" ) << 0.995 << 16. << 2.9207816224251000;
    QTest::newRow( "dof 5 tail" ) << 1e-10 << 5. << -156.82559270889432;
    QTest::newRow( "dof 1e6" ) << 0.975 << 1e6 << 1.9599663568141066;
    QTest::newRow( "dof infinite" )
        << 0.975 << std::numeric_limits<double>::infinity()
        << 1.9599639845400542;

    // Degrees of freedom below 0.5 (very heavy tails). Independent reference
    // quantiles calculated with mpmath (40 significant digits), checked with
    // scipy.stats.t.ppf
    QTest::newRow( "dof 0.1 p 0.6" ) << 0.6 << 0.1 << 1.512976170677862;
    QTest::newRow( "dof 0.1 p 0.9" ) << 0.9 << 0.1 << 1604425.7056665548;
    QTest::newRow( "dof 0.1 p 0.975" ) << 0.975 << 0.1 << 1682362288745.0105;
    QTest::newRow( "dof 0.25 p 0.6" ) << 0.6 << 0.25 << 0.56036036748301474;
    QTest::newRow( "dof 0.25 p 0.9" ) << 0.9 << 0.25 << 170.46892571212422;
    QTest::newRow( "dof 0.25 p 0.975" ) << 0.975 << 0.25 << 43640.149267980318;
    QTest::newRow( "dof 0.4 p 0.6" ) << 0.6 << 0.4 << 0.43570409818088622;
    QTest::newRow( "dof 0.4 p 0.9" ) << 0.9 << 0.4 << 20.082549252738439;
    QTest::newRow( "dof 0.4 p 0.975" ) << 0.975 << 0.4 << 642.82724615652831;
    QTest::newRow( "dof 0.4 p 0.025" ) << 0.025 << 0.4 << -642.82724615652831;
}


void tst_distribution::invCDFStandardStudentT_referenceValues() {
    QFETCH( double, p );
    QFETCH( double, dof );
    QFETCH( double, expected );

    const double value { Distribution::invCDFStandardStudentT( p, dof ) };
    QVERIFY2(
        relativeClose( value, expected, relativeAccuracy ),
        qPrintable( QString::number( value, 'g', 17 ) )
    );
}


void tst_distribution::invCDFStandardStudentT_boundsAndSymmetry() {
    const double inf { std::numeric_limits<double>::infinity() };
    QCOMPARE( Distribution::invCDFStandardStudentT( 0., 5. ), -inf );
    QCOMPARE( Distribution::invCDFStandardStudentT( 1., 5. ), inf );
    QCOMPARE( Distribution::invCDFStandardStudentT( 0.5, 5. ), 0. );
    QVERIFY( qIsNaN( Distribution::invCDFStandardStudentT( 0.5, 0. ) ) );

    for ( double dof : { 0.1, 0.25, 0.4, 0.5, 1., 1.6, 2., 3., 9., 30., 1e5 } ) {
        // Values of p for which 1 - p is exact in floating point arithmetic
        for ( double p : { 0x1p-40, 0x1p-13, 0x1p-7, 0.125, 0.3, 0.45 } ) {
            const double lower {
                Distribution::invCDFStandardStudentT( p, dof )
            };
            const double upper {
                Distribution::invCDFStandardStudentT( 1. - p, dof )
            };
            QVERIFY( lower < 0. );
            QVERIFY( relativeClose( -lower, upper, relativeAccuracy ) );
        }
    }

    // Monotonically increasing
    double previous { Distribution::invCDFStandardStudentT( 0.001, 3. ) };
    for ( double p { 0.011 }; p < 1.; p += 0.01 ) {
        const double current { Distribution::invCDFStandardStudentT( p, 3. ) };
        QVERIFY( current > previous );
        previous = current;
    }
}


void tst_distribution::invCDFStandardStudentT_inverseOfCDF_data() {
    QTest::addColumn<double>( "dof" );

    // Including non-integer degrees of freedom (also below 1 and below 0.5)
    for (
        double dof : {
            0.1, 0.25, 0.4, 0.49, 0.5, 0.85, 1.6, 2.5, 3., 5., 8., 16.7, 25.,
            100., 1000., 25000.
        }
    ) {
        QTest::newRow( qPrintable( QString::number( dof ) ) ) << dof;
    }
}


void tst_distribution::invCDFStandardStudentT_inverseOfCDF() {
    QFETCH( double, dof );

    for ( double p : { 1e-15, 1e-9, 1e-5, 0.001, 0.025, 0.1, 0.3, 0.49 } ) {
        const double t { Distribution::invCDFStandardStudentT( p, dof ) };
        QVERIFY2(
            std::isfinite( t ) &&
            relativeClose( Distribution::studentTCDF( t, dof ), p, 1e-12 ),
            qPrintable(
                QString( "p = %1: t = %2" ).arg(
                    QString::number( p ),
                    QString::number( t, 'g', 17 )
                )
            )
        );
    }
}


void tst_distribution::studentTCDF_referenceValues() {
    QCOMPARE( Distribution::studentTCDF( 0., 4. ), 0.5 );
    QVERIFY(
        relativeClose(
            Distribution::studentTCDF( 1.5, 7. ),
            0.5 + 0.5 * 0.82270151301002997,
            1e-14
        )
    );
    QVERIFY(
        relativeClose(
            Distribution::studentTCDF( -4., 3. ),
            0.5 * ( 1. - 0.97199154398985383 ),
            1e-13
        )
    );
    // Cauchy distribution
    QVERIFY(
        relativeClose( Distribution::studentTCDF( 1., 1. ), 0.75, 1e-14 )
    );
}


void tst_distribution::coverageFactor_referenceValues_data() {
    QTest::addColumn<double>( "confidence" );
    QTest::addColumn<double>( "dof" );
    QTest::addColumn<double>( "expected" );

    const double inf { std::numeric_limits<double>::infinity() };
    QTest::newRow( "95% infinite" ) << 0.95 << inf << 1.9599639845400542;
    QTest::newRow( "99% infinite" ) << 0.99 << inf << 2.5758293035489008;
    QTest::newRow( "95% dof 4" ) << 0.95 << 4. << 2.7764451051977943;
    QTest::newRow( "68.27% dof 4" ) << 0.6827 << 4. << 1.1416549872215623;
    QTest::newRow( "99% dof 16" ) << 0.99 << 16. << 2.9207816224251000;
    QTest::newRow( "95% dof 1e6" ) << 0.95 << 1e6 << 1.9599663568141066;

    // Closed-form solutions: dof 1 (Cauchy) k = tan( pi / 2 * confidence ),
    // dof 2 k = confidence * sqrt( 2 / ( 1 - confidence^2 ) )
    QTest::newRow( "95% dof 1" )
        << 0.95 << 1. << std::tan( 0.5 * 3.14159265358979323846 * 0.95 );
    QTest::newRow( "95% dof 2" )
        << 0.95 << 2. << 0.95 * std::sqrt( 2. / ( 1. - 0.95 * 0.95 ) );

    // Degrees of freedom below 0.5, see invCDFStandardStudentT_referenceValues
    QTest::newRow( "80% dof 0.1" ) << 0.8 << 0.1 << 1604425.7056665548;
    QTest::newRow( "95% dof 0.25" ) << 0.95 << 0.25 << 43640.149267980318;
    QTest::newRow( "95% dof 0.4" ) << 0.95 << 0.4 << 642.82724615652831;
}


void tst_distribution::coverageFactor_referenceValues() {
    QFETCH( double, confidence );
    QFETCH( double, dof );
    QFETCH( double, expected );

    const double value { Distribution::coverageFactor( confidence, dof ) };
    QVERIFY2(
        relativeClose( value, expected, relativeAccuracy ),
        qPrintable( QString::number( value, 'g', 17 ) )
    );
}


void tst_distribution::coverageFactor_limits() {
    QCOMPARE( Distribution::coverageFactor( 0., 10. ), 0. );
    QCOMPARE(
        Distribution::coverageFactor( 1., 10. ),
        std::numeric_limits<double>::infinity()
    );
    QVERIFY( qIsNaN( Distribution::coverageFactor( 0.95, 0. ) ) );
    QVERIFY( qIsNaN( Distribution::coverageFactor( 0.95, -1. ) ) );

    // Decreasing with increasing DOF towards the normal quantile
    double previous { Distribution::coverageFactor( 0.95, 0.1 ) };
    for (
        double dof : { 0.25, 0.4, 0.5, 1., 2., 5., 10., 100., 1e4, 1e6, 1e9 }
    ) {
        const double current { Distribution::coverageFactor( 0.95, dof ) };
        QVERIFY( current < previous );
        previous = current;
    }
    QVERIFY( previous > 1.9599639845400542 );
}


void tst_distribution::coverageFactor_nonIntegerDOF_isBetweenIntegerDOF() {
    // For non-integer DOF, the coverage factor lies between the values for
    // the neighbouring integer DOF, and the central probability of [-k, k]
    // equals the confidence
    for ( double confidence : { 0.6827, 0.95, 0.99 } ) {
        for ( double dof : { 0.5, 0.85, 1.6, 2.5, 16.7, 100.5 } ) {
            const double k { Distribution::coverageFactor( confidence, dof ) };
            QVERIFY( std::isfinite( k ) );
            const double lowerDOF { std::floor( dof ) };
            QVERIFY(
                lowerDOF == 0. ||
                k < Distribution::coverageFactor( confidence, lowerDOF )
            );
            QVERIFY(
                k > Distribution::coverageFactor( confidence, std::ceil( dof ) )
            );
            const double central {
                2. * Distribution::studentTCDF( k, dof ) - 1.
            };
            QVERIFY( relativeClose( central, confidence, 1e-12 ) );
        }
    }
}


QTEST_MAIN( tst_distribution )
#include "tst_distribution.moc"
