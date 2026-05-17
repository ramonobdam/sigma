// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "statistics.h"
#include <algorithm>
#include <cmath>
#include <numeric>


Statistics::Statistics( double p, const std::vector<double> &q )
    :   mP { p },
        mQ { q },
        mBoundsCalculated { false },
        mHistogramCalculated { false },
        mStdDevsCalculated { false },
        mSamplesSorted { false }
{
    resetMeanAndStdDevs();
    resetBounds();
    resetHistogram();
}


const QList<double> & Statistics::getHistogramValues() {
    calculateHistogram();
    return mHistogramValues;
}


double Statistics::getHigherBound() {
    calculateBounds();
    return mHigherBound;
}


double Statistics::getHistogramXMax() {
    calculateHistogram();
    return mHistogramXMax;
}


double Statistics::getHistogramXMin() {
    calculateHistogram();
    return mHistogramXMin;
}


double Statistics::getHistogramYMax() {
    calculateHistogram();
    return mHistogramYMax;
}


double Statistics::getLowerBound() {
    calculateBounds();
    return mLowerBound;
}


double Statistics::getMean() {
    calculateMeanAndStdDevs();
    return mMean;
}


double Statistics::getP() const {
    return mP;
}


double Statistics::getStdDev() {
    calculateMeanAndStdDevs();
    return mStdDev;
}


double Statistics::getStdDevOfTheMean() {
    calculateMeanAndStdDevs();
    return mStdDevOfTheMean;
}


int Statistics::getHistogramHigherIndex() {
    calculateHistogram();
    return mHistogramHigherIndex;
}


int Statistics::getHistogramLowerIndex() {
    calculateHistogram();
    return mHistogramLowerIndex;
}


int Statistics::getNumberOfSamples() const {
    return static_cast<int>( mQ.size() );
}


const std::vector<double> & Statistics::getSamples() const {
    return mQ;
}


void Statistics::addSample( double sample ) {
    mQ.push_back( sample );
    mBoundsCalculated = false;
    mHistogramCalculated = false;
    mStdDevsCalculated = false;
    mSamplesSorted = false;
}


void Statistics::addSamples( const std::vector<double> &q ) {
    mQ.insert( mQ.end(), q.begin(), q.end() );
    mBoundsCalculated = false;
    mHistogramCalculated = false;
    mStdDevsCalculated = false;
    mSamplesSorted = false;
}


void Statistics::clearSamples() {
    mQ.clear();
    resetMeanAndStdDevs();
    resetBounds();
    resetHistogram();

    // No need to calculate or sort when there are no samples:
    mBoundsCalculated = true;
    mHistogramCalculated = true;
    mStdDevsCalculated = true;
    mSamplesSorted = true;
}


void Statistics::reserveCapacity( size_t n ) {
    mQ.reserve( n );
}


void Statistics::setP( double p ) {
    if ( p != mP ) {
        mP = p;
        mBoundsCalculated = false;
        mHistogramCalculated = false;
    }
}


void Statistics::calculateBounds() {
    if ( mBoundsCalculated ) return;

    size_t n { mQ.size() };
    if ( n > 0 ) {
        // The lower and higher bounds of the probabilistically symmetric 100p %
        // coverage interval are determined by sorting the vector of samples and
        // calculating the required number of samples outside the interval based
        // on p.
        sortSamples();
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
    }
    else {
        resetBounds();
    }

    mBoundsCalculated = true;
}


void Statistics::calculateHistogram() {
    if ( mHistogramCalculated ) return;

    size_t n { mQ.size() };
    if ( n > 0 ) {
        // Determine the bin size based on the minimum and maximum sample value
        // and the number of bins
        sortSamples();
        mHistogramXMin = mQ.front();
        mHistogramXMax = mQ.back();
        double binSize {
            ( mHistogramXMax - mHistogramXMin ) /
            static_cast<double> ( sNumBins )
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
                i = i == sNumBins ? i - 1 : i;
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
            calculateBounds();
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
            mHistogramYMax = 1. / sNumBins;
            mHistogramValues.fill( mHistogramYMax );
        }
    }
    else {
        resetHistogram();
    }

    mHistogramCalculated = true;
}


void Statistics::calculateMeanAndStdDevs() {
    if ( mStdDevsCalculated ) return;

    size_t n { mQ.size() };
    if ( n > 0 ) {
        mMean = std::accumulate( mQ.begin(), mQ.end(), 0. ) /
                static_cast<double> ( n );

        // Calculate standard deviation (of the mean)
        mStdDev = 0.;
        mStdDevOfTheMean = 0.;
        if ( n > 1 ) {
            // Lambda function to calculate the sum of the squares of the
            // element difference with respect to the mean
            auto divSqr = [ this ]( double acc, double i ) {
                return acc + std::pow( i - this->mMean, 2 );
            };

            // Calculate the variance
            double var {
                std::accumulate( mQ.begin(), mQ.end(), 0., divSqr ) /
                static_cast<double> ( n - 1 )
            };

            // Standard deviation is the square root of the variance
            mStdDev = std::sqrt( var );

            // Standard deviation of the mean is the square root of the variance
            // divided by the number of samples
            mStdDevOfTheMean = std::sqrt( var / static_cast<double> ( n ) );
        }
    }
    else {
        resetMeanAndStdDevs();
    }

    mStdDevsCalculated = true;
}


void Statistics::resetBounds() {
    mLowerBound = 0.;
    mHigherBound = 0.;
}


void Statistics::resetHistogram() {
    mHistogramValues.fill( 0., sNumBins );
    mHistogramYMax = 1.;
    mHistogramXMin = -1.;
    mHistogramXMax = 1.;
    mHistogramLowerIndex = -1 ;
    mHistogramHigherIndex = sNumBins;
}


void Statistics::resetMeanAndStdDevs() {
    mMean = 0.;
    mStdDev = 0.;
    mStdDevOfTheMean = 0.;
}


void Statistics::sortSamples() {
    if ( mSamplesSorted ) return;

    std::sort( mQ.begin(), mQ.end() );
    mSamplesSorted = true;
}
