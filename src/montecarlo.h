// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "settings.h"
#include "statistics.h"
#include "utils.h"
#include <QJsonObject>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>

// Forward declaration of OutputParameter (to avoid recursion loop)
class OutputParameter;

// Class that performs the Monte Carlo simulation and stores the results. The
// calculations are performed in a separate thread using QThread.
class MonteCarlo: public QThread, private Settings, private Utils {
    Q_OBJECT

public:
    MonteCarlo(
        OutputParameter *outputParameter = nullptr,
        QObject *parent = nullptr
    );
    ~MonteCarlo();

    MonteCarlo( const MonteCarlo &mc, QObject *parent = nullptr );
    MonteCarlo& operator= ( const MonteCarlo &mc );

    OutputParameter* getOutputParameter() const;
    QJsonObject toJson() const;
    QList<double> getHistogramValues() const;
    QString getExpandedUncertaintyAsString( const bool &csvMode = false ) const;
    QString getMeanAsString( const bool &csvMode = false ) const;
    QString getNumericalToleranceAsString() const;
    QString getStatus( const bool &csvMode = false ) const;
    QString getStdDeviationAsString( const bool &csvMode = false ) const;
    QString histogramToString() const;
    Statistics getOutputStatistics() const;
    bool getRequestStop() const;
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
    void setConvergenceFactor( const double &convergenceFactor = 0. );
    void setHigherBound( const double &higherBound = 0. );
    void setHistogramHigherIndex( const int &histogramHigherIndex = 0 );
    void setHistogramLowerIndex( const int &histogramLowerIndex = -1 );
    void setHistogramValues( const QList<double> &values = QList<double>() );
    void setHistogramXMax( const double &histogramXMax = 1. );
    void setHistogramXMin( const double &histogramXMin = -1. );
    void setHistogramYMax( const double &histogramYMax = 1. );
    void setLowerBound( const double &lowerBound = 0. );
    void setMean( const double &mean = 0. );
    void setNumericalTolerance( const double &tolerance );
    void setOutputParameter( OutputParameter *outputParameter );
    void setOutputStatistics( const Statistics &outputStat );
    void setRequestStop( const bool &requestStop );
    void setStatus( const QString &status = mMonteNotCarloStarted );
    void setStdDeviation( const double &stdDeviation = 0. );
    void setValid( const bool &valid = false );
    void stop();

    static MonteCarlo fromJson( const QJsonObject &json );

    static QStringList headerLabels;

signals:
    void convergenceFactorChanged();
    void monteCarloStatusChanged();

private:
    QString getHistogramBinsAsString() const;
    QString getHistogramValuesAsString() const;
    void calculateNumericalTolerance( const double &value, const int &digits );

    const QString mConvergedString {
        "Converged after %L1 function evaluations"
    };
    const QString mInvalidOutputString {
        "Invalid output value for input parameter values: "
    };
    const QString mMonteCarloRunning { "Monte Carlo Simulation running" };
    const QString mNotConvergedString {
        "Not converged after %L1 function evaluations"
    };
    const QString mStoppedString { "Stopped after %L1 function evaluations" };

    // Numerical tolerance reduction factor according to JCGM:101 section 8.2:
    const double mNumericalToleranceFactor { 5. };

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

    static QString mHigherBoundString;
    static QString mHistogramHigherIndexString;
    static QString mHistogramLowerIndexString;
    static QString mHistogramValuesString;
    static QString mHistogramXMaxString;
    static QString mHistogramXMinString;
    static QString mHistogramYMaxString;
    static QString mLowerBoundString;
    static QString mMeanString;
    static QString mMonteNotCarloStarted;
    static QString mNumericalToleranceString;
    static QString mStatusString;
    static QString mStdDeviationString;
    static QString mValidString;
};

#endif // MONTECARLO_H
