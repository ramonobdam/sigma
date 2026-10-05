// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef RESULTCOMPARATOR_H
#define RESULTCOMPARATOR_H

#include "regressionconfig.h"
#include <QJsonValue>
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <QStringList>

// Class that compares the results of a Sigma run with the reference results of
// a previous version. Textual parts must be identical, numerical values are
// compared with the absolute and relative tolerance of the project: two values
// are equal when the difference is within the absolute tolerance or within the
// relative tolerance.
// The comparison is number aware: every text is split into numbers and the
// literal text in between (e.g. '±92.61982103' is split into '±' and
// 92.61982103), so that numbers embedded in text are compared numerically.
class ResultComparator {
public:
    explicit ResultComparator( const Tolerance &tolerance );

    QString getDifferenceReport() const;
    QStringList getDifferences() const;
    Tolerance getTolerance() const;
    bool compareProjectFiles(
        const QString &actualFileName,
        const QString &referenceFileName
    );
    bool compareCSVFiles(
        const QString &actualFileName,
        const QString &referenceFileName
    );
    void setIgnoredCSVLinePrefixes( const QStringList &prefixes );
    void setIgnoredJsonKeys( const QStringList &keys );

private:
    // A text split into the numbers it contains and the literal text in
    // between. The number of literals is always one more than the number of
    // numbers.
    struct TextTokens {
        QStringList literals;
        QList<double> numbers;
    };

    bool compareJsonValues(
        const QJsonValue &actual,
        const QJsonValue &reference,
        const QString &location
    );
    bool compareTexts(
        const QString &actual,
        const QString &reference,
        const QString &location
    );
    bool compareValues(
        double actual,
        double reference,
        const QString &location
    );
    bool isComplete() const;
    bool isIgnoredCSVLine( const QString &line ) const;
    bool readFile( const QString &fileName, QByteArray &content );
    bool valuesEqual( double actual, double reference ) const;
    void addDifference( const QString &difference );

    static TextTokens tokenize( const QString &text );
    static QString typeName( const QJsonValue &value );

    // Maximum number of differences that is reported per comparison
    static constexpr int sMaxDifferences { 25 };

    static constexpr QLatin1StringView sNoDifferencesString {
        "No differences"
    };
    static constexpr QLatin1StringView sDifferencesFoundString {
       "%1 difference(s) found (absolute tolerance %2, relative tolerance %3):"
        "\n"
    };
    static constexpr QLatin1StringView sInvalidJsonString {
        "Invalid JSON in %1: %2"
    };
    static constexpr QLatin1StringView sProjectString { "project" };
    static constexpr QLatin1StringView sEndl { "\n" };
    static constexpr QLatin1StringView sEndlEllipsis { "\n..." };
    static constexpr QLatin1StringView sNumberOfLinesVsReferenceString {
        "Number of lines: %1 (reference: %2)"
    };
    static constexpr QLatin1StringView sLineString { "line %1" };
    static constexpr QLatin1StringView sTypeDifferenceString {
        "%1: type %2 (reference: %3)"
    };
    static constexpr QLatin1StringView sMissingString {
        "%1: missing (present in reference)"
    };
    static constexpr QLatin1StringView sAddedString {
        "%1: added (absent in reference)"
    };
    static constexpr QLatin1StringView sPathString { "%1.%2" };
    static constexpr QLatin1StringView sNumElementsDifferenceString {
        "%1: %2 element(s) (reference: %3)"
    };
    static constexpr QLatin1StringView sLocationIndexString { "%1[%2]" };
    static constexpr QLatin1StringView sBooleanDifferenceString {
        "%1: %2 (reference: %3)"
    };
    static constexpr QLatin1StringView sTrueString { "true" };
    static constexpr QLatin1StringView sFalseString { "false" };
    static constexpr QLatin1StringView sTextDifferenceString {
        "%1: '%2' (reference: '%3')"
    };
    static constexpr QLatin1StringView sLocationValueString { "%1, value %2" };
    static constexpr QLatin1StringView sValueDifferenceString {
        "%1: %2 (reference: %3, difference: %4, relative: %5)"
    };
    static constexpr QLatin1StringView sFileOpenErrorString {
        "Could not open file: %1"
    };

    static constexpr int sValueDigits { 15 };
    static constexpr int sDifferenceDigits { 6 };

    Tolerance mTolerance;
    QStringList mDifferences;
    QStringList mIgnoredCSVLinePrefixes;
    QStringList mIgnoredJsonKeys;
    bool mTruncated;
};

#endif // RESULTCOMPARATOR_H
