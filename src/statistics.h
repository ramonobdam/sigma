// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef STATISTICS_H
#define STATISTICS_H

#include <QList>
#include <vector>

// The Statistics class stores samples and determines the statistics of the set.
// The samples can also be binned in a histogram.
class Statistics {

public:
    Statistics(
        const double &p = 0.,
        const std::vector<double> &q = std::vector<double>()
    );
    Statistics( const Statistics &stat );
    ~Statistics();

    Statistics& operator= ( const Statistics &stat );

    QList<double> getHistogramValues() const;
    double getHigherBound() const;
    double getHistogramXMax() const;
    double getHistogramXMin() const;
    double getHistogramYMax() const;
    double getLowerBound() const;
    double getMean() const;
    double getP() const;
    double getStdDev() const;
    double getStdDevOfTheMean() const;
    int getHistogramHigherIndex() const;
    int getHistogramLowerIndex() const;
    int getNumberOfSamples() const;
    std::vector<double> getSamples() const;
    void addSample( const double &sample );
    void addSamples( const std::vector<double> &q );
    void calculate( const bool &calculateHistogram = false );
    void clearSamples();
    void setP( const double &p );

private:
    const int mNumBins { 71 };
    QList<double> mHistogramValues;
    double mHigherBound;
    double mHistogramXMax;
    double mHistogramXMin;
    double mHistogramYMax;
    double mLowerBound;
    double mMean;
    double mP;
    double mStdDev;
    double mStdDevOfTheMean;
    int mHistogramHigherIndex;
    int mHistogramLowerIndex;
    std::vector<double> mQ;
};

#endif // STATISTICS_H
