// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "correlation.h"
#include "inputparameter.h"
#include "outputparameter.h"
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTest>
#include <QtNumeric>
#include <cmath>
#include <limits>

// Unit tests for OutputParameter::compile(): the core of the analytic
// uncertainty budget calculation (GUM JCGM 100:2008) — combined standard
// uncertainty, sensitivity coefficients, effective degrees of freedom
// (Welch-Satterthwaite), coverage factor and expanded uncertainty.
//
// The GUM H.1 "End-gauge calibration" case is used as a golden reference: the
// expected numbers were taken from demo_projects/CSV/
// GUM_JCGM_100_H1_End-gauge_calibration.csv, produced by the Sigma CLI for
// that same demo project.
class tst_outputparameter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void compile_emptyFormula_isInvalid();
    void compile_syntaxError_isInvalid();
    void compile_invalidNominalValue_isInvalid();
    void compile_constantFormula_isValidWithoutComponents();

    void compile_simpleSum_combinedUncertaintyIsRootSumOfSquares();
    void compile_correlatedInputs_increaseCombinedUncertainty();
    void compile_negativeCorrelation_decreasesCombinedUncertainty();
    void compile_constantDistribution_doesNotContributeToUncertainty();

    void compile_endGaugeCalibration_matchesGumReferenceValues();

    void equality_dependsOnlyOnFormulaAndConfidence();
    void setConfidence_isClampedToValidRange();
    void setLocked_emitsLockedChanged();

private:
    static InputParameter *addInput(
        const QString &name,
        double nominal,
        double std,
        Distribution::Type distribution = Distribution::Type::normal,
        bool dofInfinite = true,
        int dof = 1
    );
    static UncertaintyComponent findComponent(
        const OutputParameter &output,
        const QString &name
    );
};


InputParameter * tst_outputparameter::addInput(
    const QString &name,
    double nominal,
    double std,
    Distribution::Type distribution,
    bool dofInfinite,
    int dof
) {
    InputParameter parameter {};
    parameter.setName( name );
    parameter.setNominalValue( nominal );
    parameter.setStdUncertainty( std );
    parameter.setDistribution( distribution );
    parameter.setDOFInfinite( dofInfinite );
    if ( !dofInfinite ) {
        parameter.setDOF( dof );
    }
    return parameter.appendToModel();
}


UncertaintyComponent tst_outputparameter::findComponent(
    const OutputParameter &output,
    const QString &name
) {
    const QList<UncertaintyComponent> &components { output.getComponents() };
    for ( const UncertaintyComponent &component : components ) {
        if ( component.getName() == name ) {
            return component;
        }
    }
    return UncertaintyComponent {};
}


void tst_outputparameter::initTestCase() {
    // ExprTk must be told to collect the referenced variables so that
    // OutputParameter::compile() can build the uncertainty components
    OutputParameter::setCollectVariables( true );
}


void tst_outputparameter::cleanup() {
    OutputParameter::clearModel();
    Correlation::clearModel();
    InputParameter::clearModel();
}


void tst_outputparameter::compile_emptyFormula_isInvalid() {
    OutputParameter output {};
    output.setFormula( "" );
    output.compile();

    QVERIFY( !output.getValid() );
    QCOMPARE( output.getError(), QString( "Measurement function is empty" ) );
}


void tst_outputparameter::compile_syntaxError_isInvalid() {
    OutputParameter output {};
    output.setFormula( "X1 +" );
    output.compile();

    QVERIFY( !output.getValid() );
    QVERIFY( !output.getError().isEmpty() );
}


void tst_outputparameter::compile_invalidNominalValue_isInvalid() {
    OutputParameter output {};
    output.setFormula( "sqrt(-1)" );
    output.compile();

    QVERIFY( !output.getValid() );
    QVERIFY( output.getError().contains( "Nominal output value is invalid" ) );
}


void tst_outputparameter::compile_constantFormula_isValidWithoutComponents() {
    OutputParameter output {};
    output.setFormula( "2 + 3" );
    output.compile();

    QVERIFY( output.getValid() );
    QCOMPARE( output.getNumberOfComponents(), 0 );
    QCOMPARE( output.getNominalValue(), 5. );
    QCOMPARE( output.getError(), QString( "No input parameters detected" ) );
}


void tst_outputparameter::compile_simpleSum_combinedUncertaintyIsRootSumOfSquares(
) {
    addInput( "X1", 10., 3. );
    addInput( "X2", 20., 4. );

    OutputParameter output {};
    output.setFormula( "X1 + X2" );
    output.compile();

    QVERIFY( output.getValid() );
    QCOMPARE( output.getNumberOfComponents(), 2 );
    QCOMPARE( output.getNominalValue(), 30. );

    // Both sensitivities are 1, so u_c = sqrt(3^2 + 4^2) = 5
    QVERIFY( qAbs( output.getCombinedStdUncertainty() - 5. ) < 1e-6 );

    UncertaintyComponent x1 { findComponent( output, "X1" ) };
    UncertaintyComponent x2 { findComponent( output, "X2" ) };
    QVERIFY( qAbs( x1.getSensitivity() - 1. ) < 1e-6 );
    QVERIFY( qAbs( x2.getSensitivity() - 1. ) < 1e-6 );

    // Both inputs have infinite DOF, so the output has infinite DOF too
    QCOMPARE( output.getEffectiveDOF(), std::numeric_limits<int>::max() );
}


void tst_outputparameter::compile_correlatedInputs_increaseCombinedUncertainty(
) {
    InputParameter *x1 { addInput( "X1", 0., 1. ) };
    QVERIFY( x1 );
    InputParameter *x2 { addInput( "X2", 0., 1. ) };
    QVERIFY( x2 );

    Correlation correlation { nullptr, x1->getId(), x2->getId(), 0.5 };
    QVERIFY( correlation.appendToModel() != nullptr );

    OutputParameter output {};
    output.setFormula( "X1 + X2" );
    output.compile();

    QVERIFY( output.getValid() );
    // u_c^2 = 1^2*1^2 + 1^2*1^2 + 2*1*1*1*1*0.5 = 3  ->  u_c = sqrt(3)
    QVERIFY( qAbs( output.getCombinedStdUncertainty() - std::sqrt( 3. ) ) <
             1e-6 );

    // Without correlation the combined uncertainty would only be sqrt(2)
    QVERIFY( output.getCombinedStdUncertainty() > std::sqrt( 2. ) );
}


void tst_outputparameter::compile_negativeCorrelation_decreasesCombinedUncertainty(
) {
    InputParameter *x1 { addInput( "X1", 0., 1. ) };
    QVERIFY( x1 );
    InputParameter *x2 { addInput( "X2", 0., 1. ) };
    QVERIFY( x2 );

    Correlation correlation { nullptr, x1->getId(), x2->getId(), -0.5 };
    QVERIFY( correlation.appendToModel() != nullptr );

    OutputParameter output {};
    output.setFormula( "X1 + X2" );
    output.compile();

    QVERIFY( output.getValid() );
    // u_c^2 = 1 + 1 - 2*1*1*1*1*0.5 = 1 -> u_c = 1
    QVERIFY( qAbs( output.getCombinedStdUncertainty() - 1. ) < 1e-6 );
    QVERIFY( output.getCombinedStdUncertainty() < std::sqrt( 2. ) );
}


void tst_outputparameter::compile_constantDistribution_doesNotContributeToUncertainty(
) {
    addInput( "X1", 5., 2. );
    addInput( "K", 100., 99., Distribution::Type::none );

    OutputParameter output {};
    output.setFormula( "X1 + K" );
    output.compile();

    QVERIFY( output.getValid() );
    QCOMPARE( output.getNominalValue(), 105. );
    // K is a constant ('none' distribution): its component value is zero, so
    // it must not contribute to the combined uncertainty
    QVERIFY( qAbs( output.getCombinedStdUncertainty() - 2. ) < 1e-6 );
}


void tst_outputparameter::compile_endGaugeCalibration_matchesGumReferenceValues(
) {
    // JCGM 100:2008, Annex H.1 — End-gauge calibration
    addInput( "l_S", 50000623., 25., Distribution::Type::normal, false, 18 );
    addInput( "d", 215., 9.7, Distribution::Type::normal, false, 25 );
    addInput(
        "alpha_S", 1.15e-05, 1.2e-06, Distribution::Type::uniform, true
    );
    addInput( "theta", 0.1, 0.41, Distribution::Type::normal, true );
    addInput( "delta_alpha", 0., 5.8e-07, Distribution::Type::uniform, false,
               50 );
    addInput( "delta_theta", 0., 0.029, Distribution::Type::uniform, false,
               2 );

    OutputParameter output {};
    output.setFormula(
        "l_S + d - l_S * ( delta_alpha * theta + alpha_S * delta_theta )"
    );
    output.setConfidence( 0.99 );
    output.compile();

    QVERIFY2(
        output.getValid(),
        qPrintable( "compile() failed: " + output.getError() )
    );
    QCOMPARE( output.getNumberOfComponents(), 6 );

    // At the nominal values, delta_alpha = delta_theta = 0, so the
    // correction term vanishes exactly
    QVERIFY( qAbs( output.getNominalValue() - 50000838. ) < 1e-6 );

    // Reference values from the demo project CSV export
    QVERIFY(
        qAbs( output.getCombinedStdUncertainty() - 31.71062853 ) < 1e-4
    );
    QCOMPARE( output.getEffectiveDOF(), 16 );
    QVERIFY( qAbs( output.getCoverageFactor() - 2.920781622 ) < 1e-4 );
    QVERIFY( qAbs( output.getExpandedUncertainty() - 92.61982103 ) < 1e-3 );

    // Per-component sensitivities from the CSV export
    UncertaintyComponent lS { findComponent( output, "l_S" ) };
    QVERIFY( qAbs( lS.getSensitivity() - 1. ) < 1e-6 );

    UncertaintyComponent d { findComponent( output, "d" ) };
    QVERIFY( qAbs( d.getSensitivity() - 1.000012307 ) < 1e-6 );

    UncertaintyComponent deltaTheta { findComponent( output, "delta_theta" ) };
    QVERIFY( qAbs( deltaTheta.getSensitivity() - ( -575.0060081 ) ) < 1e-2 );

    UncertaintyComponent alphaS { findComponent( output, "alpha_S" ) };
    QVERIFY( qAbs( alphaS.getSensitivity() ) < 1e-6 );  // ~0
}


void tst_outputparameter::equality_dependsOnlyOnFormulaAndConfidence() {
    OutputParameter a {};
    a.setName( "Y1" );
    a.setUnit( "mm" );
    a.setFormula( "X1 + X2" );
    a.setConfidence( 0.95 );

    OutputParameter b { a };
    b.setName( "Y2" );      // different name
    b.setUnit( "cm" );      // different unit
    QVERIFY( a == b );

    OutputParameter c { a };
    c.setConfidence( 0.99 );
    QVERIFY( a != c );

    OutputParameter d { a };
    d.setFormula( "X1 - X2" );
    QVERIFY( a != d );
}


void tst_outputparameter::setConfidence_isClampedToValidRange() {
    OutputParameter output {};

    QTest::ignoreMessage(
        QtCriticalMsg,
        QRegularExpression(
            "Invalid level of confidence value of '1.5' set to '0.999'"
        )
    );
    output.setConfidence( 1.5 );          // above maximum (0.999)
    QCOMPARE( output.getConfidence(), 0.999 );

    QTest::ignoreMessage(
        QtCriticalMsg,
        QRegularExpression(
            "Invalid level of confidence value of '-0.5' set to '0'"
        )
    );
    output.setConfidence( -0.5 );         // below minimum (0.)
    QCOMPARE( output.getConfidence(), 0. );

    output.setConfidence( 0.95 );
    QCOMPARE( output.getConfidence(), 0.95 );
}


void tst_outputparameter::setLocked_emitsLockedChanged() {
    OutputParameter output {};
    QSignalSpy spy { &output, &OutputParameter::lockedChanged };

    output.setLocked( true );
    QCOMPARE( spy.count(), 1 );
    QVERIFY( output.getLocked() );

    output.setLocked( false );
    QCOMPARE( spy.count(), 2 );
    QVERIFY( !output.getLocked() );
}


QTEST_MAIN( tst_outputparameter )
#include "tst_outputparameter.moc"
