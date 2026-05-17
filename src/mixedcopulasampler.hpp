// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MIXEDCOPULASAMPLER_HPP
#define MIXEDCOPULASAMPLER_HPP

#include "distribution.h"
#include <QList>
#include <random>
#include <third_party/Eigen/Dense>
#include <utility>
#include <vector>

// Template class that generates samples for correlated marginal distributions
// via inverse CDFs. Dependence is modeled in a latent normal space and mapped
// to the marginals through a Gaussian copula in setRandomSymbolValues():
//
// z ~ N(0,I) -> y = L·z ~ N(0,ρ) -> u = Φ(y) ~ Uniform -> x = F⁻¹(u) ~ Target
// independent   correlated          correlated            correlated with
// normals       normals             uniforms              correct marginals
//
// The RandomGenerator type must be a random number engine with the interface:
// - operator()
// - seed()
template<typename RandomGenerator>
class MixedCopulaSampler {

public:
    MixedCopulaSampler( uint32_t randomSeed = 0 )
    :   mVariables {},
        mL {},
        mY {},
        mZ {},
        mReady { false },
        mGenerator { randomSeed },
        mN01 { 0., 1. }
    {}


    RandomGenerator getGenerator() const {
        return mGenerator;
    }


    void addVariable( Distribution::InvCDF invCDF ) {
        mVariables.emplace_back( std::move( invCDF ) );
    }


    void clear(){
        mReady = false;
        mVariables.clear();
        mL = {};
        mZ.resize( 0 );
        mY.resize( 0 );
    }


    void getRandomSymbolValues( std::vector<double> &values ) {
        size_t n { mVariables.size() };

        if ( n > 0 && mL.rows() == n && values.size() == n && mReady ) {
            // Step 1: Multivariate normal sampling via Cholesky factor
            for ( int i = 0; i < n; ++i ) {
                // Independent standard normal samples
                mZ( i ) = mN01( mGenerator );
            }
            // mL * mZ introduces correlation structure, producing correlated
            // normal samples
            mY.noalias() = mL * mZ;     // noalias() avoids temporary

            // Step 2: Mapping to actual marginals via Gaussian Copula
            for ( int i { 0 }; i < n; ++i ) {
                // Map correlated normal to uniform samples (applying any CDF to
                // its own distribution gives a uniform distribution)
                double u { Distribution::normalCDF( mY( i ) ) };
                // Apply the inverse CDF of the variable distribution to produce
                // a sample from that distribution
                values[ i ] = mVariables[ i ]( u );
            }
        }
    }


    void setGenerator( const RandomGenerator &generator ) {
        mGenerator = generator;
    }


    void setLatentCorrelation( const Eigen::MatrixXd &rho ) {
        // Perform Cholesky decomposition of the correlation matrix and store
        // the Cholesky factor L:
        // Lower triangular matrix L, such that rho = L * LT
        Eigen::LLT<Eigen::MatrixXd> llt( rho );
        if ( llt.info() != Eigen::Success ) {
            // Decomponsition failed; latent correlation matrix not positive
            // definite
            mReady = false;
            return;
        }
        mL = llt.matrixL();
        mZ.resize( mL.rows() );
        mY.resize( mL.rows() );
        mReady = true;
    }


    void setRandomSeed( uint32_t randomSeed ) {
        mGenerator.seed( randomSeed );
    }


private:
    Eigen::MatrixXd mL;
    Eigen::VectorXd mY;
    Eigen::VectorXd mZ;
    RandomGenerator mGenerator;
    bool mReady;
    std::normal_distribution<double> mN01;
    std::vector<Distribution::InvCDF> mVariables;
};

#endif // MIXEDCOPULASAMPLER_HPP
