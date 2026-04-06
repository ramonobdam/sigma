// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include "mathconstants.h"
#include "third_party/alglib/specialfunctions.h"
#include <QtAssert>
#include <QMap>
#include <array>
#include <cmath>


namespace Distribution {
    QMap<Type, QString> map = {
        { Type::normal, "normal" },
        { Type::uniform, "uniform" },
        { Type::triangular, "triangular" },
        { Type::arcsine, "arcsine" },
        { Type::studentst, "student's t" },
        { Type::none, "none" }
    };


    QString distributionToString( Type distribution ) {
        return map.value( distribution, "" );
    }


    QList<Type> getDistributions() {
        return QList<Type> {
            Type::normal,
            Type::uniform,
            Type::triangular,
            Type::arcsine,
            Type::studentst,
            Type::none
        };
    }


    Type stringToDistribution( const QString &distributionString ) {
        for ( auto it { map.cbegin() }; it != map.cend(); ++it ) {
            if ( it.value() == distributionString.trimmed() ) {
                return it.key();
            }
        }
        return Type::none;
    }


    double normalCDF( double z ) {
        return 0.5 * ( 1. + std::erf( z / std::sqrt( 2. ) ) );
    }


    InvCDF invCDFArcsine( double mean, double halfWidth ) {
        return [ = ]( double u ) {
            if ( u <= 0. ) {
                return mean - halfWidth;
            }
            if ( u >= 1. ) {
                return mean + halfWidth;
            }

            // Arcsine distribution for a = mean - halfWidth and
            // b - a = 2 * halfWidth
            double s { std::sin( 0.5 * MathConstants::pi * u ) };
            return mean - halfWidth + 2. * halfWidth * s * s;
        };
    }


    InvCDF invCDFUniform( double mean, double halfWidth ) {
        return [ = ]( double u ) { return mean + halfWidth * ( 2. * u - 1. ); };
    }


    InvCDF invCDFTriangular( double mean, double halfWidth ) {
        return [ = ]( double u ) {
            if ( u < 0.5 ) {
                return mean + halfWidth * ( -1. + std::sqrt( 2. * u ) );
            }
            else {
                return mean + halfWidth * ( 1. - std::sqrt( 2. * ( 1. - u ) ) );
            }
        };
    }


    InvCDF getInvCDF(
        Type distribution,
        double mean,
        double stdDev,
        int dof,
        bool dofInfinite
    ) {
        double halfWidth {};

        switch ( distribution ) {
            case Type::normal: {
                return invCDFNormal( mean, stdDev );
            }
            case Type::uniform: {
                halfWidth = std::abs( stdDev ) * std::sqrt( 3. );
                return invCDFUniform( mean, halfWidth);
            }
            case Type::triangular: {
                halfWidth = std::abs( stdDev ) * std::sqrt( 6. ) ;
                return invCDFTriangular( mean, halfWidth );
            }
            case Type::arcsine: {
                halfWidth = std::abs( stdDev ) * std::sqrt( 2. ) ;
                return invCDFArcsine( mean, halfWidth );
            }
            case Type::studentst: {
                if ( dofInfinite ) {
                    // For infinite DOF, the student's t distribution is equal
                    // to the normal distribution
                    return invCDFNormal( mean, stdDev );
                }
                else {
                    return invCDFStudentsT( mean, stdDev, dof );
                }
            }
            case Type::none: {
                return invCDFConstant( mean );
            }
            default: {
                Q_ASSERT_X(
                    false,
                    "Distribution::getInvCDF()",
                    "InvCDF not defined for Distribution::Type"
                );
                return invCDFConstant( mean );
            }
        }
    }


    InvCDF invCDFNormal( double mean, double stdDev ) {
        return [ = ]( double u ) {
            return mean + stdDev * invCDFStandardNormal( u );
        };
    }


    InvCDF invCDFConstant( double mean ) {
        return [ = ]( double ) {
            return mean;
        };
    }

    InvCDF invCDFStudentsT( double mean, double stdDev, int dof ) {
        // For Student’s t distributions, the entered standard uncertainty is
        // interpreted as the experimental standard deviation s of the mean
        // (GUM Type A).

        Q_ASSERT_X(
            dof >= 1,
            "Distribution::invCDFStudentsT",
            "Student's t doesn't exist for dof < 1"
        );

        return [ = ]( double u ) {
            return mean + stdDev * alglib::invstudenttdistribution( dof, u );
        };

    }


    double invCDFStandardNormal( double p ) {
        // Inverse standard normal CDF (Acklam approximation)
        if ( p <= 0. ) {
            return MathConstants::minInfinity;
        }
        if ( p >= 1. ) {
            return MathConstants::maxInfinity;
        }

        static const double a1 { -3.969683028665376e+01 };
        static const double a2 {  2.209460984245205e+02 };
        static const double a3 { -2.759285104469687e+02 };
        static const double a4 {  1.383577518672690e+02 };
        static const double a5 { -3.066479806614716e+01 };
        static const double a6 {  2.506628277459239e+00 };

        static const double b1 { -5.447609879822406e+01 };
        static const double b2 {  1.615858368580409e+02 };
        static const double b3 { -1.556989798598866e+02 };
        static const double b4 {  6.680131188771972e+01 };
        static const double b5 { -1.328068155288572e+01 };

        static const double c1 { -7.784894002430293e-03 };
        static const double c2 { -3.223964580411365e-01 };
        static const double c3 { -2.400758277161838e+00 };
        static const double c4 { -2.549732539343734e+00 };
        static const double c5 {  4.374664141464968e+00 };
        static const double c6 {  2.938163982698783e+00 };

        static const double d1 {  7.784695709041462e-03 };
        static const double d2 {  3.224671290700398e-01 };
        static const double d3 {  2.445134137142996e+00 };
        static const double d4 {  3.754408661907416e+00 };

        const double plow  { 0.02425 };
        const double phigh { 1. - plow };

        double q {};
        double r {};

        if ( p < plow ) {
            q = std::sqrt(-2. * std::log(p));
            return (((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
                   ((((d1*q + d2)*q + d3)*q + d4)*q + 1.);
        }

        if ( p > phigh ) {
            q = std::sqrt(-2. * std::log(1. - p));
            return -(((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
                   ((((d1*q + d2)*q + d3)*q + d4)*q + 1.);
        }

        q = p - 0.5;
        r = q * q;
        return (((((a1*r + a2)*r + a3)*r + a4)*r + a5)*r + a6)*q /
               (((((b1*r + b2)*r + b3)*r + b4)*r + b5)*r + 1.);
    }
}
