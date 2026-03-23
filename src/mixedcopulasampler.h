// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MIXEDCOPULASAMPLER_H
#define MIXEDCOPULASAMPLER_H

#include "distribution.h"
#include <third_party/Eigen/Dense>
#include <random>
#include <vector>
#include <functional>

// Forward declaration of OutputParameter (to avoid recursion loop)
class OutputParameter;

// Class that generates samples for correlated marginal distributions via
// inverse CDFs. Dependence is modeled in a latent normal space and mapped to
// the marginals through a Gaussian copula.
class MixedCopulaSampler {

public:
    MixedCopulaSampler( OutputParameter *parameter );

    OutputParameter* getOutputParameter() const;
    void addVariable( const Distribution::InvCDF &invCDF );
    void clear();
    void setLatentCorrelation( const Eigen::MatrixXd &rho );
    void setRandomSymbolValues() const;
    void setOutputParameter( OutputParameter *outputParameter );

    static void resetGenerator();

private:
    std::vector<Distribution::InvCDF> mVariables;
    Eigen::MatrixXd mL;
    OutputParameter *mOutputParameter;
    bool mReady;

    static std::mt19937 mGenerator;
};

#endif // MIXEDCOPULASAMPLER_H
