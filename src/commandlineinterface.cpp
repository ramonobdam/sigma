// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "commandlineinterface.h"
#include "exitcodes.h"
#include <QEventLoop>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocale>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QtLogging>
#include <csignal>
#include <cstdio>
#include <cstdlib>


CommandLineInterface::CommandLineInterface(
    QCoreApplication *app,
    UncertaintyCalculation *calculation
)   :   mApp { app },
        mCalculation { calculation },
        mParser {},
        // Overwritten with actual names in initializeParser():
        mVersionOption { "version" },
        mProjectLoaded { false }
{
    initializeParser();
    setupSignalHandling();
}


QCoreApplication * CommandLineInterface::getCoreApplication() const {
    return mApp;
}


UncertaintyCalculation * CommandLineInterface::getUncertaintyCalculation()
const {
    return mCalculation;
}


int CommandLineInterface::process() {
    // Use parse() instead of process() to avoid exit()
    if ( !mParser.parse( mApp->arguments() ) ) {
        qCritical() << mParser.errorText();
        return static_cast<int>( ExitCode::UsageError );
    }

    // Handle help and version manually since parse() doesn't handle them
    if ( mParser.isSet( sHelpOption ) ) {
        QTextStream out { stdout };
        out << mParser.helpText();
        return static_cast<int>( ExitCode::Success );
    }

    if ( mParser.isSet( mVersionOption ) ) {
        QTextStream out { stdout };
        out << mApp->applicationName() << " "
            << mApp->applicationVersion() << "\n";
        return static_cast<int>( ExitCode::Success );
    }

    // Process setting options
    for ( const CLISettingOption &setting : sSettingOptions ) {
        if ( mParser.isSet( setting.option ) ) {
            const QString value { mParser.value( setting.option ) };
            if ( !setting.setter( value ) ) {
                QString msg { setting.option.description() };
                // Remove the pre/post fix from the description:
                msg = msg.mid(
                    sPreFix.size(),
                    msg.size() - sPreFix.size() - sPostFix.size()
                );
                msg += " could not be set: " + value;
                qCritical() << msg;
                return static_cast<int>( ExitCode::UsageError );
            }
        }
    }

    if ( mParser.isSet( sOpenOption ) ) {
        // Open a project from file
        QUrl url { QUrl::fromLocalFile( mParser.value( sOpenOption ) ) };
        if ( !mCalculation->loadProject( url ) ) {
            return static_cast<int>( ExitCode::LoadError );
        }
        mProjectLoaded = true;
    }

    if ( mParser.isSet( sFromJsonOption ) ) {
        // Load project data in JSON format from stdin
        QTextStream in { stdin };
        QString jsonString { in.readAll() };
        QJsonParseError error {};
        QJsonDocument doc {
            QJsonDocument::fromJson( jsonString.toUtf8(), &error )
        };

        if ( error.error != QJsonParseError::NoError ) {
            qCritical() << "Invalid JSON:" << error.errorString();
            return static_cast<int>( ExitCode::UsageError );
        }

        mCalculation->projectFromJson( doc.object() );
        mProjectLoaded = true;
    }

    if ( mParser.isSet( sRunOption ) ) {
        // Run Monte Carlo simulation for the specified output parameter
        const int exitCode { runMonteCarlo( mParser.value( sRunOption ) ) };
        if ( exitCode != static_cast<int>( ExitCode::Success ) ) {
            // An error occurred
            return exitCode;
        }
    }

    if ( mParser.isSet( sRunAllOption ) ) {
        // Run Monte Carlo simulation for all output parameters
        const QStringList names { mCalculation->getOutputParameterNames() };
        for ( const QString &name : names ) {
            const int exitCode { runMonteCarlo( name ) };
            if ( exitCode != static_cast<int>( ExitCode::Success ) ) {
                // An error occurred
                return exitCode;
            }
        }
    }

    if ( mParser.isSet( sSaveOption ) ) {
        // Save the project to file
        QUrl url { QUrl::fromLocalFile( mParser.value( sSaveOption ) ) };
        if ( !mCalculation->saveProject( url ) ) {
            return static_cast<int>( ExitCode::SaveError );
        }
    }

    if ( mParser.isSet( sExportOption ) ) {
        // Export the project to CSV file
        QUrl url { QUrl::fromLocalFile( mParser.value( sExportOption ) ) };
        if ( !mCalculation->saveCSV( url ) ) {
            return static_cast<int>( ExitCode::ExportError );
        }
    }

    if ( mParser.isSet( sToJsonOption ) ) {
        // Print the project data to stdout in JSON format
        QJsonDocument doc { mCalculation->projectToJson() };
        QTextStream out { stdout };
        out << doc.toJson( QJsonDocument::Indented );
    }

    return static_cast<int>( ExitCode::Success );
}


bool CommandLineInterface::getProjectLoaded() const {
    return mProjectLoaded;
}


void CommandLineInterface::setCoreApplication( QCoreApplication *app ) {
    mApp = app;
}


void CommandLineInterface::setUncertaintyCalculation(
    UncertaintyCalculation *calculation
){
    mCalculation = calculation;
}


bool CommandLineInterface::headless( int argc, char *argv[] ) {
    // Add dashes to option names
    QStringList optionNames { sHeadlessOption.names() };
    for ( int i { 0 }; i < optionNames.size(); ++i ) {
        QString name { optionNames[ i ] };
        name = ( name.size() == 1 ? "-" : "--" ) + name;
        optionNames[ i ] = name;
    }

    // Check if the headless flag is set
    const QStringList &names { optionNames };
    for ( int i { 1 }; i < argc; ++i ) {
        for ( const QString &name : names ) {
            if ( QString( argv[ i ] ) == name ) {
                return true;
            }
        }
    }

    return false;
}


int CommandLineInterface::runMonteCarlo( const QString &name ) const {
    // Select the output parameter
    if ( mCalculation->selectOutputParameterByName( name ) ) {
        QEventLoop loop {};
        // Connect monteCarloFinished signal to quit signal of eventloop
        QObject::connect(
            mCalculation,
            &UncertaintyCalculation::monteCarloFinished,
            &loop,
            &QEventLoop::quit
        );
        mCalculation->runMonteCarlo();
        loop.exec();  // wait for Monte Carlo to finish

        if ( mCalculation->getHistogramValid() ) {
            return static_cast<int>( ExitCode::Success );
        }
        else {
            qCritical() << QString(
                "Monte Carlo simulation failed for output parameter '" +
                name +
                "'"
            );
            return static_cast<int>( ExitCode::SimulationError );
        }
    }
    else {
        qCritical() << "Output parameter '" + name + "' not available";
        return static_cast<int>( ExitCode::SimulationError );
    }
}


void CommandLineInterface::initializeParser() {
    // Define command-line parser and options
    mParser.setApplicationDescription(
        QString(
            "\n%1 [version %2]\n\n"
            "%3 implements measurement uncertainty calculation in "
            "accordance with the Guide to the expression of Uncertainty "
            "in Measurement (GUM).\n"
            "For more information, see https://github.com/ramonobdam/sigma"
        ).arg(
            APP_DISPLAY_NAME,
            mApp->applicationVersion(),
            mApp->applicationName()
        )
    );

    // Add options
    mParser.addOption( sHelpOption );
    mVersionOption = mParser.addVersionOption();
    mParser.addOption( sHeadlessOption );
    mParser.addOption( sOpenOption );
    mParser.addOption( sRunOption );
    mParser.addOption( sRunAllOption );
    mParser.addOption( sSaveOption );
    mParser.addOption( sExportOption );
    mParser.addOption( sToJsonOption );
    mParser.addOption( sFromJsonOption );

    // Add setting options
    for ( const CLISettingOption &setting : sSettingOptions ) {
        mParser.addOption( setting.option );
    }
}


void CommandLineInterface::setupSignalHandling() {
    // Quit when the user presses Ctrl+C
    std::signal(
        SIGINT,
        []( int ) {
            qInfo() << "Interrupted by user";
            std::exit( static_cast<int> ( ExitCode::Interrupted ) );
        }
    );
}
