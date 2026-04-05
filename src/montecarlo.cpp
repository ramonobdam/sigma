// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "third_party/exprtk/exprtk.hpp"
#include "montecarlo.h"
#include "outputparameter.h"
#include "settings.h"
#include "stringutils.h"
#include <QJsonArray>
#include <QJsonValue>
#include <cmath>
#include <numeric>
#include <vector>


QString MonteCarlo::mHigherBoundString = "higherBound";
QString MonteCarlo::mHistogramHigherIndexString = "histogramHigherIndex";
QString MonteCarlo::mHistogramLowerIndexString = "histogramLowerIndex";
QString MonteCarlo::mHistogramValuesString = "histogramValues";
QString MonteCarlo::mHistogramXMaxString = "histogramXMax";
QString MonteCarlo::mHistogramXMinString = "histogramXMin";
QString MonteCarlo::mHistogramYMaxString = "histogramYMax";
QString MonteCarlo::mLowerBoundString = "lowerBound";
QString MonteCarlo::mMeanString = "mean";
QString MonteCarlo::mMonteNotCarloStarted = \
    "Click 'Start' to run Monte Carlo simulation";
QString MonteCarlo::mNumericalToleranceString = "numericalTolerance";
QString MonteCarlo::mStatusString = "status";
QString MonteCarlo::mStdDeviationString = "stdDeviation";
QString MonteCarlo::mValidString = "valid";

QStringList MonteCarlo::headerLabels = {
    "Output name",
    "Unit",
    "Simulation status",
    "Numerical tolerance",
    "Output estimate",
    "Standard uncertainty",
    "Expanded uncertainty",
    "Level of confidence"
};


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


MonteCarlo& MonteCarlo::operator= ( const MonteCarlo &mc ) {
    if ( this == &mc ) {
        return *this;
    }
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
    json[ mNumericalToleranceString ] = getNumericalTolerance();
    json[ mMeanString ] = getMean();
    json[ mStdDeviationString ] = getStdDeviation();
    json[ mLowerBoundString ] = getLowerBound();
    json[ mHigherBoundString ] = getHigherBound();
    json[ mStatusString ] = getStatus();
    json[ mValidString ] = getValid();
    json[ mHistogramXMinString ] = getHistogramXMin();
    json[ mHistogramXMaxString ] = getHistogramXMax();
    json[ mHistogramYMaxString ] = getHistogramYMax();
    json[ mHistogramLowerIndexString ] = getHistogramLowerIndex();
    json[ mHistogramHigherIndexString ] = getHistogramHigherIndex();
    json[ mHistogramValuesString ] = valuesArray;

    return json;
}


QList<double> MonteCarlo::getHistogramValues() const {
    return mHistogramValues;
}


QString MonteCarlo::getExpandedUncertaintyAsString(
    const bool &csvMode
) const {
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
    return csvMode ? addQuotes( result ) : result;
}


QString MonteCarlo::getMeanAsString( const bool &csvMode ) const {
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


QString MonteCarlo::getStatus( const bool &csvMode ) const {
    return csvMode ? addQuotes ( mStatus ) : mStatus;
}


QString MonteCarlo::getStdDeviationAsString( const bool &csvMode ) const {
    const int precision = {
        csvMode ? Settings::getCSVPrecision() : Settings::getDisplayPrecision()
    };

    return StringUtils::doubleToString( getStdDeviation(), precision );
}


QString MonteCarlo::histogramToString() const {
    QString result {};
    if ( mHistogramValues.size() > 0 ) {
        OutputParameter *outputParameter { getOutputParameter() };
        if ( outputParameter ) {
            result += addQuotes(
                "Histogram " + outputParameter->getName() + ":"
            );
            result += endl;
            result += "Bins:" +
                      StringUtils::csvSeparator +
                      getHistogramBinsAsString();
            result += endl;
            result += "Probabilities:" + StringUtils::csvSeparator;
            result += getHistogramValuesAsString();
            result += endl;
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
        expression_t expression {};
        expression.register_symbol_table( InputParameter::symbolTable );
        if ( OutputParameter::parser.compile(
                mOutputParameter->getFormulaStdWString(),
                expression
             )
        ) {
            // The object and the OutputParameter are locked first
            mMutex.lock();
            mOutputParameter->setLocked( true );

            // Settings are stored localy to make sure they don't change during
            // the simulation
            int hMax { Settings::getMonteCarloMaxNumOfBatches() };
            int maxBatchSize { Settings::getMonteCarloBatchSize() };
            int monteCarloDigits { Settings::getMonteCarloDigits() };

            // Reset the status, results and the random value generator
            resetResults();
            setStatus( mMonteCarloRunning );
            MixedCopulaSampler::resetGenerator();

            // Get the confidence and create the Statistics objects accordingly
            double p { mOutputParameter->getConfidence() };
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

            // Loop on the batches
            int h { 0 };
            bool converged { false };
            double stdDevAllSamples {};
            setValid( true );
            while (
                !converged &&
                !getRequestStop() &&
                getValid() &&
                h <= hMax
            ) {
                // Perform a batch of M function evaluations for random values
                // of the input parameters (i.e. components)
                statBatch.clearSamples();
                for ( int i { 0 }; i < M; ++i ) {
                    mOutputParameter->setRandomSymbolValues();
                    double value { expression.value() };
                    if ( std::isfinite( value ) ) {
                        // Sample is valid, add to batch
                        statBatch.addSample( value );
                    }
                    else {
                        // Sample is invalid, stop the simulation
                        setValid( false );
                        break;
                    }
                }
                if ( getValid() && !getRequestStop() ) {
                    // Calculate the statistics of this batch and add them to
                    // their respective general Statistics objects
                    statBatch.calculate();
                    statMean.addSample( statBatch.getMean() );
                    statStdDev.addSample( statBatch.getStdDev() );
                    statLowerBound.addSample( statBatch.getLowerBound() );
                    statHigherBound.addSample( statBatch.getHigherBound() );

                    if ( h > 0 && !getRequestStop() ) {
                        // Calculate statistics across the batches
                        statMean.calculate();
                        statStdDev.calculate();
                        statLowerBound.calculate();
                        statHigherBound.calculate();
                        mOutputStat.addSamples( statBatch.getSamples() );
                        mOutputStat.calculate();
                        stdDevAllSamples = mOutputStat.getStdDev();

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
                            mNumericalToleranceFactor /
                            ( 2. * max )
                        };
                        if ( new_factor >= 1. ) {
                            converged = true;
                            setConvergenceFactor( 1. );
                        }
                        else if ( new_factor > getConvergenceFactor() ) {
                            setConvergenceFactor( new_factor );
                        }
                    }
                    if ( !converged ) {
                        // Not converged yet, perform another batch
                        ++h;
                    }
                }
            }
            // Simulation is finished, calculate the results and set the status
            if ( getValid() ) {
                int numSamples { mOutputStat.getNumberOfSamples() };
                if ( converged ) {
                    // Calculate and store the statistics across all samples
                    mOutputStat.calculate( true );
                    setMean( mOutputStat.getMean() );
                    setStdDeviation( mOutputStat.getStdDev() );
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
                    setStatus( mConvergedString.arg( numSamples ) );
                }
                else if ( getRequestStop() ) {
                    // The simulation was stopped by the user
                    setValid( false );
                    setStatus( mStoppedString.arg( numSamples ) );
                }
                else {
                    // The simulation didn't converge
                    setValid( false );
                    setStatus( mNotConvergedString.arg( numSamples ) );
                }
            }
            else {
                // An invalid output value was detected
                QStringList inputValues {};
                QList<UncertaintyComponent> components {
                    mOutputParameter->getComponents()
                };
                for ( UncertaintyComponent &component : components ) {
                    QString var {
                        component.getName() +
                        " = " +
                        StringUtils::doubleToString(
                            component.getSymbolValue(),
                            Settings::getDisplayPrecision()
                        )
                    };
                    inputValues.append( var );
                }
                setStatus( mInvalidOutputString + inputValues.join( ", " ) );
            }

            // Set the values in the symbol table back to nominal and release
            // the locks
            mOutputParameter->resetSymbolValues();
            setRequestStop( false );
            mOutputParameter->setLocked( false );
            mMutex.unlock();
        }
    }
}


void MonteCarlo::setConvergenceFactor( const double &convergenceFactor ) {
    mConvergenceFactor = convergenceFactor;
    emit convergenceFactorChanged();
}


void MonteCarlo::setHigherBound( const double &higherBound ) {
    mHigherBound = higherBound;
}


void MonteCarlo::setHistogramHigherIndex( const int &histogramHigherIndex ) {
    mHistogramHigherIndex = histogramHigherIndex;
}


void MonteCarlo::setHistogramLowerIndex( const int &histogramLowerIndex ) {
    mHistogramLowerIndex = histogramLowerIndex;
}


void MonteCarlo::setHistogramValues( const QList<double> &values ) {
    mHistogramValues = values;
}


void MonteCarlo::setHistogramXMax( const double &histogramXMax ) {
    mHistogramXMax = histogramXMax;
}


void MonteCarlo::setHistogramXMin( const double &histogramXMin ) {
    mHistogramXMin = histogramXMin;
}


void MonteCarlo::setHistogramYMax( const double &histogramYMax ) {
    mHistogramYMax = histogramYMax;
}


void MonteCarlo::setLowerBound( const double &lowerBound ) {
    mLowerBound = lowerBound;
}


void MonteCarlo::setMean( const double &mean ) {
    mMean = mean;
}


void MonteCarlo::setNumericalTolerance( const double &tolerance ) {
    mNumericalTolerance = tolerance;
}


void MonteCarlo::setOutputParameter( OutputParameter *outputParameter) {
    mOutputParameter = outputParameter;
}


void MonteCarlo::setOutputStatistics( const Statistics &outputStat ) {
    mOutputStat = outputStat;
}


void MonteCarlo::setRequestStop( const bool &requestStop ) {
    mRequestStop = requestStop;
}


void MonteCarlo::setStatus( const QString &status ) {
    mStatus = status;
    emit monteCarloStatusChanged();
}


void MonteCarlo::setStdDeviation( const double &stdDeviation ) {
    mStdDeviation = stdDeviation;
}


void MonteCarlo::setValid( const bool &valid ) {
    mValid = valid;
}


void MonteCarlo::stop() {
    setRequestStop( true );
}


MonteCarlo MonteCarlo::fromJson( const QJsonObject &json ) {
    MonteCarlo monteCarlo {};
    if (
        const QJsonValue v = json[ mNumericalToleranceString ]; v.isDouble()
    ) {
        monteCarlo.setNumericalTolerance( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mMeanString ]; v.isDouble() ) {
        monteCarlo.setMean( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mStdDeviationString ]; v.isDouble() ) {
        monteCarlo.setStdDeviation( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mLowerBoundString ]; v.isDouble() ) {
        monteCarlo.setLowerBound( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mHigherBoundString ]; v.isDouble() ) {
        monteCarlo.setHigherBound( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mStatusString ]; v.isString() ) {
        monteCarlo.setStatus( v.toString() );
    }
    if ( const QJsonValue v = json[ mValidString ]; v.isBool() ) {
        monteCarlo.setValid( v.toBool() );
    }
    if ( const QJsonValue v = json[ mHistogramXMinString ]; v.isDouble() ) {
        monteCarlo.setHistogramXMin( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mHistogramXMaxString ]; v.isDouble() ) {
        monteCarlo.setHistogramXMax( v.toDouble() );
    }
    if ( const QJsonValue v = json[ mHistogramYMaxString ]; v.isDouble() ) {
        monteCarlo.setHistogramYMax( v.toDouble() );
    }
    if (
        const QJsonValue v = json[ mHistogramLowerIndexString ]; v.isDouble()
    ) {
        monteCarlo.setHistogramLowerIndex( v.toInt() );
    }
    if (
        const QJsonValue v = json[ mHistogramHigherIndexString ]; v.isDouble()
    ) {
        monteCarlo.setHistogramHigherIndex( v.toInt() );
    }
    if ( const QJsonValue v = json[ mHistogramValuesString ]; v.isArray() ) {
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


void MonteCarlo::calculateNumericalTolerance(
    const double &value,
    const int &digits
) {
    // Calculate the numerical tolerance according to section 7.9.2 oj JCGM:101
    if ( std::abs( value ) > 0. ) {

        int exponent {
            static_cast<int> ( std::floor( std::log10( std::abs( value ) ) ) )
        };
        exponent = exponent < 0 ? exponent + 2 : exponent + 1;
        mNumericalTolerance =  0.5 * std::pow( 10, exponent - digits );
    }
}
