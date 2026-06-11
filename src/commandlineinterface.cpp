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
#include <QTextStream>
#include <QUrl>
#include <QtLogging>
#include <csignal>
#include <cstdio>
#include <cstdlib>


CommandLineInterface::CommandLineInterface(
    QGuiApplication *app,
    UncertaintyCalculation *calculation
)   :   mApp { app },
        mCalculation { calculation },
        mParser {}
{
    initializeParser();
    setupSignalHandling();
}


QGuiApplication * CommandLineInterface::getGuiApplication() const {
    return mApp;
}


UncertaintyCalculation * CommandLineInterface::getUncertaintyCalculation()
const {
    return mCalculation;
}


bool CommandLineInterface::getHeadless() {
    mParser.process( *mApp );
    return mParser.isSet( sHeadlessOption );;
}


int CommandLineInterface::process() {
    mParser.process( *mApp );

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


void CommandLineInterface::setGuiApplication( QGuiApplication *app ) {
    mApp = app;
}


void CommandLineInterface::setUncertaintyCalculation(
    UncertaintyCalculation *calculation
){
    mCalculation = calculation;
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
            qCritical() << "Monte Carlo simulation failed for output "
                        << "parameter '"
                        << name
                        << "'";
            return static_cast<int>( ExitCode::SimulationError );
        }
    }
    else {
        qCritical() << "Output parameter '"
                    << name
                    << "' not available";
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
            mApp->applicationDisplayName(),
            mApp->applicationVersion(),
            mApp->applicationName()
        )
    );

    // Add options
    mParser.addHelpOption();
    mParser.addVersionOption();
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
