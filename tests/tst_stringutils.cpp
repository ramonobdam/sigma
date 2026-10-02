// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"
#include <QTest>
#include <limits>
#include <string>

// Unit tests for the StringUtils namespace, used throughout the Core and
// Orchestration layers to format numbers for display and CSV export.
class tst_stringutils : public QObject {
    Q_OBJECT

private slots:
    void doubleToString_usesRequestedPrecision();
    void doubleToString_supportsFixedFormat();

    void addQuotes_wrapsInDoubleQuotes();
    void addQuotes_escapesEmbeddedQuotes();
    void addQuotes_emptyString();

    void contributionToPercentageString_formatsFraction();
    void contributionToPercentageString_nonFiniteYieldsDash();
    void contributionToPercentageString_respectsDecimals();

    void unicodeLess_greekSymbolOrder();
};


void tst_stringutils::doubleToString_usesRequestedPrecision() {
    // 'g' format, precision = number of significant digits
    QCOMPARE(
        StringUtils::doubleToString( 123.456789, 4 ),
        QString( "123.5" )
    );
    QCOMPARE(
        StringUtils::doubleToString( 0.0001234, 2 ),
        QString( "0.00012" )
    );
}


void tst_stringutils::doubleToString_supportsFixedFormat() {
    QCOMPARE(
        StringUtils::doubleToString( 3.14159, 2, 'f' ),
        QString( "3.14" )
    );
}


void tst_stringutils::addQuotes_wrapsInDoubleQuotes() {
    QCOMPARE( StringUtils::addQuotes( "abc" ), QString( "\"abc\"" ) );
}


void tst_stringutils::addQuotes_escapesEmbeddedQuotes() {
    // A double quote inside the string must be escaped for CSV by doubling it
    QCOMPARE(
        StringUtils::addQuotes( "he said \"hi\"" ),
        QString( "\"he said \"\"hi\"\"\"" )
    );
}


void tst_stringutils::addQuotes_emptyString() {
    QCOMPARE( StringUtils::addQuotes( "" ), QString( "\"\"" ) );
}


void tst_stringutils::contributionToPercentageString_formatsFraction() {
    QCOMPARE(
        StringUtils::contributionToPercentageString( 0.25 ),
        QString( "25.0%" )
    );
    QCOMPARE(
        StringUtils::contributionToPercentageString( 1. ),
        QString( "100.0%" )
    );
    QCOMPARE(
        StringUtils::contributionToPercentageString( 0. ),
        QString( "0.0%" )
    );
}


void tst_stringutils::contributionToPercentageString_nonFiniteYieldsDash() {
    double nan { std::numeric_limits<double>::quiet_NaN() };
    double inf { std::numeric_limits<double>::infinity() };
    QCOMPARE(
        StringUtils::contributionToPercentageString( nan ),
        QString( "-" )
    );
    QCOMPARE(
        StringUtils::contributionToPercentageString( inf ),
        QString( "-" )
    );
}


void tst_stringutils::contributionToPercentageString_respectsDecimals() {
    QCOMPARE(
        StringUtils::contributionToPercentageString( 0.12345, 3 ),
        QString( "12.345%" )
    );
}


void tst_stringutils::unicodeLess_greekSymbolOrder() {
    // Expected symbol order: Δ_S < Δ_b < δ
    const std::wstring deltaLower = L"δ";
    const std::wstring deltaSubB  = L"Δ_b";
    const std::wstring deltaSubS  = L"Δ_S";

    QVERIFY( StringUtils::unicodeLess( deltaSubS, deltaSubB ) );
    QVERIFY( StringUtils::unicodeLess( deltaSubB, deltaLower ) );

    QVERIFY( !StringUtils::unicodeLess( deltaLower, deltaSubB ) );
    QVERIFY( !StringUtils::unicodeLess( deltaSubB, deltaSubS ) );
}


QTEST_MAIN( tst_stringutils )
#include "tst_stringutils.moc"
