// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "regressionconfig.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>


RegressionConfig::RegressionConfig()
    :   mDemoProjectsDirectory {},
        mError {},
        mReferenceDirectory {},
        mTimeoutSeconds { sDefaultTimeoutSeconds },
        mDefaultTolerance {},
        mIgnoredCSVLinePrefixes {},
        mIgnoredJsonKeys {},
        mProjects {}
{}


QList<ProjectConfig> RegressionConfig::getProjects() const {
    return mProjects;
}


QString RegressionConfig::getDemoProjectsDirectory() const {
    return mDemoProjectsDirectory;
}


QString RegressionConfig::getError() const {
    return mError;
}


QString RegressionConfig::getReferenceDirectory() const {
    return mReferenceDirectory;
}


QStringList RegressionConfig::getIgnoredCSVLinePrefixes() const {
    return mIgnoredCSVLinePrefixes;
}


QStringList RegressionConfig::getIgnoredJsonKeys() const {
    return mIgnoredJsonKeys;
}


Tolerance RegressionConfig::getDefaultTolerance() const {
    return mDefaultTolerance;
}


bool RegressionConfig::load( const QString &fileName ) {
    mError.clear();
    mProjects.clear();

    QDir configDir { QFileInfo( fileName ).absoluteDir() };

    QFile file { fileName };
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        mError = sOpenConfigFailedString + fileName;
        return false;
    }

    QJsonParseError parseError {};
    const QJsonDocument doc {
        QJsonDocument::fromJson( file.readAll(), &parseError )
    };
    file.close();

    if ( parseError.error != QJsonParseError::NoError ) {
        mError = sInvalidJsonString.arg(
            fileName,
            QString::number( parseError.offset ),
            parseError.errorString()
        );
        return false;
    }

    if ( !doc.isObject() ) {
        mError = sNoJsonInConfigString + fileName;
        return false;
    }

    const QJsonObject root { doc.object() };

    mDemoProjectsDirectory = root.value( sDemoProjectsDirectoryKey ).toString();
    if ( mDemoProjectsDirectory.isEmpty() ) {
        mError = sMissingOrEmptyString.arg( sDemoProjectsDirectoryKey );
        return false;
    }
    // Relative paths are assumed to be with respect to the config file
    if ( QDir::isRelativePath( mDemoProjectsDirectory ) ) {
        mDemoProjectsDirectory = QDir::cleanPath(
            configDir.absoluteFilePath( mDemoProjectsDirectory )
        );
    }

    mReferenceDirectory = root.value( sReferenceDirectoryKey ).toString();
    if ( mReferenceDirectory.isEmpty() ) {
        mError = sMissingOrEmptyString.arg( sReferenceDirectoryKey );
        return false;
    }
    // Relative paths are assumed to be with respect to the config file
    if ( QDir::isRelativePath( mReferenceDirectory ) ) {
        mReferenceDirectory = QDir::cleanPath(
            configDir.absoluteFilePath( mReferenceDirectory )
        );
    }

    mTimeoutSeconds = root.value( sTimeoutKey ).toInt( sDefaultTimeoutSeconds );
    if ( mTimeoutSeconds <= 0 ) {
        mError = sMustBePositiveString.arg( sTimeoutKey );
        return false;
    }

    // The default tolerances are used for the projects that do not define
    // their own tolerances
    mDefaultTolerance = parseTolerance(
        root.value( sDefaultToleranceKey ),
        Tolerance {}
    );

    mIgnoredCSVLinePrefixes.clear();
    // Note: braced initialization of a QJsonArray with a single array
    // argument would create a nested array, so copy initialization is used
    const QJsonArray prefixes =
        root.value( sIgnoredCSVLinePrefixesKey ).toArray();
    for ( const QJsonValue &prefix : prefixes ) {
        mIgnoredCSVLinePrefixes.append( prefix.toString() );
    }

    mIgnoredJsonKeys.clear();
    const QJsonArray keys = root.value( sIgnoredJsonKeysKey ).toArray();
    for ( const QJsonValue &key : keys ) {
        mIgnoredJsonKeys.append( key.toString() );
    }

    if ( !root.value( sProjectsKey ).isArray() ) {
        mError = sMissingOrInvalidArrayString.arg( sProjectsKey );
        return false;
    }

    const QJsonArray projects = root.value( sProjectsKey ).toArray();
    for ( const QJsonValue &project : projects ) {
        if ( !parseProject( project ) ) {
            return false;
        }
    }

    if ( mProjects.isEmpty() ) {
        mError = sDoesNotContainProjectString.arg( sProjectsKey );
        return false;
    }

    return true;
}


int RegressionConfig::getTimeoutSeconds() const {
    return mTimeoutSeconds;
}


bool RegressionConfig::parseProject( const QJsonValue &value ) {
    if ( !value.isObject() ) {
        mError = sProjectEntryNotJsonObjectString;
        return false;
    }

    const QJsonObject object { value.toObject() };
    ProjectConfig project {};
    project.name = object.value( sNameKey ).toString();

    if ( project.name.isEmpty() ) {
        mError = sProjectEntryWithoutKeyString.arg( sNameKey );
        return false;
    }

    const QList<ProjectConfig> &projects { mProjects };
    for ( const ProjectConfig &existing : projects ) {
        if ( existing.name == project.name ) {
            mError = sDuplicateProjectNameString + project.name;
            return false;
        }
    }

    // The tolerances of a project are optional and fall back to the defaults
    project.tolerance = parseTolerance(
        object.value( sToleranceKey ),
        mDefaultTolerance
    );

    if ( !parseRuns( object.value( sRunsKey ), project ) ) {
        return false;
    }

    mProjects.append( project );
    return true;
}


bool RegressionConfig::parseRuns(
    const QJsonValue &value,
    ProjectConfig &project
){
    if ( !value.isArray() ) {
        mError = sMissingOrInvalidArrayForProjectString.arg(
            sRunsKey,
            project.name
        );
        return false;
    }

    const QJsonArray runs = value.toArray();
    for ( const QJsonValue &run : runs ) {
        if ( !run.isObject() ) {
            mError = sRunEntryNotAJsonObjectForProjectString + project.name;
            return false;
        }
        project.runs.append( RunConfig { parseArguments( run.toObject() ) } );
    }

    if ( project.runs.isEmpty() ) {
        mError = sEmptyArrayForProjectString.arg( sRunsKey, project.name );
        return false;
    }

    return true;
}


QStringList RegressionConfig::parseArguments( const QJsonObject &object ) {
    // Convert the command-line options of a single run into an argument list.
    // The keys are option names without the leading dashes. A value of true
    // adds the option as a flag, any other value is added as option value.
    QStringList arguments {};

    for ( auto it { object.constBegin() }; it != object.constEnd(); ++it ) {
        if ( it.key() == sCommentKey ) {
            continue;
        }

        arguments.append( "--" + it.key() );

        if ( it.value().isBool() ) {
            if ( !it.value().toBool() ) {
                // The option is explicitly disabled
                arguments.removeLast();
            }
        }
        else if ( it.value().isDouble() ) {
            arguments.append(
                QString::number(
                    it.value().toDouble(),
                    'g',
                    sCommandLineDigits
                )
            );
        }
        else {
            arguments.append( it.value().toString() );
        }
    }

    return arguments;
}


Tolerance RegressionConfig::parseTolerance(
    const QJsonValue &value,
    const Tolerance &fallback
) {
    Tolerance tolerance { fallback };

    if ( value.isObject() ) {
        const QJsonObject object { value.toObject() };
        tolerance.absolute = object.value( sAbsoluteKey ).toDouble(
            fallback.absolute
        );
        tolerance.relative = object.value( sRelativeKey ).toDouble(
            fallback.relative
        );
    }

    return tolerance;
}
