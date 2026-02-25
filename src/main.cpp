// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "applicationsettings.h"
#include "uncertaintycalculation.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QLocale>
#include <QObject>
#include <QList>
#include <QUrl>

int main( int argc, char *argv[] ) {
    // The locale is set to US to have the number format (decimal . and thousand
    // separator ,) consistent wih exprtk
    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );

    QGuiApplication app( argc, argv );
    app.setApplicationName( "Sigma - Measurement Uncertainty Toolkit" );
    app.setApplicationVersion( "1.0.0" );
    app.setOrganizationName( "Tiger Suit Interactive" );
    app.setOrganizationDomain( "TigerSuitInteractive.org" );
    app.setWindowIcon(
        QIcon( "qrc:/qt/qml/Sigma/resources/icons/Icon1024_light.png" )
    );

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit( -1 ); },
        Qt::QueuedConnection );

    ApplicationSettings *appSettings { new ApplicationSettings( &app ) };
    UncertaintyCalculation *calculation { new UncertaintyCalculation( &app ) };

    // Load a project based on the command-line argument or restore the last
    // project
    if ( argc > 1 ) {
        QUrl url { argv[ 1 ] };
        calculation->loadProject( url );
    }
    else if ( appSettings->getRestoreLastProject() ) {
        QUrl url { appSettings->getLastProjectFilePath() };
        calculation->loadProject( url );
    }

    QQmlContext *rootContext { engine.rootContext() };
    rootContext->setContextProperty( "calculation", calculation );
    rootContext->setContextProperty( "appSettings", appSettings );

    // QObject ownership should remain with C++
    QList<const QObject *> objects {
        calculation->inputItemModel(),
        calculation->outputItemModel(),
        calculation->inputSelectionModel(),
        calculation->outputSelectionModel(),
        calculation->getQMLInputParameter(),
        calculation->getQMLOutputParameter()
    };
    for ( const QObject *object : objects ) {
        engine.setObjectOwnership(
            const_cast<QObject *>( object ),
            QQmlEngine::CppOwnership
        );
    }

    engine.loadFromModule( "Sigma", "Main" );

    return app.exec();
}
