// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "mixedcopulasampler.h"
#include "outputparameter.h"
#include "uncertaintycomponent.h"

#include <QList>
#include <utility>


MixedCopulaSampler::MixedCopulaSampler( OutputParameter *parameter )
    :   mVariables {},
        mL {},
        mOutputParameter { parameter },
        mReady { false }
{}


OutputParameter * MixedCopulaSampler::getOutputParameter() const {
    return mOutputParameter;
}


void MixedCopulaSampler::addVariable( Distribution::InvCDF invCDF ) {
    mVariables.emplace_back( std::move( invCDF ) );
}


void MixedCopulaSampler::clear(){
    mReady = false;
    mVariables.clear();
    mL = {};
}


void MixedCopulaSampler::setLatentCorrelation( const Eigen::MatrixXd &rho ) {
    // Cholesky decomposition of the correlation matrix and store the Cholesky
    // factor L: Lower triangular matrix L, such that rho = L * LT
    Eigen::LLT<Eigen::MatrixXd> llt( rho );
    if ( llt.info() != Eigen::Success ) {
        // Decomponsition failed; latent correlation matrix not positive
        // definite
        mReady = false;
        return;
    }
    mL = llt.matrixL();
    mReady = true;
}


void MixedCopulaSampler::setRandomSymbolValues() const {
    int n { static_cast<int>( mVariables.size() ) };

    if ( n > 0 && mL.rows() == n && mOutputParameter && mReady ) {
        static std::normal_distribution<> N01( 0., 1. );

        // Step 1: Multivariate normal sampling via Cholesky factor
        Eigen::VectorXd z( n );
        for ( int i = 0; i < n; ++i ) {
            // Independent standard normal samples
            z( i ) = N01( sGenerator );
        }
        // mL * z introduces correlation structure, producing correlated normal
        // samples
        Eigen::VectorXd y { mL * z };

        // Step 2: Mapping to actual marginals via Gaussian Copula
        QList<UncertaintyComponent> components {
            mOutputParameter->getComponents()
        };
        for ( int i { 0 }; i < components.size() && i < n; ++i ) {
            // Map correlated normal to uniform samples (applying any CDF to its
            // own distribution gives a uniform distribution)
            double u { Distribution::normalCDF( y( i ) ) };
            // Apply the inverse CDF of the variable distribution to produce a
            // sample from that distribution
            components[ i ].setSymbolValue( mVariables[ i ]( u ) );
        }
    }
}


void MixedCopulaSampler::setOutputParameter(
    OutputParameter *outputParameter
) {
    mOutputParameter = outputParameter;
}


void MixedCopulaSampler::resetGenerator() {
    // Go back to the start of the deterministic list of random values
    sGenerator.seed();
}
