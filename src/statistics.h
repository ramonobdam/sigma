// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef STATISTICS_H
#define STATISTICS_H

#include <QList>
#include <vector>

// The Statistics class stores samples and determines the statistics of the set.
// The samples can also be binned in a histogram. To improve performance, the
// statistics are calculated lazily only when requested and the stored
// statistics are no longer valid because the data set has changed.
class Statistics {

public:
    Statistics(
        double p = 0.,
        const std::vector<double> &q = std::vector<double>()
    );

    const QList<double> &getHistogramValues();
    double getHigherBound();
    double getHistogramXMax();
    double getHistogramXMin();
    double getHistogramYMax();
    double getLowerBound();
    double getMean();
    double getP() const;
    double getStdDev();
    double getStdDevOfTheMean();
    int getHistogramHigherIndex();
    int getHistogramLowerIndex();
    int getNumberOfSamples() const;
    const std::vector<double> &getSamples() const;
    void addSample( double sample );
    void addSamples( const std::vector<double> &q );
    void clearSamples();
    void reserveCapacity( size_t n );
    void setP( double p );

private:
    static constexpr int sNumBins { 71 };

    void calculateBounds();
    void calculateHistogram();
    void calculateMeanAndStdDevs();
    void resetBounds();
    void resetHistogram();
    void resetMeanAndStdDevs();
    void sortSamples();

    QList<double> mHistogramValues;
    bool mBoundsCalculated;
    bool mHistogramCalculated;
    bool mStdDevsCalculated;
    bool mSamplesSorted;
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
