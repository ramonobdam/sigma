// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "applicationsettings.h"
#include "commandlineinterface.h"
#include "exitcodes.h"
#include "uncertaintycalculation.h"
#include "windowscaptionhelper.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QList>
#include <QLocale>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QString>
#include <QUrl>
#include <QWindow>

void setApplicationData( QCoreApplication *app ) {
    app->setApplicationName( APP_NAME );
    app->setApplicationVersion( APP_VERSION );
    app->setOrganizationName( ORG_NAME );
    app->setOrganizationDomain( ORG_DOMAIN );
}

int main( int argc, char *argv[] ) {
    // The locale is set to US to have the number format (decimal . and thousand
    // separator ,) consistent wih exprtk
    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    bool headless { CommandLineInterface::headless( argc, argv ) };

    if ( !headless ) {
        QGuiApplication app { argc, argv };
        setApplicationData( &app );
        app.setApplicationDisplayName( APP_DISPLAY_NAME );

        ApplicationSettings appSettings { &app };
        UncertaintyCalculation calculation { &app, &appSettings };
        CommandLineInterface CLI { &app, &calculation };

        // Start the GUI
        // Load the persistent settings (for GUI mode only)
        appSettings.load();
        // Create connection to save the persistent settings before quiting the
        // main event loop
        QObject::connect(
            &app,
            &QGuiApplication::aboutToQuit,
            &appSettings,
            [ &appSettings ]() { appSettings.save(); },
            Qt::DirectConnection
        );

        // Start the QML UI
        QQmlApplicationEngine engine;
        // Process the CLI settings and commands when the UI is ready
        QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreated,
            &app,
            [ & ]( QObject *object, const QUrl &url ) {
                if ( !object ) {
                    Q_UNUSED( url )
                    // QML failed to load — exit with error
                    QGuiApplication::exit(
                        static_cast<int>( ExitCode::QmlLoadFailure )
                    );
                    return;
                }
                // UI is ready — apply CLI settings and commands, exit on error
                int exitCode { CLI.process() };
                if ( exitCode != 0 ) {
                    QGuiApplication::exit( exitCode );
                    return;
                }

                // Restore the last project when setting 'Restore last project
                // on startup' is set and no project is loaded yet from the CLI
                // using --open or --from-json
                if (
                    appSettings.getRestoreLastProject() &&
                    !CLI.getProjectLoaded()
                ) {
                    QUrl lastUrl { appSettings.getLastProjectFilePath() };
                    calculation.loadProject( lastUrl );
                }
            },
            Qt::QueuedConnection
        );

        WindowsCaptionHelper captionHelper { &app };

        QQmlContext *rootContext { engine.rootContext() };
        rootContext->setContextProperty( "calculation", &calculation );
        rootContext->setContextProperty( "appSettings", &appSettings );
        rootContext->setContextProperty( "captionHelper", &captionHelper );

        // QObject ownership should remain with C++
        QList<QObject *> objects {
            calculation.inputItemModel(),
            calculation.outputItemModel(),
            calculation.correlationItemModel(),
            calculation.resultsItemModel(),
            calculation.budgetItemModel(),
            calculation.distributionsModel(),
            calculation.unitsModel(),
            calculation.undoHistoryModel(),
            calculation.inputSelectionModel(),
            calculation.outputSelectionModel(),
            calculation.correlationSelectionModel()
        };
        for ( QObject *object : objects ) {
            engine.setObjectOwnership( object, QQmlEngine::CppOwnership );
        }

        engine.loadFromModule( "Sigma", "Main" );

        return app.exec();
    }
    else {
        // Headless mode
        QCoreApplication app { argc, argv };
        setApplicationData( &app );

        ApplicationSettings appSettings { &app };
        UncertaintyCalculation calculation { &app, &appSettings };
        CommandLineInterface CLI { &app, &calculation };

        // Apply CLI settings and commands
        return CLI.process();
    }
}
