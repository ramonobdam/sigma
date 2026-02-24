// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "mixedcopulasampler.h"
#include "outputparameter.h"
#include "uncertaintycomponent.h"

#include <QList>
#include <cmath>


std::mt19937 MixedCopulaSampler::mGenerator = std::mt19937();


MixedCopulaSampler::MixedCopulaSampler( OutputParameter *parameter )
    :   mOutputParameter { parameter },
        mL {}
{}


OutputParameter *MixedCopulaSampler::getOutputParameter() const {
    return mOutputParameter;
}


void MixedCopulaSampler::addVariable( Distribution::InvCDF invCDF ) {
    mVariables.emplace_back( std::move(invCDF) );
}

void MixedCopulaSampler::clear(){
    mReady = false;
    mVariables.clear();
    mL = Eigen::MatrixXd();
}


void MixedCopulaSampler::setLatentCorrelation(
    const Eigen::MatrixXd &rho
) {
    Eigen::LLT<Eigen::MatrixXd> llt( rho );
    if ( llt.info() != Eigen::Success ) {
        // Latent correlation matrix not positive definite
        mReady = false;
        return;
    }
    mL = llt.matrixL();
    mReady = true;
}


void MixedCopulaSampler::setRandomSymbolValues() const {
    int n { static_cast<int>( mVariables.size() ) };

    if ( n > 0 && mOutputParameter && mReady ) {
        static std::normal_distribution<> N01( 0., 1. );

        // Step 1: latent correlated normal
        Eigen::VectorXd z( n );
        for (int i = 0; i < n; ++i) {
            z( i ) = N01( mGenerator );
        }
        Eigen::VectorXd y = mL * z;

        // Step 2: inverse CDF mapping
        QList<UncertaintyComponent> components {
            mOutputParameter->getComponents()
        };
        for ( int i { 0 }; i < components.size() && i < n; ++i ) {
            double u { Distribution::normalCDF( y( i ) ) };
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
    mGenerator.seed();
}
