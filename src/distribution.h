// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <QList>
#include <QString>
#include <functional>

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
        Type distribution,
        double mean,
        double stdDev,
        int dof,
        bool dofInfinite
    );
    InvCDF invCDFArcsine( double mean, double halfWidth );
    InvCDF invCDFConstant( double mean );
    InvCDF invCDFNormal( double mean, double stdDev );
    InvCDF invCDFStudentsT( double mean, double stdDev, int dof );
    InvCDF invCDFTriangular( double mean, double halfWidth );
    InvCDF invCDFUniform( double mean, double halfWidth );
    QList<Type> getDistributions();
    QString distributionToString( Type distribution );
    Type stringToDistribution( const QString &distributionString );
    double invCDFStandardNormal( double p );
    double normalCDF( double z );
}

#endif // DISTRIBUTION_H
