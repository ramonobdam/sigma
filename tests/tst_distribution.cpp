// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include <QTest>
#include <QtNumeric>
#include <cmath>

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
    void invCDFStandardNormal_boundsAndMedian();
    void invCDFStandardNormal_isMonotonicallyIncreasing();

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
    void getInvCDF_none_isConstant();
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


void tst_distribution::getInvCDF_none_isConstant() {
    Distribution::InvCDF invCDF {
        Distribution::getInvCDF( Distribution::Type::none, 7., 3., 1, true )
    };
    // The standard uncertainty is ignored for a constant ('none') parameter
    QCOMPARE( invCDF( 0. ), 7. );
    QCOMPARE( invCDF( 1. ), 7. );
}


QTEST_MAIN( tst_distribution )
#include "tst_distribution.moc"
