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
    InvCDF invCDFStudentsT( double mean, double stdDev, double dof );
    InvCDF invCDFTriangular( double mean, double halfWidth );
    InvCDF invCDFUniform( double mean, double halfWidth );
    QList<Type> getDistributions();
    QString distributionToString( Type distribution );
    Type stringToDistribution( const QString &distributionString );
    bool stringToDistribution(
        const QString &distributionString,
        Type &distribution
    );
    // Returns true when the distribution is the normal distribution (also for
    // a Student's t distribution with infinite DOF)
    bool isNormal( Type distribution, bool dofInfinite );
    double invCDFStandardNormal( double p );
    double normalCDF( double z );

    // Student's t-distribution for real (also non-integer) degrees of freedom
    // dof > 0, infinite DOF give the normal distribution. Returns NaN for
    // dof <= 0.
    // Coverage factor k: the two-sided interval [-k, k] contains the fraction
    // 'confidence' of the standard Student's t-distribution
    double coverageFactor( double confidence, double dof );
    double invCDFStandardStudentT( double p, double dof );
    double studentTCDF( double t, double dof );
}

#endif // DISTRIBUTION_H
