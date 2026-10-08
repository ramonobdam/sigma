// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "uncertaintycomponent.h"
#include <QString>
#include <QTest>
#include <QtNumeric>
#include <cmath>

// Unit tests for UncertaintyComponent: the class that links an InputParameter
// to an OutputParameter's measurement function and calculates the
// sensitivity coefficient (partial derivative), component value and
// correlation contribution used to build the uncertainty budget (GUM
// JCGM 100:2008).
class tst_uncertaintycomponent : public QObject {
    Q_OBJECT

private slots:
    void cleanup();

    void forwardsBasicPropertiesFromInputParameter();
    void nullInputParameter_returnsSafeDefaults();

    void getEvaluationValues_nonZeroNominal();
    void getEvaluationValues_zeroNominalFallsBackToFixedStep();

    void calculateSensitivity_matchesAnalyticDerivativeForCubic();

    void getComponentValue_isZeroForConstantDistribution();
    void getComponentValue_isAbsoluteSensitivityTimesUncertainty();

    void getCorrelationValue_usesRegisteredCorrelation();
    void getCorrelationValue_isZeroWithoutCorrelation();

    void getWelchSatterthwaiteTerms_finiteDofSimpleCase();
    void getWelchSatterthwaiteTerms_finiteDofCorrelatedCase();
    void getWelchSatterthwaiteTerms_isZeroForInfiniteDof();

    void addCorrelatedComponent_doesNotAddDuplicates();
};


void tst_uncertaintycomponent::cleanup() {
    Correlation::clearModel();
    InputParameter::clearModel();
}


void tst_uncertaintycomponent::forwardsBasicPropertiesFromInputParameter() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    parameter.setUnit( "mm" );
    parameter.setNominalValue( 5. );
    parameter.setStdUncertainty( 0.5 );
    parameter.setDistribution( Distribution::Type::normal );
    parameter.setDOFInfinite( false );
    parameter.setDOF( 9 );

    UncertaintyComponent component { &parameter };
    QCOMPARE( component.getName(), QString( "X1" ) );
    QCOMPARE( component.getUnit(), QString( "mm" ) );
    QCOMPARE( component.getNominalValue(), 5. );
    QCOMPARE( component.getStdUncertainty(), 0.5 );
    QCOMPARE( component.getDistribution(), Distribution::Type::normal );
    QVERIFY( !component.getDOFInfinite() );
    QCOMPARE( component.getDOF(), 9 );
    QCOMPARE( component.getInputParameter(), &parameter );
    QCOMPARE( component.getInputParameterId(), parameter.getId() );
}


void tst_uncertaintycomponent::nullInputParameter_returnsSafeDefaults() {
    UncertaintyComponent component {};
    QCOMPARE( component.getName(), QString() );
    QCOMPARE( component.getNominalValue(), 0. );
    QCOMPARE( component.getStdUncertainty(), 0. );
    QCOMPARE( component.getDistribution(), Distribution::Type::none );
    QVERIFY( component.getDOFInfinite() );
    QCOMPARE( component.getDOF(), 1 );
    QCOMPARE( component.getComponentValue(), 0. );
    QCOMPARE( component.getCorrelationValue(), 0. );
    QCOMPARE( component.getEvaluationValues().size(), 0 );
}


void tst_uncertaintycomponent::getEvaluationValues_nonZeroNominal() {
    InputParameter parameter {};
    parameter.setNominalValue( 100. );

    UncertaintyComponent component { &parameter };
    QList<double> values { component.getEvaluationValues() };
    QCOMPARE( values.size(), 4 );

    const double stepSize { 100. * 1e-6 };
    QVERIFY( qAbs( values[ 0 ] - ( 100. + 2. * stepSize ) ) < 1e-9 );
    QVERIFY( qAbs( values[ 1 ] - ( 100. + 1. * stepSize ) ) < 1e-9 );
    QVERIFY( qAbs( values[ 2 ] - ( 100. - 1. * stepSize ) ) < 1e-9 );
    QVERIFY( qAbs( values[ 3 ] - ( 100. - 2. * stepSize ) ) < 1e-9 );
}


void tst_uncertaintycomponent::getEvaluationValues_zeroNominalFallsBackToFixedStep(
) {
    InputParameter parameter {};
    parameter.setNominalValue( 0. );

    UncertaintyComponent component { &parameter };
    QList<double> values { component.getEvaluationValues() };
    QCOMPARE( values.size(), 4 );

    const double stepSize { 1e-6 };  // fixed fallback step
    QVERIFY( qAbs( values[ 0 ] - 2. * stepSize ) < 1e-12 );
    QVERIFY( qAbs( values[ 1 ] - 1. * stepSize ) < 1e-12 );
    QVERIFY( qAbs( values[ 2 ] + 1. * stepSize ) < 1e-12 );
    QVERIFY( qAbs( values[ 3 ] + 2. * stepSize ) < 1e-12 );
}


void tst_uncertaintycomponent::calculateSensitivity_matchesAnalyticDerivativeForCubic(
) {
    // The 4-point stencil (-f(x+2h)+8f(x+h)-8f(x-h)+f(x-2h)) / (12h) is exact
    // for polynomials up to degree 4 — used here with f(x) = x^3, x0 = 2,
    // whose exact derivative is 3*x0^2 = 12
    UncertaintyComponent component {};
    const double h { 0.01 };
    auto f = []( double x ) { return x * x * x; };
    QList<double> outputValues {
        f( 2. + 2. * h ),
        f( 2. + 1. * h ),
        f( 2. - 1. * h ),
        f( 2. - 2. * h )
    };

    component.calculateSensitivity( h, outputValues );
    QVERIFY( qAbs( component.getSensitivity() - 12. ) < 1e-9 );
}


void tst_uncertaintycomponent::getComponentValue_isZeroForConstantDistribution(
) {
    InputParameter parameter {};
    parameter.setDistribution( Distribution::Type::none );
    parameter.setStdUncertainty( 5. );

    UncertaintyComponent component { &parameter };
    component.setSentitivity( 2. );
    QCOMPARE( component.getComponentValue(), 0. );
}


void tst_uncertaintycomponent::getComponentValue_isAbsoluteSensitivityTimesUncertainty(
) {
    InputParameter parameter {};
    parameter.setDistribution( Distribution::Type::normal );
    parameter.setStdUncertainty( 4. );

    UncertaintyComponent component { &parameter };
    component.setSentitivity( -3. );
    QVERIFY( qAbs( component.getComponentValue() - 12. ) < 1e-12 );
}


void tst_uncertaintycomponent::getCorrelationValue_usesRegisteredCorrelation() {
    InputParameter a {};
    a.setName( "A" );
    a.setDistribution( Distribution::Type::normal );
    a.setStdUncertainty( 2. );
    InputParameter *paramA { a.appendToModel() };
    QVERIFY( paramA );

    InputParameter b {};
    b.setName( "B" );
    b.setDistribution( Distribution::Type::normal );
    b.setStdUncertainty( 3. );
    InputParameter *paramB { b.appendToModel() };
    QVERIFY( paramB );

    Correlation correlation { nullptr, paramA->getId(), paramB->getId(), 0.5 };
    QVERIFY( correlation.appendToModel() );

    UncertaintyComponent componentA { paramA };
    componentA.setSentitivity( 1. );
    UncertaintyComponent componentB { paramB };
    componentB.setSentitivity( 1. );

    componentA.addCorrelatedComponent( &componentB );

    // correlationValue = c_A * c_B * u_A * u_B * r = 1*1*2*3*0.5 = 3
    QVERIFY( qAbs( componentA.getCorrelationValue() - 3. ) < 1e-12 );
}


void tst_uncertaintycomponent::getCorrelationValue_isZeroWithoutCorrelation() {
    InputParameter parameter {};
    parameter.setDistribution( Distribution::Type::normal );
    parameter.setStdUncertainty( 1. );

    UncertaintyComponent component { &parameter };
    component.setSentitivity( 1. );
    // No correlated components have been added
    QCOMPARE( component.getCorrelationValue(), 0. );
}


void tst_uncertaintycomponent::getWelchSatterthwaiteTerms_finiteDofSimpleCase() {
    InputParameter parameter {};
    parameter.setDistribution( Distribution::Type::normal );
    parameter.setStdUncertainty( 2. );
    parameter.setDOFInfinite( false );
    parameter.setDOF( 10 );

    UncertaintyComponent component { &parameter };
    component.setSentitivity( 3. );

    // No correlated components: term = (c*u)^4 / v = (3*2)^4 / 10 = 1296/10
    double expected { std::pow( 3. * 2., 4 ) / 10. };
    QVERIFY(
        qAbs( component.getWelchSatterthwaiteTerms() - expected ) < 1e-9
    );
}


void tst_uncertaintycomponent::getWelchSatterthwaiteTerms_finiteDofCorrelatedCase(
) {
    // Define two correlated InputParameters
    InputParameter a {};
    a.setName( "A" );
    a.setDistribution( Distribution::Type::normal );
    a.setStdUncertainty( 2. );
    a.setDOFInfinite( false );
    a.setDOF( 10 );
    InputParameter *paramA { a.appendToModel() };
    QVERIFY( paramA );

    InputParameter b {};
    b.setName( "B" );
    b.setDistribution( Distribution::Type::normal );
    b.setStdUncertainty( 3. );
    b.setDOFInfinite( false );
    b.setDOF( 33 );
    InputParameter *paramB { b.appendToModel() };
    QVERIFY( paramB );

    Correlation correlation { nullptr, paramA->getId(), paramB->getId(), 0.5 };
    QVERIFY( correlation.appendToModel() );

    UncertaintyComponent componentA { paramA };
    componentA.setSentitivity( 3. );
    UncertaintyComponent componentB { paramB };
    componentB.setSentitivity( 4. );

    componentA.addCorrelatedComponent( &componentB );

    // Variance term
    double expected { std::pow( 3. * 2., 4 ) / 10. };

    // Correlation terms
    double term {};
    // First term: (r_ij*c_i*c_j*u_i* u_j)^2 * ((v_i+v_j+0.5)/(v_i*v_j))/2
    term = std::pow( 0.5 * 3. * 4. * 2. * 3., 2 );
    term *= ( ( 10. + 33. + 0.5 ) / ( 10. * 33.) ) / 2.;
    expected += term;

    // Second term: (r_ij*c_i*c_j*u_i*u_j)*((c_i*u_i)^2/v_i + (c_i*u_i)^2/v_j)
    term = 0.5 * 3. * 4. * 2. * 3.;
    term *= ( std::pow( 3. * 2., 2 ) / 10. + std::pow( 4. * 3., 2 ) / 33. );
    expected += term;

    QVERIFY(
        qAbs( componentA.getWelchSatterthwaiteTerms() - expected ) < 1e-9
    );
}


void tst_uncertaintycomponent::getWelchSatterthwaiteTerms_isZeroForInfiniteDof(
) {
    InputParameter parameter {};
    parameter.setDistribution( Distribution::Type::normal );
    parameter.setStdUncertainty( 2. );
    parameter.setDOFInfinite( true );

    UncertaintyComponent component { &parameter };
    component.setSentitivity( 3. );
    QCOMPARE( component.getWelchSatterthwaiteTerms(), 0. );
}


void tst_uncertaintycomponent::addCorrelatedComponent_doesNotAddDuplicates() {
    InputParameter a {};
    a.setName( "A" );
    InputParameter b {};
    b.setName( "B" );

    UncertaintyComponent componentA { &a };
    UncertaintyComponent componentB { &b };

    componentA.addCorrelatedComponent( &componentB );
    componentA.addCorrelatedComponent( &componentB );  // duplicate
    componentA.addCorrelatedComponent( nullptr );       // ignored

    QCOMPARE( componentA.getCorrelatedComponents().size(), 1 );
}


QTEST_MAIN( tst_uncertaintycomponent )
#include "tst_uncertaintycomponent.moc"
