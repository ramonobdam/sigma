// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "inputparameter.h"
#include "outputparameter.h"
#include "samplebatch.h"
#include "samplewriter.h"
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <vector>

// Unit tests for SampleWriter: writes Monte Carlo samples (input values and
// output value per row) to a CSV file, and determines valid and unique CSV
// file names for the output parameters (used by --export-samples-dir).
class tst_samplewriter : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void fileBaseName_data();
    void fileBaseName();

    void uniqueFileNames_data();
    void uniqueFileNames();

    void write_commit_writesHeaderAndExactValues();
    void write_withoutCommit_writesNoFile();
    void write_withoutInputValues_fails();
    void open_notALocalFile_fails();

private:
    void createModel( OutputParameter &output );
    QStringList readLines( const QString &path );
};


void tst_samplewriter::initTestCase() {
    OutputParameter::setCollectVariables( true );
}


void tst_samplewriter::cleanup() {
    OutputParameter::clearModel();
    InputParameter::clearModel();
}


void tst_samplewriter::fileBaseName_data() {
    QTest::addColumn<QString>( "name" );
    QTest::addColumn<QString>( "expected" );

    QTest::newRow( "valid name" ) << "Y1" << "Y1";
    QTest::newRow( "invalid characters" )
        << "a<b>c:d\"e/f\\g|h?i*j" << "a_b_c_d_e_f_g_h_i_j";
    QTest::newRow( "control character" ) << "a\tb" << "a_b";
    QTest::newRow( "unicode is kept" ) << "Δp °C" << "Δp °C";
    QTest::newRow( "reserved name" ) << "CON" << "CON_";
    QTest::newRow( "reserved name lower case" ) << "nul" << "nul_";
    QTest::newRow( "reserved COM port" ) << "COM1" << "COM1_";
    QTest::newRow( "reserved LPT port" ) << "LPT9" << "LPT9_";
    QTest::newRow( "reserved name with extension" ) << "AUX.v2" << "AUX_.v2";
    QTest::newRow( "reserved name as prefix" ) << "CONSOLE" << "CONSOLE";
    QTest::newRow( "reserved name as suffix" ) << "XCON" << "XCON";
    QTest::newRow( "COM10 is not reserved" ) << "COM10" << "COM10";
}


void tst_samplewriter::fileBaseName() {
    QFETCH( QString, name );
    QFETCH( QString, expected );
    QCOMPARE( SampleWriter::fileBaseName( name ), expected );
}


void tst_samplewriter::uniqueFileNames_data() {
    QTest::addColumn<QStringList>( "names" );
    QTest::addColumn<QStringList>( "expected" );

    QTest::newRow( "no names" ) << QStringList {} << QStringList {};
    QTest::newRow( "unique names" )
        << QStringList { "Y", "Z" }
        << QStringList { "Y.csv", "Z.csv" };
    QTest::newRow( "duplicate after replacement" )
        << QStringList { "a/b", "a:b" }
        << QStringList { "a_b.csv", "a_b_2.csv" };
    QTest::newRow( "multiple duplicates in order" )
        << QStringList { "a/b", "a:b", "a*b" }
        << QStringList { "a_b.csv", "a_b_2.csv", "a_b_3.csv" };
    QTest::newRow( "case-insensitive duplicate" )
        << QStringList { "Y", "y" }
        << QStringList { "Y.csv", "y_2.csv" };
    QTest::newRow( "suffix doesn't take a later name" )
        << QStringList { "a/b", "a:b", "a_b_2" }
        << QStringList { "a_b.csv", "a_b_3.csv", "a_b_2.csv" };
    QTest::newRow( "suffix doesn't take an earlier name" )
        << QStringList { "a_b_2", "a/b", "a:b" }
        << QStringList { "a_b_2.csv", "a_b.csv", "a_b_3.csv" };
    QTest::newRow( "suffix doesn't take another name case-insensitively" )
        << QStringList { "a/b", "a:b", "A_B_2" }
        << QStringList { "a_b.csv", "a_b_3.csv", "A_B_2.csv" };
    QTest::newRow( "reserved name and its replacement" )
        << QStringList { "CON", "CON_" }
        << QStringList { "CON_.csv", "CON__2.csv" };
}


void tst_samplewriter::uniqueFileNames() {
    QFETCH( QStringList, names );
    QFETCH( QStringList, expected );

    const QStringList fileNames { SampleWriter::uniqueFileNames( names ) };
    QCOMPARE( fileNames, expected );

    // All file names must be unique, also when ignoring case
    QStringList lowerCase {};
    for ( const QString &fileName : fileNames ) {
        lowerCase.append( fileName.toLower() );
    }
    lowerCase.removeDuplicates();
    QCOMPARE( lowerCase.size(), fileNames.size() );
}


void tst_samplewriter::write_commit_writesHeaderAndExactValues() {
    OutputParameter output {};
    createModel( output );

    SampleBatch batch { &output, 100, 7 };
    QVERIFY( batch.generateSamples( true ) );

    QTemporaryDir dir {};
    QVERIFY( dir.isValid() );
    const QString path { dir.filePath( "samples.csv" ) };

    QStringList inputNames {};
    const QList<UncertaintyComponent> components { output.getComponents() };
    for ( const UncertaintyComponent &component : components ) {
        inputNames.append( component.getName() );
    }

    SampleWriter writer {};
    QVERIFY( writer.open( QUrl::fromLocalFile( path ), inputNames, "Y" ) );
    QVERIFY( writer.isOpen() );
    QCOMPARE( writer.getFileName(), QString( "samples.csv" ) );
    QVERIFY( writer.write( batch ) );
    QVERIFY( writer.commit() );
    QVERIFY( writer.getError().isEmpty() );

    const QStringList lines { readLines( path ) };
    QCOMPARE( lines.size(), 1 + 100 );
    QCOMPARE(
        lines.first(),
        QString( "\"%1\",\"%2\",\"Y\"" ).arg( inputNames[ 0 ], inputNames[ 1 ] )
    );

    // The values are written with full (round-trip) precision
    const std::vector<double> &inputs { batch.getInputValues() };
    const std::vector<double> &outputs { batch.getSamples() };
    for ( int i { 0 }; i < 100; ++i ) {
        const QStringList values { lines[ i + 1 ].split( ',' ) };
        QCOMPARE( values.size(), 3 );
        QCOMPARE( values[ 0 ].toDouble(), inputs[ 2 * i ] );
        QCOMPARE( values[ 1 ].toDouble(), inputs[ 2 * i + 1 ] );
        QCOMPARE( values[ 2 ].toDouble(), outputs[ i ] );
    }
}


void tst_samplewriter::write_withoutCommit_writesNoFile() {
    OutputParameter output {};
    createModel( output );

    SampleBatch batch { &output, 10, 7 };
    QVERIFY( batch.generateSamples( true ) );

    QTemporaryDir dir {};
    const QString path { dir.filePath( "samples.csv" ) };

    {
        SampleWriter writer {};
        QVERIFY( writer.open(
            QUrl::fromLocalFile( path ), { "X1", "X2" }, "Y"
        ) );
        QVERIFY( writer.write( batch ) );
        writer.cancel();
        QVERIFY( !writer.isOpen() );
    }
    QVERIFY( !QFile::exists( path ) );

    // Destroying an open writer without commit doesn't write the file either
    {
        SampleWriter writer {};
        QVERIFY( writer.open(
            QUrl::fromLocalFile( path ), { "X1", "X2" }, "Y"
        ) );
        QVERIFY( writer.write( batch ) );
    }
    QVERIFY( !QFile::exists( path ) );
}


void tst_samplewriter::write_withoutInputValues_fails() {
    OutputParameter output {};
    createModel( output );

    // Generated without storing the input values
    SampleBatch batch { &output, 10, 7 };
    QVERIFY( batch.generateSamples() );

    QTemporaryDir dir {};
    const QString path { dir.filePath( "samples.csv" ) };

    SampleWriter writer {};
    QVERIFY( writer.open( QUrl::fromLocalFile( path ), { "X1", "X2" }, "Y" ) );
    QVERIFY( !writer.write( batch ) );
    QVERIFY( !writer.getError().isEmpty() );
    QVERIFY( !writer.isOpen() );
    QVERIFY( !writer.commit() );
    QVERIFY( !QFile::exists( path ) );
}


void tst_samplewriter::open_notALocalFile_fails() {
    SampleWriter writer {};
    QVERIFY( !writer.open( QUrl( "https://example.com/samples.csv" ), {}, "Y" ) );
    QVERIFY( !writer.isOpen() );
    QVERIFY( writer.getError().contains( "is not a local file" ) );
}


void tst_samplewriter::createModel( OutputParameter &output ) {
    for ( const QString &name : { "X1", "X2" } ) {
        InputParameter parameter {};
        parameter.setName( name );
        parameter.setStdUncertainty( 1. );
        parameter.appendToModel();
    }
    output.setName( "Y" );
    output.setFormula( "X1 * X2" );
    output.compile();
    QVERIFY( output.getValid() );
}


QStringList tst_samplewriter::readLines( const QString &path ) {
    QFile file { path };
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        return {};
    }
    return QString::fromUtf8( file.readAll() ).split(
        '\n',
        Qt::SkipEmptyParts
    );
}


QTEST_MAIN( tst_samplewriter )
#include "tst_samplewriter.moc"
