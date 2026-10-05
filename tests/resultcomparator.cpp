// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "resultcomparator.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <cmath>

namespace {
    // Matches integer, decimal and scientific notation numbers
    const QRegularExpression sNumberExpression {
        "[-+]?(?:\\d+\\.?\\d*|\\.\\d+)(?:[eE][-+]?\\d+)?"
    };
}


ResultComparator::ResultComparator( const Tolerance &tolerance )
    :   mTolerance { tolerance },
        mDifferences {},
        mIgnoredCSVLinePrefixes {},
        mIgnoredJsonKeys {},
        mTruncated { false }
{}


QString ResultComparator::getDifferenceReport() const {
    if ( mDifferences.isEmpty() ) {
        return sNoDifferencesString;
    }

    QString report {
        sDifferencesFoundString.arg(
            QString::number( mDifferences.size() ),
            QString::number( mTolerance.absolute, 'g' ),
            QString::number( mTolerance.relative, 'g' )
        )
    };
    report += mDifferences.join( sEndl );

    if ( mTruncated ) {
        report += sEndlEllipsis;
    }

    return report;
}


QStringList ResultComparator::getDifferences() const {
    return mDifferences;
}


Tolerance ResultComparator::getTolerance() const {
    return mTolerance;
}


bool ResultComparator::compareProjectFiles(
    const QString &actualFileName,
    const QString &referenceFileName
) {
    mDifferences.clear();
    mTruncated = false;

    QByteArray actualContent {};
    QByteArray referenceContent {};
    if ( !readFile( actualFileName, actualContent ) ||
         !readFile( referenceFileName, referenceContent ) ) {
        return false;
    }

    // Sigma project files (.sig) are stored in JSON format
    QJsonParseError actualError {};
    QJsonParseError referenceError {};
    const QJsonDocument actualDoc {
        QJsonDocument::fromJson( actualContent, &actualError )
    };
    const QJsonDocument referenceDoc {
        QJsonDocument::fromJson( referenceContent, &referenceError )
    };

    if ( actualError.error != QJsonParseError::NoError ) {
        addDifference(
            sInvalidJsonString.arg( actualFileName, actualError.errorString() )
        );
        return false;
    }

    if ( referenceError.error != QJsonParseError::NoError ) {
        addDifference(
            sInvalidJsonString.arg(
                referenceFileName, referenceError.errorString()
            )
        );
        return false;
    }

    compareJsonValues(
        QJsonValue { actualDoc.object() },
        QJsonValue { referenceDoc.object() },
        sProjectString
    );

    return mDifferences.isEmpty();
}


bool ResultComparator::compareCSVFiles(
    const QString &actualFileName,
    const QString &referenceFileName
) {
    mDifferences.clear();
    mTruncated = false;

    QByteArray actualContent {};
    QByteArray referenceContent {};
    if ( !readFile( actualFileName, actualContent ) ||
         !readFile( referenceFileName, referenceContent ) ) {
        return false;
    }

    const QStringList actualLines {
        QString::fromUtf8( actualContent ).split( sEndl )
    };
    const QStringList referenceLines {
        QString::fromUtf8( referenceContent ).split( sEndl )
    };

    if ( actualLines.size() != referenceLines.size() ) {
        addDifference(
            sNumberOfLinesVsReferenceString.arg(
                QString::number( actualLines.size() ),
                QString::number( referenceLines.size() )
            )
        );
    }

    const qsizetype lineCount {
        qMin( actualLines.size(), referenceLines.size() )
    };

    for ( qsizetype i { 0 }; i < lineCount && !isComplete(); ++i ) {
        const QString &actualLine { actualLines.at( i ) };
        const QString &referenceLine { referenceLines.at( i ) };

        if ( isIgnoredCSVLine( actualLine ) ||
             isIgnoredCSVLine( referenceLine ) ) {
            continue;
        }

        compareTexts(
            actualLine,
            referenceLine,
            sLineString.arg( QString::number( i + 1 ) )
        );
    }

    return mDifferences.isEmpty();
}


void ResultComparator::setIgnoredCSVLinePrefixes(
    const QStringList &prefixes
) {
    mIgnoredCSVLinePrefixes = prefixes;
}


void ResultComparator::setIgnoredJsonKeys( const QStringList &keys ) {
    mIgnoredJsonKeys = keys;
}


bool ResultComparator::compareJsonValues(
    const QJsonValue &actual,
    const QJsonValue &reference,
    const QString &location
) {
    if ( actual.type() != reference.type() ) {
        addDifference(
            sTypeDifferenceString.arg(
                location,
                typeName( actual ),
                typeName( reference )
            )
        );
        return false;
    }

    if ( actual.isObject() ) {
        const QJsonObject actualObject { actual.toObject() };
        const QJsonObject referenceObject { reference.toObject() };

        QStringList keys { actualObject.keys() };
        for ( const QString &key : referenceObject.keys() ) {
            if ( !keys.contains( key ) ) {
                keys.append( key );
            }
        }
        keys.sort();

        bool equal { true };
        for ( const QString &key : std::as_const( keys ) ) {
            if ( isComplete() ) {
                break;
            }
            if ( mIgnoredJsonKeys.contains( key ) ) {
                continue;
            }

            const QString path { sPathString.arg( location, key ) };

            if ( !actualObject.contains( key ) ) {
                addDifference( sMissingString.arg( path ) );
                equal = false;
            }
            else if ( !referenceObject.contains( key ) ) {
                addDifference( sAddedString.arg( path ) );
                equal = false;
            }
            else {
                equal &= compareJsonValues(
                    actualObject.value( key ),
                    referenceObject.value( key ),
                    path
                );
            }
        }
        return equal;
    }

    if ( actual.isArray() ) {
        const QJsonArray actualArray = actual.toArray();
        const QJsonArray referenceArray = reference.toArray();

        bool equal { true };
        if ( actualArray.size() != referenceArray.size() ) {
            addDifference(
                sNumElementsDifferenceString.arg(
                    location,
                    QString::number( actualArray.size() ),
                    QString::number( referenceArray.size() )
                )
            );
            equal = false;
        }

        const qsizetype size {
            qMin( actualArray.size(), referenceArray.size() )
        };
        for ( qsizetype i { 0 }; i < size && !isComplete(); ++i ) {
            equal &= compareJsonValues(
                actualArray.at( i ),
                referenceArray.at( i ),
                sLocationIndexString.arg( location, QString::number( i ) )
            );
        }
        return equal;
    }

    if ( actual.isDouble() ) {
        return compareValues( actual.toDouble(), reference.toDouble(), location );
    }

    if ( actual.isString() ) {
        // Strings can contain numbers, e.g. the Monte Carlo simulation status
        return compareTexts( actual.toString(), reference.toString(), location );
    }

    if ( actual.isBool() && actual.toBool() != reference.toBool() ) {
        addDifference(
            sBooleanDifferenceString.arg(
                location,
                actual.toBool() ? sTrueString : sFalseString,
                reference.toBool() ? sTrueString : sFalseString
            )
        );
        return false;
    }

    return true;
}


bool ResultComparator::compareTexts(
    const QString &actual,
    const QString &reference,
    const QString &location
){
    if ( actual == reference ) {
        return true;
    }

    const TextTokens actualTokens { tokenize( actual ) };
    const TextTokens referenceTokens { tokenize( reference ) };

    if ( actualTokens.literals != referenceTokens.literals ) {
        addDifference(
            sTextDifferenceString.arg(
                location,
                actual,
                reference
            )
        );
        return false;
    }

    // The literal parts are identical, so only the numbers can differ
    bool equal { true };
    for ( qsizetype i { 0 }; i < actualTokens.numbers.size(); ++i ) {
        equal &= compareValues(
            actualTokens.numbers.at( i ),
            referenceTokens.numbers.at( i ),
            sLocationValueString.arg(
                location,
                QString::number( i + 1 )
            )
        );
    }

    return equal;
}


bool ResultComparator::compareValues(
    double actual,
    double reference,
    const QString &location
){
    if ( valuesEqual( actual, reference ) ) {
        return true;
    }

    const double difference { actual - reference };
    const double magnitude {
        qMax( std::abs( actual ), std::abs( reference ) )
    };
    const double relative {
        magnitude > 0. ? std::abs( difference ) / magnitude : 0.
    };

    addDifference(
        sValueDifferenceString.arg(
            location,
            QString::number( actual, 'g', sValueDigits ),
            QString::number( reference, 'g', sValueDigits ),
            QString::number( difference, 'g', sDifferenceDigits ),
            QString::number( relative, 'g', sDifferenceDigits )
        )
    );

    return false;
}


bool ResultComparator::isComplete() const {
    // Stop comparing when the maximum number of differences is reached
    return mTruncated;
}


bool ResultComparator::isIgnoredCSVLine( const QString &line ) const {
    for ( const QString &prefix : mIgnoredCSVLinePrefixes ) {
        if ( !prefix.isEmpty() && line.startsWith( prefix ) ) {
            return true;
        }
    }
    return false;
}


bool ResultComparator::readFile( const QString &fileName, QByteArray &content )
{
    QFile file { fileName };
    if ( !file.open( QIODevice::ReadOnly ) ) {
        addDifference( sFileOpenErrorString.arg( fileName ) );
        return false;
    }

    content = file.readAll();
    file.close();
    return true;
}


bool ResultComparator::valuesEqual( double actual, double reference ) const {
    if ( std::isnan( actual ) || std::isnan( reference ) ) {
        return std::isnan( actual ) && std::isnan( reference );
    }

    if ( actual == reference ) {
        return true;
    }

    const double difference { std::abs( actual - reference ) };
    if ( difference <= mTolerance.absolute ) {
        return true;
    }

    const double magnitude {
        qMax( std::abs( actual ), std::abs( reference ) )
    };
    return difference <= mTolerance.relative * magnitude;
}


void ResultComparator::addDifference( const QString &difference ) {
    if ( mDifferences.size() < sMaxDifferences ) {
        mDifferences.append( difference );
    }
    else {
        mTruncated = true;
    }
}


ResultComparator::TextTokens ResultComparator::tokenize(
    const QString &text
){
    // Split the text into the numbers it contains and the literal text in
    // between, so that the numbers can be compared with a tolerance
    TextTokens tokens {};
    qsizetype position { 0 };

    QRegularExpressionMatchIterator it {
        sNumberExpression.globalMatch( text )
    };
    while ( it.hasNext() ) {
        const QRegularExpressionMatch match { it.next() };
        bool ok {};
        const double value { match.captured().toDouble( &ok ) };
        if ( !ok ) {
            continue;
        }
        tokens.literals.append(
            text.mid( position, match.capturedStart() - position )
        );
        tokens.numbers.append( value );
        position = match.capturedEnd();
    }

    tokens.literals.append( text.mid( position ) );
    return tokens;
}


QString ResultComparator::typeName( const QJsonValue &value ) {
    switch ( value.type() ) {
        case QJsonValue::Null:      return "null";
        case QJsonValue::Bool:      return "bool";
        case QJsonValue::Double:    return "number";
        case QJsonValue::String:    return "string";
        case QJsonValue::Array:     return "array";
        case QJsonValue::Object:    return "object";
        default:                    return "undefined";
    }
}
