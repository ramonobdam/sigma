// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef COMMANDLINEINTERFACE_H
#define COMMANDLINEINTERFACE_H

#include "settings.h"
#include "uncertaintycalculation.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <cmath>
#include <functional>

// Class that implements the Sigma command-line interface with the following
// options. Note that the allowable ranges of the settings options are obtained
// from the Settings class.

// -h, --help            Displays help on commandline options.
// -v, --version         Displays version information.
// -H, --headless        Run Sigma without the GUI.
// --open <file>         Opens a project from <file>.
// --run <par>           Runs Monte Carlo simulation for output parameter <par>.
// --run-all             Runs Monte Carlo simulation for all output parameters.
// --save <file>         Saves the project to <file>.
// --export <file>       Exports the results to CSV file.
// --to-json             Prints the project data to stdout in JSON format.
// --from-json           Loads the project data from stdin in JSON format.
// --csvdigits <digits>  Sets the CSV export significant digits (1 to 20).
// --mcdigits <digits>   Sets the Monte Carlo significant digits (1 to 3).
// --mcbatchsize <size>  Sets the Monte Carlo batch size (1e+02 to 1e+06).
// --mcmaxbatches <num>  Sets the Monte Carlo maximum number of batches (1e+01
//                       to 1e+05).

// The process() method maps the commands to the UncertaintyCalculation
// instance.
class CommandLineInterface {
public:
    CommandLineInterface(
        QCoreApplication *app,
        UncertaintyCalculation *calculation
    );

    QCoreApplication *getCoreApplication() const;
    UncertaintyCalculation *getUncertaintyCalculation() const;
    int process();
    void setCoreApplication( QCoreApplication *app );
    void setUncertaintyCalculation( UncertaintyCalculation *calculation );

    static bool headless( int argc, char *argv[] );

private:
    int runMonteCarlo( const QString &name ) const;
    void initializeParser();
    void setupSignalHandling();

    struct CLISettingOption {
        QCommandLineOption option;
        std::function<bool( const QString & )> setter;
    };

    // Pre- and postfix added to the setting options descriptions (so they can
    // be stripped away for the error messages):
    static constexpr QLatin1StringView sPreFix { "Sets the " };
    static constexpr QLatin1StringView sPostFix { "." };

    inline static const QList<CLISettingOption> sSettingOptions {
        {
            QCommandLineOption {
                "csvdigits",
                QString( "%1CSV export significant digits (%2 to %3)%4" ).arg(
                    sPreFix,
                    QString::number( Settings::getMinCSVPrecision() ),
                    QString::number( Settings::getMaxCSVPrecision() ),
                    sPostFix
                ),
                "digits"
            },
            []( const QString &value ) {
                bool ok {};
                double d { value.toDouble( &ok ) };
                if ( !ok ) return false;
                return Settings::setCSVPrecision(
                    static_cast<int>( std::round( d ) )
                );
            }
        },
        {
            QCommandLineOption {
                "mcdigits",
                QString( "%1Monte Carlo significant digits (%2 to %3)%4" ).arg(
                    sPreFix,
                    QString::number( Settings::getMinMonteCarloDigits() ),
                    QString::number( Settings::getMaxMonteCarloDigits() ),
                    sPostFix
                    ),
                "digits"
            },
            []( const QString &value ) {
                bool ok {};
                double d { value.toDouble( &ok ) };
                if ( !ok ) return false;
                return Settings::setMonteCarloDigits(
                    static_cast<int>( std::round( d ) )
                );
            }
        },
        {
            QCommandLineOption {
                "mcbatchsize",
                QString( "%1Monte Carlo batch size (%2 to %3)%4" ).arg(
                    sPreFix,
                    QString::number(
                        Settings::getMinMonteCarloBatchSize(),
                        'e',
                        0
                    ),
                    QString::number(
                        Settings::getMaxMonteCarloBatchSize(),
                        'e',
                        0
                    ),
                    sPostFix
                ),
                "size"
            },
            []( const QString &value ) {
                bool ok {};
                double d { value.toDouble( &ok ) };
                if ( !ok ) return false;
                return Settings::setMonteCarloBatchSize(
                    static_cast<int>( std::round( d ) )
                );
            }
        },
        {
            QCommandLineOption {
                "mcmaxbatches",
                QString(
                    "%1Monte Carlo maximum number of batches (%2 to %3)%4"
                ).arg(
                    sPreFix,
                    QString::number(
                        Settings::getMinMonteCarloMaxNumOfBatches(),
                        'e',
                        0
                    ),
                    QString::number(
                        Settings::getMaxMonteCarloMaxNumOfBatches(),
                        'e',
                        0
                    ),
                    sPostFix
                ),
                "num"
            },
            []( const QString &value ) {
                bool ok {};
                double d { value.toDouble( &ok ) };
                if ( !ok ) return false;
                return Settings::setMonteCarloMaxNumOfBatches(
                    static_cast<int>( std::round( d ) )
                );
            }
        }
    };

    inline static QCommandLineOption sExportOption {
        "export",
        "Exports the results to CSV file.",
        "file"
    };
    inline static QCommandLineOption sHeadlessOption {
        { "H", "headless" },
        "Runs Sigma without the GUI."
    };
    inline static QCommandLineOption sHelpOption {
        {
         "h",
         "help"
// On Windows, also support -?
#ifdef Q_OS_WINDOWS
, "?"
#endif
        },
        "Displays help on commandline options."
    };
    inline static QCommandLineOption sOpenOption {
        "open",
        "Opens a project from <file>.",
        "file"
    };
    inline static QCommandLineOption sRunOption {
        "run",
        "Runs Monte Carlo simulation for output parameter <par>.",
        "par"
    };
    inline static QCommandLineOption sRunAllOption {
        "run-all",
        "Runs Monte Carlo simulation for all output parameters."
    };
    inline static QCommandLineOption sSaveOption {
        "save",
        "Saves the project to <file>.",
        "file"
    };
    inline static QCommandLineOption sToJsonOption {
        "to-json",
        "Prints the project data to stdout in JSON format."
    };
    inline static QCommandLineOption sFromJsonOption {
        "from-json",
        "Loads the project data from stdin in JSON format."
    };

    QCommandLineOption mVersionOption;
    QCommandLineParser mParser;
    QCoreApplication *mApp;
    UncertaintyCalculation *mCalculation;
};

#endif // COMMANDLINEINTERFACE_H
