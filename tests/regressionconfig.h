// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef REGRESSIONCONFIG_H
#define REGRESSIONCONFIG_H

#include <QJsonObject>
#include <QJsonValue>
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <QStringList>

// The absolute and relative tolerance used to compare numerical results. Two
// values are considered equal when the difference is within the absolute or
// within the relative tolerance.
struct Tolerance {
    double absolute { 0. };
    double relative { 0. };
};

// One invocation of the Sigma command-line interface. The arguments are stored
// as a ready to use argument list, e.g. { "--run-all", "--csvdigits", "10" }.
// The --open, --save, --export and --headless options are not part of the
// configuration file, since they depend on the working directory of the test.
struct RunConfig {
    QStringList arguments;
};

// A demo project with its command-line options and result tolerances
struct ProjectConfig {
    QString name;
    Tolerance tolerance;
    QList<RunConfig> runs;
};

// Class that reads the regression test configuration from a JSON file. The
// file contains the command-line options of every demo project, an optional
// absolute and relative tolerance per project and the default tolerances that
// are used when a project does not define its own.
class RegressionConfig {
public:
    RegressionConfig();

    QList<ProjectConfig> getProjects() const;
    QString getDemoProjectsDirectory() const;
    QString getError() const;
    QString getReferenceDirectory() const;
    QStringList getIgnoredCSVLinePrefixes() const;
    QStringList getIgnoredJsonKeys() const;
    Tolerance getDefaultTolerance() const;
    bool load( const QString &fileName );
    int getTimeoutSeconds() const;

private:
    // Keys of the JSON configuration file
    static constexpr QLatin1StringView sCommentKey { "_comment" };
    static constexpr QLatin1StringView sDefaultToleranceKey {
        "defaultTolerance"
    };
    static constexpr QLatin1StringView sIgnoredCSVLinePrefixesKey {
        "ignoredCSVLinePrefixes"
    };
    static constexpr QLatin1StringView sIgnoredJsonKeysKey {
        "ignoredJSONKeys"
    };
    static constexpr QLatin1StringView sNameKey { "name" };
    static constexpr QLatin1StringView sProjectsKey { "projects" };
    static constexpr QLatin1StringView sReferenceDirectoryKey {
        "referenceDirectory"
    };
    static constexpr QLatin1StringView sDemoProjectsDirectoryKey {
        "demoProjectsDirectory"
    };
    static constexpr QLatin1StringView sRunsKey { "runs" };
    static constexpr QLatin1StringView sTimeoutKey { "timeoutSeconds" };
    static constexpr QLatin1StringView sToleranceKey { "tolerance" };
    static constexpr QLatin1StringView sAbsoluteKey { "absolute" };
    static constexpr QLatin1StringView sRelativeKey { "relative" };

    // Error messages
    static constexpr QLatin1StringView sOpenConfigFailedString {
        "Could not open configuration file: "
    };
    static constexpr QLatin1StringView sInvalidJsonString {
        "Invalid JSON in %1 at offset %2: %3"
    };
    static constexpr QLatin1StringView sNoJsonInConfigString {
        "Configuration file does not contain a JSON object: "
    };
    static constexpr QLatin1StringView sMissingOrEmptyString {
        "Missing or empty '%1'"
    };
    static constexpr QLatin1StringView sMustBePositiveString {
        "'%1' must be positive"
    };
    static constexpr QLatin1StringView sMissingOrInvalidArrayString {
        "Missing or invalid '%1' array"
    };
    static constexpr QLatin1StringView sDoesNotContainProjectString {
        "'%1' does not contain any project"
    };
    static constexpr QLatin1StringView sProjectEntryNotJsonObjectString {
        "Project entry is not a JSON object"
    };
    static constexpr QLatin1StringView sProjectEntryWithoutKeyString {
        "Project entry without '%1'"
    };
    static constexpr QLatin1StringView sDuplicateProjectNameString {
        "Duplicate project name: "
    };
    static constexpr QLatin1StringView sMissingOrInvalidArrayForProjectString {
        "Missing or invalid '%1' array for project %2"
    };
    static constexpr QLatin1StringView sRunEntryNotAJsonObjectForProjectString {
        "Run entry is not a JSON object for project "
    };
    static constexpr QLatin1StringView sEmptyArrayForProjectString {
        "Empty '%1' array for project %2"
    };

    static constexpr int sCommandLineDigits { 15 };
    static constexpr int sDefaultTimeoutSeconds { 600 };

    bool parseProject( const QJsonValue &value );
    bool parseRuns( const QJsonValue &value, ProjectConfig &project );
    static QStringList parseArguments( const QJsonObject &object );
    static Tolerance parseTolerance(
        const QJsonValue &value,
        const Tolerance &fallback
    );

    QList<ProjectConfig> mProjects;
    QString mDemoProjectsDirectory;
    QString mError;
    QString mReferenceDirectory;
    QStringList mIgnoredCSVLinePrefixes;
    QStringList mIgnoredJsonKeys;
    Tolerance mDefaultTolerance;
    int mTimeoutSeconds;
};

#endif // REGRESSIONCONFIG_H
