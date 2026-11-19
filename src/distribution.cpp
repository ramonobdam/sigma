// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include "mathconstants.h"
#include "third_party/alglib/specialfunctions.h"
#include <QtAssert>
#include <QMap>
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
        Type type {};
        stringToDistribution( distributionString, type );
        return type;
    }

    bool stringToDistribution(
        const QString &distributionString,
        Type &distribution
    ) {
        // Set the distribution type based on distributionString if it matches
        // an existing distribution and return true if succesful
        for ( auto it { map.cbegin() }; it != map.cend(); ++it ) {
            if ( it.value() == distributionString.trimmed() ) {
                distribution = it.key();
                return true;
            }
        }
        distribution = Type::none;
        return false;
    }


    double normalCDF( double z ) {
        // erfc() instead of 1 + erf() avoids the loss of significant digits
        // in the lower tail (where erf() approaches -1)
        return 0.5 * std::erfc( -z * MathConstants::invSqrt2 );
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
        // Inverse standard normal CDF, accurate to about 1e-16 (full double
        // precision): algorithm AS241 (PPND16) of M.J. Wichura, Applied
        // Statistics 37(3), 1988, pp. 477-484
        if ( p <= 0. ) {
            return MathConstants::minInfinity;
        }
        if ( p >= 1. ) {
            return MathConstants::maxInfinity;
        }

        const double q { p - 0.5 };

        if ( std::abs( q ) <= 0.425 ) {
            // Central region: 0.075 <= p <= 0.925
            const double r { 0.180625 - q * q };
            return q *
                (((((((2509.0809287301226727 * r +
                       33430.575583588128105) * r +
                      67265.770927008700853) * r +
                     45921.953931549871457) * r +
                    13731.693765509461125) * r +
                   1971.5909503065514427) * r +
                  133.14166789178437745) * r +
                 3.387132872796366608) /
                (((((((5226.495278852545925 * r +
                       28729.085735721942674) * r +
                      39307.89580009271061) * r +
                     21213.794301586595867) * r +
                    5394.1960214247511077) * r +
                   687.1870074920579083) * r +
                  42.313330701600911252) * r +
                 1.);
        }

        // Tails: r = sqrt( -ln( min( p, 1 - p ) ) )
        double r { std::sqrt( -std::log( q < 0. ? p : 1. - p ) ) };
        double value {};

        if ( r <= 5. ) {
            // Intermediate tail: up to about 1e-11 from 0 or 1
            r -= 1.6;
            value =
                (((((((7.7454501427834140764e-4 * r +
                       0.0227238449892691845833) * r +
                      0.24178072517745061177) * r +
                     1.27045825245236838258) * r +
                    3.64784832476320460504) * r +
                   5.7694972214606914055) * r +
                  4.6303378461565452959) * r +
                 1.42343711074968357734) /
                (((((((1.05075007164441684324e-9 * r +
                       5.475938084995344946e-4) * r +
                      0.0151986665636164571966) * r +
                     0.14810397642748007459) * r +
                    0.68976733498510000455) * r +
                   1.6763848301838038494) * r +
                  2.05319162663775882187) * r +
                 1.);
        }
        else {
            // Far tail
            r -= 5.;
            value =
                (((((((2.01033439929228813265e-7 * r +
                       2.71155556874348757815e-5) * r +
                      0.0012426609473880784386) * r +
                     0.026532189526576123093) * r +
                    0.29656057182850489123) * r +
                   1.7848265399172913358) * r +
                  5.4637849111641143699) * r +
                 6.6579046435011037772) /
                (((((((2.04426310338993978564e-15 * r +
                       1.4215117583164458887e-7) * r +
                      1.8463183175100546818e-5) * r +
                     7.868691311456132591e-4) * r +
                    0.0148753612908506148525) * r +
                   0.13692988092273580531) * r +
                  0.59983220655588793769) * r +
                 1.);
        }

        return q < 0. ? -value : value;
    }


    bool isNormal( Type distribution, bool dofInfinite ) {
        // A Student's t distribution with infinite DOF is equal to the normal
        // distribution
        return distribution == Type::normal ||
               ( distribution == Type::studentst && dofInfinite );
    }
}
