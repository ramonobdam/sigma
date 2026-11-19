// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "samplewriter.h"
#include "statistics.h"
#include <QJsonObject>
#include <QLatin1StringView>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QUrl>

// Forward declaration of OutputParameter (to avoid recursion loop)
class OutputParameter;

// Class that performs the Monte Carlo simulation and stores the results. The
// calculations are performed in a separate thread using QThread.
class MonteCarlo: public QThread {
    Q_OBJECT

public:
    MonteCarlo(
        OutputParameter *outputParameter = nullptr,
        QObject *parent = nullptr
    );
    ~MonteCarlo();

    MonteCarlo( const MonteCarlo &mc, QObject *parent = nullptr );
    MonteCarlo & operator= ( const MonteCarlo &mc );

    OutputParameter* getOutputParameter() const;
    QJsonObject toJson() const;
    QList<double> getHistogramValues() const;
    QString getExpandedUncertaintyAsString( bool csvMode = false ) const;
    QString getMeanAsString( bool csvMode = false ) const;
    QString getNumericalToleranceAsString() const;
    QString getStatus( bool csvMode = false ) const;
    QString getStdDeviationAsString( bool csvMode = false ) const;
    QString histogramToCSVString() const;
    Statistics getOutputStatistics() const;
    bool getRequestStop() const;
    bool getSamplesExported() const;
    // The result of the sample export of the last run, empty when the last
    // run didn't export samples
    const QString &getSampleExportMessage() const;
    bool getValid() const;
    double getConvergenceFactor() const;
    double getHigherBound() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getLowerBound() const;
    double getMean() const;
    double getNumericalTolerance() const;
    double getStdDeviation() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    void resetResults();
    void run() override;
    void setConvergenceFactor( double convergenceFactor = 0. );
    void setHigherBound( double higherBound = 0. );
    void setHistogramHigherIndex( int histogramHigherIndex = 0 );
    void setHistogramLowerIndex( int histogramLowerIndex = -1 );
    void setHistogramValues( const QList<double> &values = QList<double>() );
    void setHistogramXMax( double histogramXMax = 1. );
    void setHistogramXMin( double histogramXMin = -1. );
    void setHistogramYMax( double histogramYMax = 1. );
    void setLowerBound( double lowerBound = 0. );
    void setMean( double mean = 0. );
    void setNumericalTolerance( double tolerance );
    void setOutputParameter( OutputParameter *outputParameter );
    void setOutputStatistics( const Statistics &outputStat );
    void setRequestStop( bool requestStop );
    // Sets the CSV file the samples of the next run are exported to. An empty
    // url means no export. The url is used by one run only, it is cleared
    // when the run starts. Returns false while a simulation is running.
    bool setSampleExportUrl( const QUrl &url );
    void setStatus( const QString &status = sMonteNotCarloStarted );
    void setStdDeviation( double stdDeviation = 0. );
    void setValid( bool valid = false );
    void stop();

    static MonteCarlo fromJson( const QJsonObject &json );

    inline static const QStringList headerLabels {
        "Output name",
        "Unit",
        "Simulation status",
        "Numerical tolerance",
        "Output estimate",
        "Standard uncertainty",
        "Expanded uncertainty",
        "Level of confidence"
    };

signals:
    void convergenceFactorChanged();
    void monteCarloStatusChanged();

private:
    QString getHistogramBinsAsString() const;
    QString getHistogramValuesAsString() const;
    void calculateNumericalTolerance( double value, int digits );
    bool openSampleWriter( SampleWriter &writer, const QUrl &url );

    OutputParameter *mOutputParameter;
    QList<double> mHistogramValues;
    QMutex mMutex;
    QString mStatus;
    Statistics mOutputStat;
    bool mRequestStop;
    bool mValid;
    double mConvergenceFactor;
    double mHigherBound;
    double mHistogramXMax;
    double mHistogramXMin;
    double mHistogramYMax;
    double mLowerBound;
    double mMean;
    double mNumericalTolerance;
    double mStdDeviation;
    int mHistogramHigherIndex;
    int mHistogramLowerIndex;

    // The sample export state belongs to a single run: it is deliberately not
    // copied by the copy constructor and assignment operator
    QUrl mSampleExportUrl;
    QString mSampleExportMessage;
    bool mSamplesExported;

    static constexpr QLatin1StringView sConvergedString {
        "Converged after %L1 function evaluations"
    };
    static constexpr QLatin1StringView sMonteCarloRunning {
        "Monte Carlo Simulation running"
    };
    static constexpr QLatin1StringView sNotConvergedString {
        "Not converged after %L1 function evaluations"
    };
    static constexpr QLatin1StringView sStoppedString {
        "Stopped after %L1 function evaluations"
    };
    static constexpr QLatin1StringView sSamplesExportedString {
        "The Monte Carlo samples have been exported to '%1'"
    };
    static constexpr QLatin1StringView sSamplesNotExportedString {
        "The Monte Carlo samples could not be exported: %1"
    };

    static constexpr QLatin1StringView sHigherBoundString { "higherBound" };
    static constexpr QLatin1StringView sHistogramHigherIndexString {
        "histogramHigherIndex"
    };
    static constexpr QLatin1StringView sHistogramLowerIndexString {
        "histogramLowerIndex"
    };
    static constexpr QLatin1StringView sHistogramValuesString {
        "histogramValues"
    };
    static constexpr QLatin1StringView sHistogramXMaxString { "histogramXMax" };
    static constexpr QLatin1StringView sHistogramXMinString { "histogramXMin" };
    static constexpr QLatin1StringView sHistogramYMaxString { "histogramYMax" };
    static constexpr QLatin1StringView sLowerBoundString { "lowerBound" };
    static constexpr QLatin1StringView sMeanString { "mean" };
    static constexpr QLatin1StringView sMonteNotCarloStarted {
        "Click 'Start' to run Monte Carlo simulation"
    };
    static constexpr QLatin1StringView sNumericalToleranceString {
        "numericalTolerance"
    };
    static constexpr QLatin1StringView sStatusString { "status" };
    static constexpr QLatin1StringView sStdDeviationString { "stdDeviation" };
    static constexpr QLatin1StringView sValidString { "valid" };

    // Numerical tolerance reduction factor according to JCGM:101 section 8.2:
    // a tolerance of δ/5 is recommended for Monte Carlo results that are used
    // to validate the GUM uncertainty framework
    static constexpr double sNumericalToleranceFactor { 5. };

    // Minimum number of batches before convergence is checked. JCGM:101
    // section 7.9.4 requires at least 2, but the standard deviations of the
    // batch statistics are then estimated with only 1 degree of freedom and
    // can be strongly underestimated. With 10 batches (9 degrees of freedom)
    // premature convergence is unlikely. Before the minimum is reached, the
    // convergence factor (progress) is limited to the fraction of the minimum
    // number of batches that has been completed. Note that the lowest allowed
    // value of the maximum number of batches setting (--mcmaxbatches) is 10 as
    // well.
    static constexpr int sMinNumOfBatches { 10 };

    // Number of samples per chunk: the batches are split into chunks with
    // their own random seed, which are evaluated in parallel. Changing this
    // value changes the (reproducible) results.
    static constexpr int sChunkSize { 10000 };
};

#endif // MONTECARLO_H
