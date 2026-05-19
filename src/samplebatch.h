// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef SAMPLEBATCH_H
#define SAMPLEBATCH_H

#include "mixedcopulasampler.hpp"
#include "inputparameter.h"
#include "outputparameter.h"
#include "third_party/exprtk/exprtk.hpp"
#include <QLatin1StringView>
#include <QList>
#include <QString>
#include <random>
#include <vector>

// Class that creates a batch of output value samples based on an
// OutputParameter, batch size and random seed using the Mersenne Twister
// pseudo-random number generator. This class contains its own symbol table,
// filled with only the symbols of the OutputParameter, to enable parallel
// calculation of batches.
class SampleBatch {

public:
    SampleBatch(
        OutputParameter *outputParameter = nullptr,
        int batchSize = 0,
        uint32_t randomSeed = 0
    );

    OutputParameter *getOutputParameter() const;
    bool generateSamples();
    const QString &getError() const;
    const std::vector<double> &getSamples() const;
    int getBatchSize() const;
    void setBatchSize( int batchSize );
    void setOutputParameter( OutputParameter *outputParameter );
    void setRandomSeed( uint32_t randomSeed );

private:
    void addVariablesToMixedCopulaSampler();
    void fillSymbolTable();

    MixedCopulaSampler<std::mt19937> mMixedCopulaSampler;
    OutputParameter *mOutputParameter;
    QString mError;
    int mBatchSize;
    parser_t mParser;
    std::vector<double> mSamples;
    std::vector<double> mSymbolValues;
    symbol_table_t mSymbolTable;

    static constexpr QLatin1StringView sInvalidOutputString {
        "Invalid output value for input parameter values: "
    };
    static constexpr QLatin1StringView sNoSamplesGeneratedString {
        "No samples could be generated"
    };
};

#endif // SAMPLEBATCH_H
