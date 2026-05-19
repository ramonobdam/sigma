// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "samplebatch.h"
#include "settings.h"
#include "stringutils.h"
#include "uncertaintycomponent.h"
#include "math.h"
#include <QStringList>


SampleBatch::SampleBatch(
    OutputParameter *outputParameter,
    int batchSize ,
    uint32_t randomSeed
)   :   mOutputParameter { outputParameter },
        mSymbolValues {},
        mError {},
        mParser {},
        mSamples {},
        mSymbolTable {},
        mMixedCopulaSampler { randomSeed }
{
    setBatchSize( batchSize );
    mSymbolTable.add_constants();
}


OutputParameter * SampleBatch::getOutputParameter() const {
    return mOutputParameter;
}


bool SampleBatch::generateSamples() {
    if ( !mOutputParameter || mBatchSize == 0 ) {
        mError = sNoSamplesGeneratedString;
        return false;
    }

    mSamples.clear();
    mError.clear();

    // Set up the MixedCopulaSampler
    mMixedCopulaSampler.clear();
    mMixedCopulaSampler.setLatentCorrelation(
        mOutputParameter->getLatentCorrelation()
    );
    addVariablesToMixedCopulaSampler();

    // Compile the expression
    fillSymbolTable();
    expression_t expr {};
    expr.register_symbol_table( mSymbolTable );
    if ( mParser.compile( mOutputParameter->getFormulaStdWString(), expr ) ) {
        // Generate samples
        for ( int i { 0 }; i < mBatchSize; ++i ) {
            mMixedCopulaSampler.getRandomSymbolValues( mSymbolValues );
            double value { expr.value() };
            if ( std::isfinite( value ) ) {
                mSamples.push_back( value );
            }
            else {
                // An invalid output value was detected
                QStringList inputValues {};
                const QList<UncertaintyComponent> &components {
                    mOutputParameter->getComponents()
                };
                for ( int j { 0 }; j < components.size(); ++j ) {
                    QString var {
                        components[ j ].getName() +
                        " = " +
                        StringUtils::doubleToString(
                            mSymbolValues[ j ],
                            Settings::getDisplayPrecision()
                        )
                    };
                    inputValues.append( var );
                }
                mError = sInvalidOutputString + inputValues.join( ", " );

                return false;
            }
        }
    }

    return true;
}


const QString & SampleBatch::getError() const {
    return mError;
}


const std::vector<double> & SampleBatch::getSamples() const {
    return mSamples;
}


int SampleBatch::getBatchSize() const {
    return mBatchSize;
}


void SampleBatch::setBatchSize( int batchSize ) {
    mBatchSize = batchSize;
    mSamples.reserve( mBatchSize );
}


void SampleBatch::setOutputParameter( OutputParameter *outputParameter ) {
    mOutputParameter = outputParameter;
}


void SampleBatch::setRandomSeed( uint32_t randomSeed ) {
    mMixedCopulaSampler.setRandomSeed( randomSeed );
}


void SampleBatch::addVariablesToMixedCopulaSampler() {
    const QList<UncertaintyComponent> &components {
        mOutputParameter->getComponents()
    };
    for ( const UncertaintyComponent &component : components ) {
        mMixedCopulaSampler.addVariable( component.getInvCDF() );
    }

}


void SampleBatch::fillSymbolTable() {
    // Fill the symbol table based on the names of the components
    if ( !mOutputParameter ) return;

    const QList<UncertaintyComponent> &components {
        mOutputParameter->getComponents()
    };
    mSymbolValues.clear();
    mSymbolValues.reserve( components.size() );  // prevent reallocation
    for ( const UncertaintyComponent &component : components ) {
        mSymbolValues.push_back( 0. );
        mSymbolTable.add_variable(
            component.getNameStdWString(),
            mSymbolValues.back()
        );
    }
}
