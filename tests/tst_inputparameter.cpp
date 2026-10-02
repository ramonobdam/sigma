// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "inputparameter.h"
#include <QSignalSpy>
#include <QTest>
#include <QtNumeric>

// Unit tests for InputParameter: the class that stores an input quantity's
// nominal value, standard uncertainty and probability distribution, and
// manages the static model + ExprTk symbol table shared by all input
// parameters and referenced by output parameter measurement functions.
class tst_inputparameter : public QObject {
    Q_OBJECT

private slots:
    void cleanup();

    void defaultConstruction_hasExpectedDefaults();
    void setName_trimsWhitespace();
    void setUnit_trimsWhitespace();
    void setStdUncertainty_negativeIsClampedToAbsoluteValue();
    void setDOF_isClampedToValidRange();
    void setDistribution_string_validAndInvalid();
    void getInvCDF_dispatchesToDistribution();
    void getDOFAsString_infiniteVersusFinite();

    void jsonRoundTrip();

    void validName_rejectsReservedAndDuplicateNames();
    void appendToModel_addsToModelAndSymbolTable();
    void remove_removesFromModelAndSymbolTable();
    void update_changesNameInModelAndSymbolTable();

    void equality_ignoresNameCaseAndUnit();
    void inputParameterIsConstant_onlyForNoneDistribution();
};


void tst_inputparameter::cleanup() {
    // Statics are shared across all test functions in this process — reset
    // both the model and the ExprTk symbol table between tests
    InputParameter::clearModel();
}


void tst_inputparameter::defaultConstruction_hasExpectedDefaults() {
    InputParameter parameter {};
    QCOMPARE( parameter.getName(), QString() );
    QCOMPARE( parameter.getUnit(), QString() );
    QCOMPARE( parameter.getNominalValue(), 0. );
    QCOMPARE( parameter.getStdUncertainty(), 1. );
    QCOMPARE( parameter.getDistribution(), Distribution::Type::normal );
    QVERIFY( parameter.getDOFInfinite() );
    QCOMPARE( parameter.getDOF(), 1 );
    QVERIFY( !parameter.getLocked() );
}


void tst_inputparameter::setName_trimsWhitespace() {
    InputParameter parameter {};
    parameter.setName( "  X1  " );
    QCOMPARE( parameter.getName(), QString( "X1" ) );
}

void tst_inputparameter::setUnit_trimsWhitespace() {
    InputParameter parameter {};
    parameter.setUnit( "  mm  " );
    QCOMPARE( parameter.getUnit(), QString( "mm" ) );
}


void tst_inputparameter::setStdUncertainty_negativeIsClampedToAbsoluteValue() {
    InputParameter parameter {};
    parameter.setStdUncertainty( -5. );
    QCOMPARE( parameter.getStdUncertainty(), 5. );

    parameter.setStdUncertainty( 2.5 );
    QCOMPARE( parameter.getStdUncertainty(), 2.5 );
}


void tst_inputparameter::setDOF_isClampedToValidRange() {
    InputParameter parameter {};

    parameter.setDOF( 0 );          // below minimum (1)
    QCOMPARE( parameter.getDOF(), 1 );

    parameter.setDOF( 42 );         // within range
    QCOMPARE( parameter.getDOF(), 42 );

    parameter.setDOF( 10000000 );   // above maximum (1000000)
    QCOMPARE( parameter.getDOF(), 1000000 );
}


void tst_inputparameter::setDistribution_string_validAndInvalid() {
    InputParameter parameter {};

    parameter.setDistribution( "uniform" );
    QCOMPARE( parameter.getDistribution(), Distribution::Type::uniform );
    QCOMPARE( parameter.getDistributionAsString(), QString( "uniform" ) );

    // An unrecognized distribution string resets the distribution to 'none'
    // (and logs a critical warning)
    parameter.setDistribution( "not-a-distribution" );
    QCOMPARE( parameter.getDistribution(), Distribution::Type::none );
}


void tst_inputparameter::getInvCDF_dispatchesToDistribution() {
    InputParameter parameter {};
    parameter.setNominalValue( 10. );
    parameter.setStdUncertainty( 2. );
    parameter.setDistribution( Distribution::Type::normal );

    // A known reference value: Phi^-1(0.975) ~= 1.959964 for standard
    // normal; general normal distribution: x = mean + Phi^-1 × stdDev
    Distribution::InvCDF invCDF { parameter.getInvCDF() };
    QVERIFY( qAbs( invCDF( 0.975 ) - ( 10. + 2. * 1.959964 ) ) < 1e-4 );
}


void tst_inputparameter::getDOFAsString_infiniteVersusFinite() {
    InputParameter parameter {};
    parameter.setDOFInfinite( true );
    QCOMPARE( parameter.getDOFAsString(), QString( "infinite" ) );

    parameter.setDOFInfinite( false );
    parameter.setDOF( 15 );
    QCOMPARE( parameter.getDOFAsString(), QString( "15" ) );
}


void tst_inputparameter::jsonRoundTrip() {
    InputParameter original {};
    original.setName( "X1" );
    original.setUnit( "mm" );
    original.setNominalValue( 12.5 );
    original.setStdUncertainty( 0.3 );
    original.setDistribution( Distribution::Type::triangular );
    original.setDOFInfinite( false );
    original.setDOF( 7 );

    QJsonObject json { original.toJson() };
    QCOMPARE( json[ "name" ].toString(), QString( "X1" ) );
    QCOMPARE( json[ "unit" ].toString(), QString( "mm" ) );
    QCOMPARE( json[ "nominalValue" ].toDouble(), 12.5 );
    QCOMPARE( json[ "stdUncertainty" ].toDouble(), 0.3 );
    QCOMPARE( json[ "distribution" ].toString(), QString( "triangular" ) );
    QCOMPARE( json[ "DOFInfinite" ].toBool(), false );
    QCOMPARE( json[ "DOF" ].toInt(), 7 );

    InputParameter restored {};
    restored.updateFromJson( json );
    QCOMPARE( restored.getName(), original.getName() );
    QCOMPARE( restored.getUnit(), original.getUnit() );
    QCOMPARE( restored.getNominalValue(), original.getNominalValue() );
    QCOMPARE( restored.getStdUncertainty(), original.getStdUncertainty() );
    QCOMPARE( restored.getDistribution(), original.getDistribution() );
    QCOMPARE( restored.getDOFInfinite(), original.getDOFInfinite() );
    QCOMPARE( restored.getDOF(), original.getDOF() );
    QCOMPARE( restored.getId(), original.getId() );
    QVERIFY( restored == original );
}


void tst_inputparameter::validName_rejectsReservedAndDuplicateNames() {
    // Reserved mathematical constants cannot be used as a parameter name
    QVERIFY( !InputParameter::validName( "pi" ) );
    QVERIFY( !InputParameter::validName( "epsilon" ) );
    QVERIFY( !InputParameter::validName( "inf" ) );

    QVERIFY( InputParameter::validName( "X1" ) );

    InputParameter parameter {};
    parameter.setName( "X1" );
    QVERIFY( parameter.appendToModel() != nullptr );

    // The name is now taken
    QVERIFY( !InputParameter::validName( "X1" ) );
    // ... but is still considered valid for the currently selected row
    QVERIFY( InputParameter::validName( "X1", true ) );
}


void tst_inputparameter::appendToModel_addsToModelAndSymbolTable() {
    InputParameter parameter {};
    parameter.setName( "X1" );

    InputParameter *added { parameter.appendToModel() };
    QVERIFY( added != nullptr );
    QCOMPARE( InputParameter::getAll().size(), 1 );
    QCOMPARE( InputParameter::getByName( "X1" ), added );
    QCOMPARE( InputParameter::getById( added->getId() ), added );
    QCOMPARE( InputParameter::getRowIndex( added->getId() ), 0 );

    // The name is now registered as a valid ExprTk symbol
    QVERIFY( InputParameter::getSymbolTable().symbol_exists( L"X1" ) );

    // An invalid name (already a reserved constant) cannot be appended
    InputParameter invalid {};
    invalid.setName( "pi" );
    QCOMPARE( invalid.appendToModel(), nullptr );
    QCOMPARE( InputParameter::getAll().size(), 1 );
}


void tst_inputparameter::remove_removesFromModelAndSymbolTable() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    InputParameter *added { parameter.appendToModel() };
    QUuid id { added->getId() };

    QVERIFY( InputParameter::remove( id ) );
    QCOMPARE( InputParameter::getAll().size(), 0 );
    QVERIFY( !InputParameter::getSymbolTable().symbol_exists( L"X1" ) );
    QCOMPARE( InputParameter::getById( id ), nullptr );
}


void tst_inputparameter::update_changesNameInModelAndSymbolTable() {
    InputParameter parameter {};
    parameter.setName( "X1" );
    InputParameter *added { parameter.appendToModel() };
    QUuid id { added->getId() };

    InputParameter updated { *added };
    updated.setName( "X2" );
    updated.setNominalValue( 99. );

    QVERIFY( InputParameter::update( id, &updated ) );

    InputParameter *result { InputParameter::getById( id ) };
    QVERIFY( result != nullptr );
    QCOMPARE( result->getName(), QString( "X2" ) );
    QCOMPARE( result->getNominalValue(), 99. );

    QVERIFY( !InputParameter::getSymbolTable().symbol_exists( L"X1" ) );
    QVERIFY( InputParameter::getSymbolTable().symbol_exists( L"X2" ) );
}


void tst_inputparameter::equality_ignoresNameCaseAndUnit() {
    InputParameter a {};
    a.setName( "X1" );
    a.setUnit( "mm" );
    a.setNominalValue( 1. );
    a.setStdUncertainty( 0.1 );

    InputParameter b { a };
    b.setName( "x1" );      // different capitalization
    b.setUnit( "cm" );      // different unit

    QVERIFY( a == b );

    InputParameter c { a };
    c.setDistribution( Distribution::Type::uniform );
    QVERIFY( a != c );
}


void tst_inputparameter::inputParameterIsConstant_onlyForNoneDistribution() {
    InputParameter parameter {};
    parameter.setName( "K" );
    parameter.setDistribution( Distribution::Type::none );
    parameter.appendToModel();

    QVERIFY( InputParameter::inputParameterIsConstant( "K" ) );

    InputParameter normalParam {};
    normalParam.setName( "X1" );
    normalParam.setDistribution( Distribution::Type::normal );
    normalParam.appendToModel();

    QVERIFY( !InputParameter::inputParameterIsConstant( "X1" ) );
    QVERIFY( !InputParameter::inputParameterIsConstant( "does-not-exist" ) );
}


QTEST_MAIN( tst_inputparameter )
#include "tst_inputparameter.moc"
