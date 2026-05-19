// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "montecarlo.h"
#include "outputparameter.h"
#include "samplebatch.h"
#include "settings.h"
#include "stringutils.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFuture>
#include <QJsonArray>
#include <QJsonValue>
#include <QLocale>
#include <QtConcurrentMap>
#include <cmath>
#include <random>
#include <vector>

// Print Monte Carlo simulation timing values to console
#define PRINT_MONTECARLO_TIMING 0   // 0 = off, 1 = on

MonteCarlo::MonteCarlo( OutputParameter *outputParameter, QObject *parent )
    :   QThread { parent },
        mOutputParameter { outputParameter },
        mMean {},
        mStdDeviation {},
        mLowerBound {},
        mHigherBound {},
        mStatus {},
        mValid { true },
        mOutputStat {},
        mConvergenceFactor {},
        mNumericalTolerance { 0.01 },
        mMutex {},
        mHistogramValues {},
        mHistogramXMin { -1. },
        mHistogramXMax { 1. },
        mHistogramYMax { 1. },
        mHistogramLowerIndex {},
        mHistogramHigherIndex {},
        mRequestStop {}
{}


MonteCarlo::~MonteCarlo() {
    quit();
    wait();
}


MonteCarlo::MonteCarlo( const MonteCarlo &mc, QObject *parent )
    :   QThread { parent },
        mOutputParameter { mc.getOutputParameter() },
        mMean { mc.getMean() },
        mStdDeviation { mc.getStdDeviation() },
        mLowerBound { mc.getLowerBound() },
        mHigherBound { mc.getHigherBound() },
        mStatus { mc.getStatus() },
        mValid { mc.getValid() },
        mOutputStat { mc.getOutputStatistics() },
        mConvergenceFactor { mc.getConvergenceFactor() },
        mNumericalTolerance { mc.getNumericalTolerance() },
        mMutex {},
        mHistogramValues { mc.getHistogramValues() },
        mHistogramXMin { mc.getHistogramXMin() },
        mHistogramXMax { mc.getHistogramXMax() },
        mHistogramYMax { mc.getHistogramYMax() },
        mHistogramLowerIndex { mc.getHistogramLowerIndex() },
        mHistogramHigherIndex { mc.getHistogramHigherIndex() },
        mRequestStop { mc.getRequestStop() }
{}


MonteCarlo & MonteCarlo::operator= ( const MonteCarlo &mc ) {
    if ( this != &mc ) {
        setOutputParameter( mc.getOutputParameter() );
        setMean( mc.getMean() );
        setStdDeviation( mc.getStdDeviation() );
        setLowerBound( mc.getLowerBound() );
        setHigherBound( mc.getHigherBound() );
        setStatus( mc.getStatus() );
        setValid( mc.getValid() );
        setOutputStatistics( mc.getOutputStatistics() );
        setConvergenceFactor( mc.getConvergenceFactor() );
        setNumericalTolerance( mc.getNumericalTolerance() );
        setHistogramValues( mc.getHistogramValues() );
        setHistogramXMin( mc.getHistogramXMin() );
        setHistogramXMax( mc.getHistogramXMax() );
        setHistogramYMax( mc.getHistogramYMax() );
        setHistogramLowerIndex( mc.getHistogramLowerIndex() );
        setHistogramHigherIndex( mc.getHistogramHigherIndex() );
        setRequestStop( mc.getRequestStop() );
        setParent( mc.parent() );
    }
    return *this;
}


OutputParameter* MonteCarlo::getOutputParameter() const {
    return mOutputParameter;
}


QJsonObject MonteCarlo::toJson() const {
    QJsonArray valuesArray {};
    for ( const double &value : mHistogramValues ) {
        valuesArray.append( QJsonValue( value ) );
    }

    QJsonObject json {};
    json[ sNumericalToleranceString ] = getNumericalTolerance();
    json[ sMeanString ] = getMean();
    json[ sStdDeviationString ] = getStdDeviation();
    json[ sLowerBoundString ] = getLowerBound();
    json[ sHigherBoundString ] = getHigherBound();
    json[ sStatusString ] = getStatus();
    json[ sValidString ] = getValid();
    json[ sHistogramXMinString ] = getHistogramXMin();
    json[ sHistogramXMaxString ] = getHistogramXMax();
    json[ sHistogramYMaxString ] = getHistogramYMax();
    json[ sHistogramLowerIndexString ] = getHistogramLowerIndex();
    json[ sHistogramHigherIndexString ] = getHistogramHigherIndex();
    json[ sHistogramValuesString ] = valuesArray;

    return json;
}


QList<double> MonteCarlo::getHistogramValues() const {
    return mHistogramValues;
}


QString MonteCarlo::getExpandedUncertaintyAsString( bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    double lower { getLowerBound() - getMean() };
    double higher { getHigherBound() - getMean() };
    QString result {
        StringUtils::doubleToString( lower, precision ) +
        ", +" +
        StringUtils::doubleToString( higher, precision )
    };
    return csvMode ? StringUtils::addQuotes( result ) : result;
}


QString MonteCarlo::getMeanAsString( bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString( getMean(), precision );
}


QString MonteCarlo::getNumericalToleranceAsString() const {
    return StringUtils::doubleToString(
        getNumericalTolerance(),
        Settings::getMonteCarloDigits()
    );
}


QString MonteCarlo::getStatus( bool csvMode ) const {
    return csvMode ? StringUtils::addQuotes ( mStatus ) : mStatus;
}


QString MonteCarlo::getStdDeviationAsString( bool csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString( getStdDeviation(), precision );
}


QString MonteCarlo::histogramToCSVString() const {
    QString result {};
    if ( mHistogramValues.size() > 0 ) {
        OutputParameter *outputParameter { getOutputParameter() };
        if ( outputParameter ) {
            result += StringUtils::addQuotes(
                "Histogram " + outputParameter->getName() + ":"
            );
            result += StringUtils::endl;
            result += "Bins:" +
                      StringUtils::csvSeparator +
                      getHistogramBinsAsString();
            result += StringUtils::endl;
            result += "Probabilities:" + StringUtils::csvSeparator;
            result += getHistogramValuesAsString();
            result += StringUtils::endl;
        }
    }
    return result;
}


Statistics MonteCarlo::getOutputStatistics() const {
    return mOutputStat;
}


bool MonteCarlo::getRequestStop() const {
    return mRequestStop;
}


bool MonteCarlo::getValid() const {
    return mValid;
}


double MonteCarlo::getConvergenceFactor() const {
    return mConvergenceFactor;
}


double MonteCarlo::getHigherBound() const {
    return mHigherBound;
}


double MonteCarlo::getHistogramXMax() const {
    return mHistogramXMax;
}


double MonteCarlo::getHistogramXMin() const {
    return mHistogramXMin;
}


double MonteCarlo::getHistogramYMax() const {
    return mHistogramYMax;
}


double MonteCarlo::getLowerBound() const {
    return mLowerBound;
}


double MonteCarlo::getMean() const {
    return mMean;
}


double MonteCarlo::getNumericalTolerance() const {
    return mNumericalTolerance;
}


double MonteCarlo::getStdDeviation() const {
    return mStdDeviation;
}


int MonteCarlo::getHistogramHigherIndex() const {
    return mHistogramHigherIndex;
}


int MonteCarlo::getHistogramLowerIndex() const {
    return mHistogramLowerIndex;
}


void MonteCarlo::resetResults() {
    setMean();
    setStdDeviation();
    setLowerBound();
    setHigherBound();
    setStatus();
    setValid();
    setConvergenceFactor();
    setHistogramValues();
    setHistogramXMin();
    setHistogramXMax();
    setHistogramYMax();
    setHistogramLowerIndex();
    setHistogramHigherIndex();

    mOutputStat.clearSamples();
}


void MonteCarlo::run() {
    if ( mOutputParameter ) {
#if PRINT_MONTECARLO_TIMING
        qDebug() << "Start Monte Carlo simulation";
        qint64 tEvaluate {};
        qint64 tStats {};
        QElapsedTimer timer {};
        QElapsedTimer startTimer {};
        timer.start();
        startTimer.start();
#endif
        int nThreads { QThread::idealThreadCount() };

        // The object and the OutputParameter are locked first
        mMutex.lock();
        mOutputParameter->setLocked( true );

        // Settings are stored localy to make sure they don't change during the
        // simulation
        int hMax { Settings::getMonteCarloMaxNumOfBatches() };
        int maxBatchSize { Settings::getMonteCarloBatchSize() };
        int monteCarloDigits { Settings::getMonteCarloDigits() };

        // Reset the status and the results
        resetResults();
        setStatus( sMonteCarloRunning );

        // Get the confidence and create the Statistics objects accordingly
        const double p { mOutputParameter->getConfidence() };
        mOutputStat.setP( p );
        Statistics statBatch { Statistics( p ) };
        Statistics statMean { Statistics( p ) };
        Statistics statStdDev { Statistics( p ) };
        Statistics statLowerBound { Statistics( p ) };
        Statistics statHigherBound { Statistics( p ) };

        // In accordance to the GUM, the batch size M is set based on
        // maxBatchSize and the confidence p
        int M { std::max(
                maxBatchSize,
                static_cast<int> ( std::lround( 100 / ( 1 - p ) ) )
                )
        };
        statBatch.reserveCapacity( M );

        // Loop on the batches
        int h { 0 };
        int nSamples { 0 };
        bool converged { false };
        double meanAllSamples { 0. };
        double stdDevAllSamples { 0. };
        mValid = true;

        // Split the batches into nThreads sub-batches
        QList<int> subBatchIndices {};
        QList<int> subBatchSizes {};

        for ( int i { 0 }; i < nThreads; ++i ) {
            subBatchIndices.append( i );
            if ( i < nThreads - 1 ) {
                subBatchSizes.append( M / nThreads );
            }
            else {
                // Put the remainder in the last batch
                subBatchSizes.append( M - ( M / nThreads ) * i );
            }
        }

        while ( !converged && !mRequestStop && mValid && h <= hMax ) {
            // Perform a batch of M function evaluations for random values
            // of the input parameters (i.e. components)
            statBatch.clearSamples();

#if PRINT_MONTECARLO_TIMING
            timer.restart();
#endif
            // Generate random seed sequence
            std::vector<uint32_t> randomSeeds( nThreads );
            std::seed_seq seq { static_cast<uint32_t>( h ) };
            seq.generate( randomSeeds.begin(), randomSeeds.end() );

            // Create sub-batches in parallel
            QFuture<std::vector<double>> future = QtConcurrent::mapped(
                subBatchIndices,
                [ & ]( int subBatchIndex ) {
                    SampleBatch subBatch {
                        mOutputParameter,
                        subBatchSizes[ subBatchIndex ],
                        randomSeeds[ subBatchIndex ]
                    };
                    if ( !subBatch.generateSamples() ) {
                        mValid = false;
                        setStatus( subBatch.getError() );
                    }
                    return subBatch.getSamples();
                }
            );
            future.waitForFinished();

            // Combine sub-batch results
            if ( mValid ) {
                for ( int i { 0 }; i < future.resultCount(); ++i ) {
                    statBatch.addSamples( future.resultAt( i ) );
                }
            }
#if PRINT_MONTECARLO_TIMING
            // Total time needed for function evaluations and storage
            tEvaluate += timer.restart();
#endif

            if ( mValid && !mRequestStop ) {
                // Add the statistics of this batch to their respective
                // general Statistics objects
                statLowerBound.addSample( statBatch.getLowerBound() );
                statHigherBound.addSample( statBatch.getHigherBound() );
                statMean.addSample( statBatch.getMean() );
                statStdDev.addSample( statBatch.getStdDev() );

                if ( h > 0 && !mRequestStop ) {
                    // Statistics across all the batches
                    mOutputStat.addSamples( statBatch.getSamples() );

                    // Calculate the Combined (Pooled) Variance and stdDev based
                    // on the statistics of the previous total set of samples
                    // and the new batch. This is faster than repeatedly
                    // calculating the stdDev for all samples.
                    double varianceAllSamples {
                        (
                            ( nSamples - 1 ) * std::pow( stdDevAllSamples, 2 ) +
                            ( M - 1 ) * std::pow( statBatch.getStdDev(), 2 ) +
                            ( nSamples * M ) / ( nSamples + M ) *
                            std::pow( meanAllSamples - statBatch.getMean(), 2 )
                        )
                        /
                        ( nSamples + M - 1 )
                    };
                    stdDevAllSamples = std::sqrt( varianceAllSamples );

                    // Calculate the new mean based on the previous mean and the
                    // number of samples
                    meanAllSamples =
                        (
                            nSamples * meanAllSamples +
                            M * statBatch.getMean()
                        )
                        /
                        ( nSamples + M );
                    // Calculate the new total number of samples
                    nSamples += M;

                    // Determine the new convergence factor
                    std::vector<double> factors {};
                    factors.push_back( statMean.getStdDevOfTheMean() );
                    factors.push_back( statStdDev.getStdDevOfTheMean() );
                    factors.push_back(
                        statLowerBound.getStdDevOfTheMean()
                    );
                    factors.push_back(
                        statHigherBound.getStdDevOfTheMean()
                    );
                    double max { *std::max_element(
                                    factors.begin(),
                                    factors.end()
                                  )
                    };
                    calculateNumericalTolerance(
                        stdDevAllSamples,
                        monteCarloDigits
                    );
                    double new_factor {
                        getNumericalTolerance() /
                        sNumericalToleranceFactor /
                        ( 2. * max )
                    };
                    if ( new_factor >= 1. ) {
                        converged = true;
                        setConvergenceFactor( 1. );
                    }
                    else if ( new_factor > getConvergenceFactor() ) {
                        setConvergenceFactor( new_factor );
                    }

#if PRINT_MONTECARLO_TIMING
                    // Time needed to determine batch statistics
                    tStats += timer.restart();
#endif
                }


                if ( !converged ) {
                    // Not converged yet, perform another batch
                    ++h;
                }
            }
        }
        // Simulation is finished, calculate the results and set the status

#if PRINT_MONTECARLO_TIMING
        qDebug() << "Batches finished after"
                 << startTimer.elapsed()
                 << "ms";
#endif

        if ( mValid ) {
            if ( converged ) {
                // Store the statistics across all samples
                setLowerBound( mOutputStat.getLowerBound() );
                setHigherBound( mOutputStat.getHigherBound() );
                setHistogramValues( mOutputStat.getHistogramValues() );
                setHistogramXMin ( mOutputStat.getHistogramXMin() );
                setHistogramXMax ( mOutputStat.getHistogramXMax() );
                setHistogramYMax ( mOutputStat.getHistogramYMax() );
                setHistogramLowerIndex (
                    mOutputStat.getHistogramLowerIndex()
                );
                setHistogramHigherIndex (
                    mOutputStat.getHistogramHigherIndex()
                );
                setMean( mOutputStat.getMean() );
                setStdDeviation( mOutputStat.getStdDev() );
                setStatus(
                    sConvergedString.arg( QLocale().toString( nSamples ) )
                );
            }
            else if ( mRequestStop ) {
                // The simulation was stopped by the user
                mValid = false;
                setStatus(
                    sStoppedString.arg( QLocale().toString( nSamples ) )
                );
            }
            else {
                // The simulation didn't converge
                mValid = false;
                setStatus(
                    sNotConvergedString.arg(
                        QLocale().toString( nSamples )
                    )
                );
            }
        }

        // Remove the sample data and release the locks
        mOutputStat.clearSamples();
        mRequestStop = false;
        mOutputParameter->setLocked( false );
        mMutex.unlock();
        emit monteCarloStatusChanged();

#if PRINT_MONTECARLO_TIMING
        double tTotal { static_cast<double> ( startTimer.elapsed() ) };
        qDebug() << "Total time:" << tTotal << "ms";
        qDebug() << "Function evaluation time:"
                 << tEvaluate << "ms |"
                 << tEvaluate / tTotal * 100
                 << "%";
        qDebug() << "Statistics calculation time:"
                 << tStats << "ms |"
                 << tStats / tTotal * 100
                 << "%\n";
#endif
    }
}


void MonteCarlo::setConvergenceFactor( double convergenceFactor ) {
    mConvergenceFactor = convergenceFactor;
    emit convergenceFactorChanged();
}


void MonteCarlo::setHigherBound( double higherBound ) {
    mHigherBound = higherBound;
}


void MonteCarlo::setHistogramHigherIndex( int histogramHigherIndex ) {
    mHistogramHigherIndex = histogramHigherIndex;
}


void MonteCarlo::setHistogramLowerIndex( int histogramLowerIndex ) {
    mHistogramLowerIndex = histogramLowerIndex;
}


void MonteCarlo::setHistogramValues( const QList<double> &values ) {
    mHistogramValues = values;
}


void MonteCarlo::setHistogramXMax( double histogramXMax ) {
    mHistogramXMax = histogramXMax;
}


void MonteCarlo::setHistogramXMin( double histogramXMin ) {
    mHistogramXMin = histogramXMin;
}


void MonteCarlo::setHistogramYMax( double histogramYMax ) {
    mHistogramYMax = histogramYMax;
}


void MonteCarlo::setLowerBound( double lowerBound ) {
    mLowerBound = lowerBound;
}


void MonteCarlo::setMean( double mean ) {
    mMean = mean;
}


void MonteCarlo::setNumericalTolerance( double tolerance ) {
    mNumericalTolerance = tolerance;
}


void MonteCarlo::setOutputParameter( OutputParameter *outputParameter) {
    mOutputParameter = outputParameter;
}


void MonteCarlo::setOutputStatistics( const Statistics &outputStat ) {
    mOutputStat = outputStat;
}


void MonteCarlo::setRequestStop( bool requestStop ) {
    mRequestStop = requestStop;
}


void MonteCarlo::setStatus( const QString &status ) {
    mStatus = status;
    emit monteCarloStatusChanged();
}


void MonteCarlo::setStdDeviation( double stdDeviation ) {
    mStdDeviation = stdDeviation;
}


void MonteCarlo::setValid( bool valid ) {
    mValid = valid;
}


void MonteCarlo::stop() {
    setRequestStop( true );
}


MonteCarlo MonteCarlo::fromJson( const QJsonObject &json ) {
    MonteCarlo monteCarlo {};
    if (
        const QJsonValue v = json[ sNumericalToleranceString ]; v.isDouble()
    ) {
        monteCarlo.setNumericalTolerance( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sMeanString ]; v.isDouble() ) {
        monteCarlo.setMean( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sStdDeviationString ]; v.isDouble() ) {
        monteCarlo.setStdDeviation( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sLowerBoundString ]; v.isDouble() ) {
        monteCarlo.setLowerBound( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sHigherBoundString ]; v.isDouble() ) {
        monteCarlo.setHigherBound( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sStatusString ]; v.isString() ) {
        monteCarlo.setStatus( v.toString() );
    }
    if ( const QJsonValue v = json[ sValidString ]; v.isBool() ) {
        monteCarlo.setValid( v.toBool() );
    }
    if ( const QJsonValue v = json[ sHistogramXMinString ]; v.isDouble() ) {
        monteCarlo.setHistogramXMin( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sHistogramXMaxString ]; v.isDouble() ) {
        monteCarlo.setHistogramXMax( v.toDouble() );
    }
    if ( const QJsonValue v = json[ sHistogramYMaxString ]; v.isDouble() ) {
        monteCarlo.setHistogramYMax( v.toDouble() );
    }
    if (
        const QJsonValue v = json[ sHistogramLowerIndexString ]; v.isDouble()
    ) {
        monteCarlo.setHistogramLowerIndex( v.toInt() );
    }
    if (
        const QJsonValue v = json[ sHistogramHigherIndexString ]; v.isDouble()
    ) {
        monteCarlo.setHistogramHigherIndex( v.toInt() );
    }
    if ( const QJsonValue v = json[ sHistogramValuesString ]; v.isArray() ) {
        const QJsonArray jsonValues { v.toArray() };
        QList<double> histogramValues {};
        for ( const QJsonValue &value : jsonValues ) {
            histogramValues.append( value.toDouble() );
        }
        monteCarlo.setHistogramValues( histogramValues );
    }
    return monteCarlo;
}


QString MonteCarlo::getHistogramBinsAsString() const {
    const int precision { Settings::getCSVPrecision() };

    qsizetype numOfBins { mHistogramValues.size() };
    QStringList formattedValues {};
    double binSize {
        ( mHistogramXMax - mHistogramXMin ) / static_cast<double> ( numOfBins )
    };
    for ( int i { 0 }; i < numOfBins; ++i ) {
        double value { mHistogramXMin + binSize * i };
        formattedValues.append(
            StringUtils::doubleToString( value, precision )
        );
    }
    return formattedValues.join( StringUtils::csvSeparator );
}


QString MonteCarlo::getHistogramValuesAsString() const {
    const int precision { Settings::getCSVPrecision() };

    QStringList formattedValues {};
    for ( const double &value : mHistogramValues ) {
        formattedValues.append(
            StringUtils::doubleToString( value, precision )
        );
    }
    return formattedValues.join( StringUtils::csvSeparator );
}


void MonteCarlo::calculateNumericalTolerance( double value, int digits ) {
    // Calculate the numerical tolerance according to section 7.9.2 oj JCGM:101
    if ( std::abs( value ) > 0. ) {

        int exponent {
            static_cast<int> ( std::floor( std::log10( std::abs( value ) ) ) )
        };
        exponent = exponent < 0 ? exponent + 2 : exponent + 1;
        mNumericalTolerance =  0.5 * std::pow( 10, exponent - digits );
    }
}
