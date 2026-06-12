// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef COMMANDLINEINTERFACE_H
#define COMMANDLINEINTERFACE_H

#include "settings.h"
#include "uncertaintycalculation.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <cmath>
#include <functional>

// Class that implements the Sigma command-line interface with the following
// options. Note that the allowable ranges of the settings options are obtained
// from the Settings class.

// -h, --help            Displays help on commandline options.
// --help-all            Displays help, including generic Qt options.
// -v, --version         Displays version information.
// -H, --headless        Run Sigma without the GUI.
// --open <file>         Open a project from <file>.
// --run <par>           Run Monte Carlo simulation for output parameter <par>.
// --run-all             Run Monte Carlo simulation for all output parameters.
// --save <file>         Save the project to <file>.
// --export <file>       Export results to CSV file.
// --to-json             Print the project data to stdout in JSON format.
// --from-json           Load the project data from stdin in JSON format.
// --csvdigits <digits>  Set CSV export significant digits (1 to 20).
// --mcdigits <digits>   Set Monte Carlo significant digits (1 to 3).
// --mcbatchsize <size>  Set Monte Carlo batch size (1e+02 to 1e+06).
// --mcmaxbatches <num>  Set Monte Carlo maximum number of batches (1e+01 to
//                       1e+05).

// The process() method maps the commands to the UncertaintyCalculation
// instance.
class CommandLineInterface {
public:
    CommandLineInterface(
        QGuiApplication *app,
        UncertaintyCalculation *calculation
    );

    QGuiApplication *getGuiApplication() const;
    UncertaintyCalculation *getUncertaintyCalculation() const;
    bool getHeadless();
    int process();
    void setGuiApplication( QGuiApplication *app );
    void setUncertaintyCalculation( UncertaintyCalculation *calculation );

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
    static constexpr QLatin1StringView sPreFix { "Set " };
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
        "Export results to CSV file.",
        "file"
    };
    inline static QCommandLineOption sHeadlessOption {
        { "H", "headless" },
        "Run Sigma without the GUI."
    };
    inline static QCommandLineOption sOpenOption {
        "open",
        "Open a project from <file>.",
        "file"
    };
    inline static QCommandLineOption sRunOption {
        "run",
        "Run Monte Carlo simulation for output parameter <par>.",
        "par"
    };
    inline static QCommandLineOption sRunAllOption {
        "run-all",
        "Run Monte Carlo simulation for all output parameters."
    };
    inline static QCommandLineOption sSaveOption {
        "save",
        "Save the project to <file>.",
        "file"
    };
    inline static QCommandLineOption sToJsonOption {
        "to-json",
        "Print the project data to stdout in JSON format."
    };
    inline static QCommandLineOption sFromJsonOption {
        "from-json",
        "Load the project data from stdin in JSON format."
    };

    QCommandLineParser mParser;
    QGuiApplication *mApp;
    UncertaintyCalculation *mCalculation;
};

#endif // COMMANDLINEINTERFACE_H
