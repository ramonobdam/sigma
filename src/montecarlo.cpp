// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "montecarlo.h"
#include "outputparameter.h"
#include "samplebatch.h"
#include "samplewriter.h"
#include "settings.h"
#include "stringutils.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFuture>
#include <QJsonArray>
#include <QJsonValue>
#include <QLocale>
#include <QtConcurrentMap>
#include <algorithm>
#include <cmath>
#include <memory>
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
        mValid { false },
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
        mRequestStop {},
        mSampleExportUrl {},
        mSampleExportMessage {},
        mSamplesExported { false }
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
        mRequestStop { mc.getRequestStop() },
        // The sample export state is not copied (it belongs to a single run)
        mSampleExportUrl {},
        mSampleExportMessage {},
        mSamplesExported { false }
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


bool MonteCarlo::getSamplesExported() const {
    return mSamplesExported;
}


const QString & MonteCarlo::getSampleExportMessage() const {
    return mSampleExportMessage;
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
        // The object and the OutputParameter are locked first
        mMutex.lock();
        mOutputParameter->setLocked( true );

        // The sample export url applies to this run only
        const QUrl sampleExportUrl { mSampleExportUrl };
        const bool exportSamples { !sampleExportUrl.isEmpty() };
        SampleWriter sampleWriter {};
        mSampleExportUrl.clear();
        mSamplesExported = false;
        mSampleExportMessage.clear();

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
        qint64 nSamples { 0 };
        bool converged { false };
        double meanAllSamples { 0. };
        double stdDevAllSamples { 0. };
        mValid = true;

        // Open the sample export file before the first batch, so an invalid
        // file is reported immediately instead of after the simulation
        if ( exportSamples ) {
            mValid = openSampleWriter( sampleWriter, sampleExportUrl );
        }

        // Split the batches into chunks of (at most) sChunkSize samples. The
        // chunks and their random seeds only depend on M and the batch index,
        // not on the number of threads, so the results are the same on every
        // machine. The chunks are distributed over the available threads.
        const int nChunks { ( M + sChunkSize - 1 ) / sChunkSize };
        QList<int> chunkIndices {};
        QList<int> chunkSizes {};
        for ( int i { 0 }; i < nChunks; ++i ) {
            chunkIndices.append( i );
            // The last chunk contains the remainder
            chunkSizes.append(
                i < nChunks - 1 ? sChunkSize : M - sChunkSize * i
            );
        }

        while ( !converged && !mRequestStop && mValid && h <= hMax ) {
            // Perform a batch of M function evaluations for random values
            // of the input parameters (i.e. components)
            statBatch.clearSamples();

#if PRINT_MONTECARLO_TIMING
            timer.restart();
#endif
            // The random seed of each chunk is derived from the batch index
            // and the chunk index
            std::vector<uint32_t> randomSeeds( nChunks );
            for ( int i { 0 }; i < nChunks; ++i ) {
                std::seed_seq seq {
                    static_cast<uint32_t>( h ),
                    static_cast<uint32_t>( i )
                };
                seq.generate(
                    randomSeeds.begin() + i,
                    randomSeeds.begin() + i + 1
                );
            }

            // Create the chunks in parallel
            QFuture<std::shared_ptr<SampleBatch>> future = QtConcurrent::mapped(
                chunkIndices,
                [ & ]( int chunkIndex ) {
                    auto chunk = std::make_shared<SampleBatch>(
                        mOutputParameter,
                        chunkSizes[ chunkIndex ],
                        randomSeeds[ chunkIndex ]
                    );
                    // A failure is reported via the chunk's error, which is
                    // handled below (not here, to avoid a data race)
                    chunk->generateSamples( exportSamples );
                    return chunk;
                }
            );
            future.waitForFinished();

            // Report the error of the first failed chunk (in chunk order, so
            // the reported error doesn't depend on the thread timing)
            for ( int i { 0 }; i < future.resultCount() && mValid; ++i ) {
                const QString &error { future.resultAt( i )->getError() };
                if ( !error.isEmpty() ) {
                    mValid = false;
                    setStatus( error );
                }
            }

            // Combine the chunk results (in chunk order)
            if ( mValid ) {
                for ( int i { 0 }; i < future.resultCount(); ++i ) {
                    statBatch.addSamples( future.resultAt( i )->getSamples() );
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

                // Statistics across all the batches
                mOutputStat.addSamples( statBatch.getSamples() );

                // Export exactly the samples that are used for the results
                if ( exportSamples ) {
                    for ( int i { 0 }; i < future.resultCount(); ++i ) {
                        if ( !sampleWriter.write( *future.resultAt( i ) ) ) {
                            mValid = false;
                            setStatus(
                                sSamplesNotExportedString.arg(
                                    sampleWriter.getError()
                                )
                            );
                            break;
                        }
                    }
                }

                // Calculate the Combined (Pooled) Variance and stdDev based
                // on the statistics of the previous total set of samples
                // and the new batch. This is faster than repeatedly
                // calculating the stdDev for all samples.
                const double n1 { static_cast<double>( nSamples ) };
                const double n2 { static_cast<double>( M ) };
                const double nTotal { n1 + n2 };
                const double batchMean { statBatch.getMean() };
                const double batchStdDev { statBatch.getStdDev() };
                const double meanDiff { meanAllSamples - batchMean };

                const double varianceAllSamples {
                    (
                        ( n1 - 1. ) * stdDevAllSamples * stdDevAllSamples +
                        ( n2 - 1. ) * batchStdDev * batchStdDev +
                        n1 * n2 / nTotal * meanDiff * meanDiff
                    )
                    /
                    ( nTotal - 1. )
                };
                stdDevAllSamples = std::sqrt( varianceAllSamples );

                // Update the mean of all samples
                meanAllSamples = ( n1 * meanAllSamples + n2 * batchMean )
                                 /
                                 nTotal;

                // Update the total number of samples
                nSamples += M;

                if ( h > 0 && !mRequestStop ) {
                    // Determine the new convergence factor, which requires at
                    // least 2 batches
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

                    // Convergence also requires sMinNumOfBatches batches (h
                    // starts at 0). Until then, the factor (i.e. the shown
                    // progress) is limited to the fraction of the minimum
                    // number of batches that has been completed.
                    new_factor = std::min(
                        new_factor,
                        static_cast<double>( h + 1 ) / sMinNumOfBatches
                    );

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

        // Finish the sample export: the file is only created when the
        // simulation converged (mValid is only true in that case). The result
        // is reported via getSampleExportMessage() and not in the status, so
        // the (saved) results are the same as for a run without export.
        if ( exportSamples ) {
            if ( mValid && sampleWriter.commit() ) {
                mSamplesExported = true;
                mSampleExportMessage = sSamplesExportedString.arg(
                    sampleWriter.getFileName()
                );
            }
            else {
                sampleWriter.cancel();
                mSampleExportMessage = sampleWriter.getError().isEmpty() ?
                    sSamplesNotExportedString.arg( mStatus ) :
                    sSamplesNotExportedString.arg( sampleWriter.getError() );
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


bool MonteCarlo::setSampleExportUrl( const QUrl &url ) {
    if ( isRunning() ) {
        return false;
    }
    mSampleExportUrl = url;
    return true;
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
        // Use copy constructor '()' instead of initializer_list constructor
        // '{}' — QJsonArray implicitly converts to QJsonValue, so brace-init
        // would match the initializer_list<QJsonValue> constructor instead,
        // wrapping the whole array as a single element rather than copying it.
        const QJsonArray jsonValues ( v.toArray() );
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


bool MonteCarlo::openSampleWriter( SampleWriter &writer, const QUrl &url ) {
    // The columns are the components (in the order of the symbol values of
    // SampleBatch) followed by the output parameter
    QStringList inputNames {};
    const QList<UncertaintyComponent> components {
        mOutputParameter->getComponents()
    };
    for ( const UncertaintyComponent &component : components ) {
        inputNames.append( component.getName() );
    }

    if ( !writer.open( url, inputNames, mOutputParameter->getName() ) ) {
        setStatus( sSamplesNotExportedString.arg( writer.getError() ) );
        return false;
    }
    return true;
}
