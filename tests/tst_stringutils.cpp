// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"
#include <QDir>
#include <QTest>
#include <QUrl>
#include <limits>
#include <cwctype>

// Unit tests for the StringUtils namespace, used throughout the Core and
// Orchestration layers to format numbers for display and CSV export.
class tst_stringutils : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void doubleToString_usesRequestedPrecision();
    void doubleToString_supportsFixedFormat();

    void addQuotes_wrapsInDoubleQuotes();
    void addQuotes_escapesEmbeddedQuotes();
    void addQuotes_emptyString();

    void contributionToPercentageString_formatsFraction();
    void contributionToPercentageString_nonFiniteYieldsDash();
    void contributionToPercentageString_respectsDecimals();

    void std_towlower_GreekSymbols();

    void urlToLocalPath_data();
    void urlToLocalPath();
    void urlToFileName_data();
    void urlToFileName();
};


void tst_stringutils::initTestCase() {
    StringUtils::setupLocale();
}


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


void tst_stringutils::std_towlower_GreekSymbols() {
    // Make sure std::towlower() handles Unicode
    // correctly
    QCOMPARE( std::towlower( L'A' ), L'a' );
    QCOMPARE( std::towlower( L'Δ' ), L'δ' );
    QCOMPARE( std::towlower( L'Σ' ), L'σ' );
}


void tst_stringutils::urlToLocalPath_data() {
    QTest::addColumn<QUrl>( "url" );
    QTest::addColumn<QString>( "expected" );

    // Local paths are shown with the native separators of the platform
    QTest::newRow( "local file with special characters" )
        << QUrl::fromLocalFile( "/home/user/My Projects/100% #1 résumé.sig" )
        << QDir::toNativeSeparators(
               "/home/user/My Projects/100% #1 résumé.sig"
           );
    QTest::newRow( "encoded characters" )
        << QUrl( "file:///home/user/My%20Projects/r%C3%A9sum%C3%A9%25.sig" )
        << QDir::toNativeSeparators( "/home/user/My Projects/résumé%.sig" );
    QTest::newRow( "network path" )
        << QUrl( "file://server/share/project.sig" )
        << QDir::toNativeSeparators( "//server/share/project.sig" );
#ifdef Q_OS_WINDOWS
    QTest::newRow( "drive letter" )
        << QUrl( "file:///C:/My%20Projects/project.sig" )
        << QString( "C:\\My Projects\\project.sig" );
#endif
    QTest::newRow( "not a local file" )
        << QUrl( "https://example.com/dir/project.sig" )
        << QString( "https://example.com/dir/project.sig" );
    QTest::newRow( "empty" ) << QUrl() << QString();
}


void tst_stringutils::urlToLocalPath() {
    QFETCH( QUrl, url );
    QFETCH( QString, expected );
    QCOMPARE( StringUtils::urlToLocalPath( url ), expected );
}


void tst_stringutils::urlToFileName_data() {
    QTest::addColumn<QUrl>( "url" );
    QTest::addColumn<QString>( "expected" );

    QTest::newRow( "local file with special characters" )
        << QUrl::fromLocalFile( "/home/user/My Projects/100% #1 résumé.sig" )
        << QString( "100% #1 résumé.sig" );
    QTest::newRow( "encoded characters" )
        << QUrl( "file:///home/user/My%20Projects/r%C3%A9sum%C3%A9%25.sig" )
        << QString( "résumé%.sig" );
    QTest::newRow( "network path" )
        << QUrl( "file://server/share/project.sig" )
        << QString( "project.sig" );
    QTest::newRow( "not a local file" )
        << QUrl( "https://example.com/dir/results.csv" )
        << QString( "results.csv" );
    QTest::newRow( "empty" ) << QUrl() << QString();
}


void tst_stringutils::urlToFileName() {
    QFETCH( QUrl, url );
    QFETCH( QString, expected );
    QCOMPARE( StringUtils::urlToFileName( url ), expected );
}


QTEST_MAIN( tst_stringutils )
#include "tst_stringutils.moc"
