// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "statistics.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <QDebug>


Statistics::Statistics( const double &p, const std::vector<double> &q )
    :   mP { p },
        mQ { q },
        mMean {},
        mStdDev {},
        mStdDevOfTheMean {},
        mLowerBound {},
        mHigherBound {},
        mHistogramValues { QList<double> ( mNumBins, 0. ) },
        mHistogramYMax { 1. },
        mHistogramXMin { -1. },
        mHistogramXMax { 1. },
        mHistogramLowerIndex { -1 },
        mHistogramHigherIndex { mNumBins }
{}


Statistics::Statistics( const Statistics &stat )
    :   mP { stat.getP() },
        mQ { stat.getSamples() },
        mMean { stat.getMean() },
        mStdDev { stat.getStdDev() },
        mStdDevOfTheMean { stat.getStdDevOfTheMean() },
        mLowerBound { stat.getLowerBound() },
        mHigherBound { stat.getHigherBound() },
        mHistogramValues { stat.getHistogramValues() },
        mHistogramYMax { stat.getHistogramYMax() },
        mHistogramXMin { stat.getHistogramXMin() },
        mHistogramXMax { stat.getHistogramXMax() },
        mHistogramLowerIndex { stat.getHistogramLowerIndex() },
        mHistogramHigherIndex { stat.getHistogramHigherIndex() }
{}


Statistics::~Statistics() {}


Statistics& Statistics::operator= ( const Statistics &stat ) {
    if ( this == &stat ) {
        return *this;
    }

    mP = stat.getP();
    mQ = stat.getSamples();
    mMean = stat.getMean();
    mStdDev = stat.getStdDev();
    mStdDevOfTheMean = stat.getStdDevOfTheMean();
    mLowerBound = stat.getLowerBound();
    mHigherBound = stat.getHigherBound();
    mHistogramValues = stat.getHistogramValues();
    mHistogramYMax = stat.getHistogramYMax();
    mHistogramXMin = stat.getHistogramXMin();
    mHistogramXMax = stat.getHistogramXMax();
    mHistogramLowerIndex = stat.getHistogramLowerIndex();
    mHistogramHigherIndex = stat.getHistogramHigherIndex();

    return *this;
}


QList<double> Statistics::getHistogramValues() const {
    return mHistogramValues;
}


double Statistics::getHigherBound() const {
    return mHigherBound;
}


double Statistics::getHistogramXMax() const {
    return mHistogramXMax;
}


double Statistics::getHistogramXMin() const {
    return mHistogramXMin;
}


double Statistics::getHistogramYMax() const {
    return mHistogramYMax;
}


double Statistics::getLowerBound() const {
    return mLowerBound;
}


double Statistics::getMean() const {
    return mMean;
}


double Statistics::getP() const {
    return mP;
}


double Statistics::getStdDev() const {
    return mStdDev;
}


double Statistics::getStdDevOfTheMean() const {
    return mStdDevOfTheMean;
}


int Statistics::getHistogramHigherIndex() const {
    return mHistogramHigherIndex;
}


int Statistics::getHistogramLowerIndex() const {
    return mHistogramLowerIndex;
}


int Statistics::getNumberOfSamples() const {
    return mQ.size();
}


std::vector<double> Statistics::getSamples() const {
    return mQ;
}


void Statistics::addSample( const double &sample ) {
    mQ.push_back( sample );
}


void Statistics::addSamples( const std::vector<double> &q ) {
    mQ.insert( mQ.end(), q.begin(), q.end() );
}


void Statistics::calculate( const bool &calculateHistogram ) {
    size_t n { mQ.size() };
    if ( n > 0 ) {
        mMean = std::accumulate(
                    mQ.begin(), mQ.end(), 0.
                ) / static_cast<double> ( n );

        // Lambda function to calculate the sum of the squares of the element
        // difference with respect to the mean.
        auto divSqr = [ this ]( const double &acc, const double &i ) {
            return acc + std::pow( i - this->mMean, 2 );
        };

        // Calculate the variance.
        double var {
            std::accumulate(
                mQ.begin(), mQ.end(), 0., divSqr
            ) / static_cast<double> ( n - 1 )
        };

        // Standard deviation is the square root of the variance.
        mStdDev = std::sqrt( var );

        // Standard deviation of the mean is the square root of the variance
        // divided by the number of samples.
        mStdDevOfTheMean = std::sqrt( var / static_cast<double> ( n ) );

        // The lower and higher bounds of the probabilistically symmetric 100p %
        // coverage interval are determined by sorting the vector of samples and
        // calculating the required number of samples outside the interval based
        // on p.
        std::sort( mQ.begin(), mQ.end() );
        size_t q {
            static_cast<size_t> (
                std::lround( std::abs( mP ) * static_cast<double> ( n ) )
            )
        };
        q = q > n ? n : q;
        size_t l {
            static_cast<size_t> (
                std::lround( static_cast<double> ( n - q ) / 2. )
            )
        };
        l = l < 1 ? 1 : l;
        size_t h { l + q > n ? n : l + q };
        mLowerBound = mQ[ l - 1 ];
        mHigherBound = mQ[ h - 1 ];

        // Histogram calculation
        if ( calculateHistogram ) {
            // Determine the bin size based on the minimum and maximum sample
            // value and the number of bins.
            mHistogramXMin = *mQ.begin();
            mHistogramXMax = *( mQ.end() - 1 );
            double binSize {
                ( mHistogramXMax - mHistogramXMin ) /
                static_cast<double> ( mNumBins )
            };


            if ( binSize > 0. ) {
                mHistogramValues.fill( 0. );

                // Each of the n samples adds 1/n to the probability of a bin.
                const double invN { 1. / static_cast<double> ( n ) };
                int i {};
                for ( const double &sample : mQ ) {
                    i = std::floor( ( sample - mHistogramXMin ) / binSize );
                    // The sample that is equal to the right side of the last
                    // bin (i.e the maximum) is added to the last bin.
                    i = i == mNumBins ? i - 1 : i;
                    mHistogramValues[ i ] += invN;
                }

                // Ymax is used to set the limit on the Y-axis of the histogram
                // plot.
                mHistogramYMax = *std::max_element(
                                     mHistogramValues.begin(),
                                     mHistogramValues.end()
                                 );

                // Lower and higher index are used to determine the color of the
                // bins outside of the coverage interval.
                mHistogramLowerIndex = static_cast<int> (
                                          std::floor(
                                              ( mLowerBound - mHistogramXMin ) /
                                              binSize
                                          )
                                       );
                mHistogramHigherIndex = static_cast<int> (
                                          std::floor(
                                              ( mHigherBound - mHistogramXMin) /
                                              binSize
                                          ) + 1
                                        );
            }
            else {
                // All samples are equal
                mHistogramYMax = 1. / mNumBins;
                mHistogramValues.fill( mHistogramYMax );
            }
        }
    }
}


void Statistics::clearSamples() {
    mQ.clear();
    mMean = 0.;
    mStdDev = 0.;
    mStdDevOfTheMean = 0.;
    mLowerBound = 0.;
    mHigherBound = 0.;
    mHistogramValues.fill( 0. );
    mHistogramYMax = 1.;
}


void Statistics::setP( const double &p ) {
    mP = p;
}
