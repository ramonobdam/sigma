// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include "utils.h"
#include <QList>
#include <QString>
#include <functional>
#include <random>

// Forward declaration of InputParameter (to avoid recursion loop)
class InputParameter;

// Namespace with distribution type enumerator and probability distribution
// related functions
namespace Distribution {
    enum class Type : char {
        normal,
        uniform,
        triangular,
        arcsine,
        studentst,
        none
    };

    using InvCDF = std::function<double( double )>;

    InvCDF getInvCDF(
        const Type &distribution,
        const double &mean,
        const double &stdDev,
        const int &dof,
        const bool &dofInfinite
    );
    InvCDF invCDFArcsine( double mean, double halfWidth );
    InvCDF invCDFConstant( double mean );
    InvCDF invCDFNormal( double mean, double stdDev );
    InvCDF invCDFStudentsT( double mean, double stdDev, int dof );
    InvCDF invCDFTriangular( double mean, double halfWidth );
    InvCDF invCDFUniform( double mean, double halfWidth );
    QList<Type> getDistributions();
    QString distributionToString( const Type &distribution );
    Type stringToDistribution( const QString &distributionString );
    double invCDFStandardNormal( const double &p );
    double normalCDF( double z );
}

#endif // DISTRIBUTION_H
