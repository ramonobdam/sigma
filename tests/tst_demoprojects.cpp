// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "regressionconfig.h"
#include "resultcomparator.h"
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>

// Regression test that automates the validation of the demo projects
// end-to-end. Every demo project is processed by the Sigma command-line
// interface with the options defined in the JSON configuration file
// (demo_projects.json). The resulting project file (.sig) and the exported
// results (.csv) are compared with the reference results of a previous version,
// using the numerical tolerances of the configuration file.

// The tests are executed in the order in which they are declared:
//   runProject         runs the demo projects with the Sigma executable
//   compareProjectFile compares the saved project files
//   compareCSVFile     compares the exported CSV files

// The following environment variables can be used to override the defaults
// that are set by CMake:
//   SIGMA_EXECUTABLE         path of the Sigma executable
//   SIGMA_REGRESSION_CONFIG  path of the JSON configuration file
class tst_demoProjects : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();

    void runProject_data();
    void runProject();

    void compareProjectFile_data();
    void compareProjectFile();

    void compareCSVFile_data();
    void compareCSVFile();

private:
    ProjectConfig getProjectConfig( const QString &name ) const;
    QString getReferenceCSVFile( const QString &name ) const;
    QString getReferenceProjectFile( const QString &name ) const;
    QString getWorkCSVFile( const QString &name ) const;
    QString getWorkProjectFile( const QString &name ) const;
    ResultComparator createComparator( const QString &name ) const;
    bool copyProjectToWorkDirectory( const QString &name, QString &error );
    void addProjectRows();

    static QString environmentPath(
        const char *variable,
        const QString &fallback
    );

    static constexpr QLatin1StringView sCSVDirectoryName { "CSV" };
    static constexpr QLatin1StringView sProjectExtension { ".sig" };
    static constexpr QLatin1StringView sCSVExtension { ".csv" };

    QSet<QString> mFailedProjects;
    QString mConfigFile;
    QString mDemoProjectsDirectory;
    QString mReferenceDirectory;
    QString mSigmaExecutable;
    QTemporaryDir mWorkDirectory;
    RegressionConfig mConfig;
    bool mKeepWorkDirectory { false };
};


void tst_demoProjects::initTestCase() {
    // Resolve the paths, which are set by CMake and can be overruled by
    // environment variables
    mConfigFile = environmentPath(
        "SIGMA_REGRESSION_CONFIG",
        QString( SIGMA_REGRESSION_CONFIG )
    );

    mSigmaExecutable = environmentPath(
        "SIGMA_EXECUTABLE",
        QString( SIGMA_EXECUTABLE )
    );

    QVERIFY2(
        mConfig.load( mConfigFile ),
        qPrintable( mConfig.getError() )
    );

    mDemoProjectsDirectory = environmentPath(
        "SIGMA_DEMO_PROJECTS_DIR",
        mConfig.getDemoProjectsDirectory()
    );


    mReferenceDirectory = environmentPath(
        "SIGMA_REFERENCE_DIR",
        mConfig.getReferenceDirectory()
    );

    QVERIFY2(
        QFileInfo { mSigmaExecutable }.isExecutable(),
        qPrintable( "Sigma executable not found: " + mSigmaExecutable )
    );
    QVERIFY2(
        QFileInfo { mDemoProjectsDirectory }.isDir(),
        qPrintable(
            "Demo projects directory not found: " + mDemoProjectsDirectory
        )
    );
    QVERIFY2(
        QFileInfo { mReferenceDirectory }.isDir(),
        qPrintable(
            "Reference directory not found: " + mReferenceDirectory
        )
    );
    QVERIFY2(
        mWorkDirectory.isValid(),
        qPrintable( mWorkDirectory.errorString() )
    );
    QVERIFY(
        QDir { mWorkDirectory.path() }.mkpath( sCSVDirectoryName )
    );

    qInfo().noquote() << "Sigma executable:  " << mSigmaExecutable;
    qInfo().noquote() << "Configuration:     " << mConfigFile;
    qInfo().noquote() << "Demo projects:     " << mDemoProjectsDirectory;
    qInfo().noquote() << "Reference results: " << mReferenceDirectory;
    qInfo().noquote() << "Working directory: " << mWorkDirectory.path();
}


void tst_demoProjects::cleanup() {
    // Keep the results of a failed test for manual inspection
    if ( QTest::currentTestFailed() ) {
        mKeepWorkDirectory = true;
    }
}


void tst_demoProjects::cleanupTestCase() {
    if ( mKeepWorkDirectory && mWorkDirectory.isValid() ) {
        mWorkDirectory.setAutoRemove( false );
        qWarning().noquote()
            << "Results of the failed tests are kept in:"
            << mWorkDirectory.path();
    }
}


void tst_demoProjects::runProject_data() {
    addProjectRows();
}


void tst_demoProjects::runProject() {
    QFETCH( QString, project );

    const ProjectConfig config { getProjectConfig( project ) };
    const QString projectFile { getWorkProjectFile( project ) };
    const QString csvFile { getWorkCSVFile( project ) };

    // Work on a copy, so that the demo projects in the repository are not
    // modified by the test
    QString error {};
    if ( !copyProjectToWorkDirectory( project, error ) ) {
        mFailedProjects.insert( project );
        QFAIL( qPrintable( error ) );
    }

    // A project can require multiple runs, e.g. to simulate a single output
    // parameter with a different number of significant digits
    for ( const RunConfig &run : config.runs ) {
        QStringList arguments {
            "--headless",
            "--open", projectFile,
            "--save", projectFile,
            "--export", csvFile
        };
        arguments.append( run.arguments );

        QProcess process {};
        process.setWorkingDirectory( mWorkDirectory.path() );
        process.start( mSigmaExecutable, arguments );

        if ( !process.waitForStarted( mConfig.getTimeoutSeconds() * 1000 ) ) {
            mFailedProjects.insert( project );
            QFAIL(
                qPrintable(
                    "Could not start " + mSigmaExecutable + ": " +
                    process.errorString()
                )
            );
        }

        if ( !process.waitForFinished( mConfig.getTimeoutSeconds() * 1000 ) ) {
            process.kill();
            process.waitForFinished();
            mFailedProjects.insert( project );
            QFAIL(
                qPrintable(
                    QString( "Timeout after %1 s: %2 %3" ).arg(
                        QString::number( mConfig.getTimeoutSeconds() ),
                        mSigmaExecutable,
                        arguments.join( " " )
                    )
                )
            );
        }

        const QString output {
            QString::fromUtf8( process.readAllStandardError() ).trimmed()
        };

        if ( process.exitStatus() != QProcess::NormalExit ||
             process.exitCode() != 0 ) {
            mFailedProjects.insert( project );
            QFAIL(
                qPrintable(
                    QString( "Exit code %1 for: %2\n%3" ).arg(
                        QString::number( process.exitCode() ),
                        arguments.join( " " ),
                        output
                    )
                )
            );
        }
    }

    QVERIFY2(
        QFileInfo::exists( projectFile ),
        qPrintable( "Project file not saved: " + projectFile )
    );
    QVERIFY2(
        QFileInfo::exists( csvFile ),
        qPrintable( "CSV file not exported: " + csvFile )
    );
}


void tst_demoProjects::compareProjectFile_data() {
    addProjectRows();
}


void tst_demoProjects::compareProjectFile() {
    QFETCH( QString, project );

    if ( mFailedProjects.contains( project ) ) {
        QSKIP( "The project could not be run" );
    }

    const QString referenceFile { getReferenceProjectFile( project ) };
    QVERIFY2(
        QFileInfo::exists( referenceFile ),
        qPrintable( "Reference project file not found: " + referenceFile )
    );

    ResultComparator comparator { createComparator( project ) };
    QVERIFY2(
        comparator.compareProjectFiles(
            getWorkProjectFile( project ),
            referenceFile
        ),
        qPrintable( comparator.getDifferenceReport() )
    );
}


void tst_demoProjects::compareCSVFile_data() {
    addProjectRows();
}


void tst_demoProjects::compareCSVFile() {
    QFETCH( QString, project );

    if ( mFailedProjects.contains( project ) ) {
        QSKIP( "The project could not be run" );
    }

    const QString referenceFile { getReferenceCSVFile( project ) };
    QVERIFY2(
        QFileInfo::exists( referenceFile ),
        qPrintable( "Reference CSV file not found: " + referenceFile )
    );

    ResultComparator comparator { createComparator( project ) };
    QVERIFY2(
        comparator.compareCSVFiles(
            getWorkCSVFile( project ),
            referenceFile
        ),
        qPrintable( comparator.getDifferenceReport() )
    );
}


ProjectConfig tst_demoProjects::getProjectConfig( const QString &name ) const {
    const QList<ProjectConfig> projects { mConfig.getProjects() };
    for ( const ProjectConfig &project : projects ) {
        if ( project.name == name ) {
            return project;
        }
    }
    return ProjectConfig {};
}


QString tst_demoProjects::getReferenceCSVFile( const QString &name ) const {
    return mReferenceDirectory + "/" + sCSVDirectoryName + "/" + name +
           sCSVExtension;
}


QString tst_demoProjects::getReferenceProjectFile( const QString &name ) const
{
    return mReferenceDirectory + "/" + name + sProjectExtension;
}


QString tst_demoProjects::getWorkCSVFile( const QString &name ) const {
    return mWorkDirectory.path() + "/" + sCSVDirectoryName + "/" + name +
           sCSVExtension;
}


QString tst_demoProjects::getWorkProjectFile( const QString &name ) const {
    return mWorkDirectory.path() + "/" + name + sProjectExtension;
}


ResultComparator tst_demoProjects::createComparator(
    const QString &name
) const {
    // The tolerances of the project are used, which fall back to the default
    // tolerances of the configuration file
    ResultComparator comparator { getProjectConfig( name ).tolerance };
    comparator.setIgnoredCSVLinePrefixes(
        mConfig.getIgnoredCSVLinePrefixes()
    );
    comparator.setIgnoredJsonKeys( mConfig.getIgnoredJsonKeys() );
    return comparator;
}


bool tst_demoProjects::copyProjectToWorkDirectory(
    const QString &name,
    QString &error
){
    const QString source {
        mDemoProjectsDirectory + "/" + name + sProjectExtension
    };
    const QString destination { getWorkProjectFile( name ) };

    if ( !QFileInfo::exists( source ) ) {
        error = "Demo project not found: " + source;
        return false;
    }

    if ( QFile::exists( destination ) && !QFile::remove( destination ) ) {
        error = "Could not remove: " + destination;
        return false;
    }

    if ( !QFile::copy( source, destination ) ) {
        error = "Could not copy " + source + " to " + destination;
        return false;
    }

    // The copy of a read-only demo project must be writable, since the project
    // is saved after the Monte Carlo simulation
    QFile file { destination };
    if ( !file.setPermissions(
            QFile::ReadOwner | QFile::WriteOwner |
            QFile::ReadGroup | QFile::ReadOther
        ) ) {
        error = "Could not set the permissions of: " + destination;
        return false;
    }

    return true;
}


void tst_demoProjects::addProjectRows() {
    // Add a data row for every project of the configuration file
    QTest::addColumn<QString>( "project" );

    const QList<ProjectConfig> projects { mConfig.getProjects() };
    for ( const ProjectConfig &project : projects ) {
        QTest::newRow( qPrintable( project.name ) ) << project.name;
    }
}


QString tst_demoProjects::environmentPath(
    const char *variable,
    const QString &fallback
){
    const QByteArray value { qgetenv( variable ) };
    return value.isEmpty() ? fallback : QString::fromLocal8Bit( value );
}


QTEST_GUILESS_MAIN( tst_demoProjects )

#include "tst_demoprojects.moc"
