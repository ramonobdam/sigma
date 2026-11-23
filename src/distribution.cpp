// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "distribution.h"
#include "mathconstants.h"
#include <QtAssert>
#include <QMap>
#include <algorithm>
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


    namespace {
        // Constants of the Student's t-distribution that only depend on the
        // degrees of freedom
        struct StudentTConstants {
            double dof;
            double a;
            double lnBeta;
            double lnPdfConstant;
        };

        // Probabilities and probability density of the Student's
        // t-distribution for a value t >= 0
        struct StudentTProbabilities {
            double tail;     // P( T > t )
            double central;  // P( |T| < t )
            double pdf;
        };

        double logGammaRatioHalf( double a ) {
            // Returns ln( Gamma( a + 1/2 ) / Gamma( a ) ). For large a the
            // difference of the Stirling series is used to prevent the loss of
            // precision when subtracting two large lgamma() values.
            if ( a < 20. ) {
                return std::lgamma( a + 0.5 ) - std::lgamma( a );
            }
            auto stirling = []( double z ) {
                const double z2 { 1. / ( z * z ) };
                return (
                    1. / 12. + z2 * (
                        -1. / 360. + z2 * (
                            1. / 1260. + z2 * ( -1. / 1680. + z2 / 1188. )
                        )
                    )
                ) / z;
            };
            return 0.5 * std::log( a ) +
                   ( a * std::log1p( 0.5 / a ) - 0.5 ) +
                   stirling( a + 0.5 ) - stirling( a );
        }

        StudentTConstants studentTConstants( double dof ) {
            // ln B( dof / 2, 1 / 2 ) and the logarithm of the constant factor
            // of the probability density function
            StudentTConstants constants {};
            constants.dof = dof;
            constants.a = 0.5 * dof;
            constants.lnBeta = 0.5 * std::log( MathConstants::pi ) -
                               logGammaRatioHalf( constants.a );
            constants.lnPdfConstant = -0.5 * std::log( dof ) -
                                      constants.lnBeta;
            return constants;
        }

        double betaContinuedFraction( double a, double b, double x ) {
            // Continued fraction of the regularized incomplete beta function
            // I_x( a, b ), evaluated with the modified Lentz method. Converges
            // rapidly for x < ( a + 1 ) / ( a + b + 2 ).
            constexpr int maxIterations { 10000 };
            constexpr double epsilon { 1e-16 };
            constexpr double fpMin { 1e-300 };

            auto limit = []( double value ) {
                return std::abs( value ) < fpMin ? fpMin : value;
            };

            const double qab { a + b };
            const double qap { a + 1. };
            const double qam { a - 1. };
            double c { 1. };
            double d { 1. / limit( 1. - qab * x / qap ) };
            double h { d };
            for ( int m { 1 }; m <= maxIterations; ++m ) {
                const double m2 { 2. * m };

                // Even step
                double aa { m * ( b - m ) * x / ( ( qam + m2 ) * ( a + m2 ) ) };
                d = 1. / limit( 1. + aa * d );
                c = limit( 1. + aa / c );
                h *= d * c;

                // Odd step
                aa = -( a + m ) * ( qab + m ) * x /
                     ( ( a + m2 ) * ( qap + m2 ) );
                d = 1. / limit( 1. + aa * d );
                c = limit( 1. + aa / c );
                const double delta { d * c };
                h *= delta;

                if ( std::abs( delta - 1. ) < epsilon ) {
                    break;
                }
            }
            return h;
        }

        StudentTProbabilities studentTProbabilities(
            double t,
            const StudentTConstants &constants
        ) {
            // The Student's t probabilities follow from the regularized
            // incomplete beta function:
            //   P( T > t )   = I_x( dof / 2, 1 / 2 ) / 2
            //   P( |T| < t ) = I_y( 1 / 2, dof / 2 )
            // with x = dof / ( dof + t^2 ) and y = 1 - x = t^2 / ( dof + t^2 ).
            // The logarithms of x and y are calculated from r = t^2 / dof to
            // retain the relative precision for large degrees of freedom.
            if ( t <= 0. ) {
                return { 0.5, 0., std::exp( constants.lnPdfConstant ) };
            }
            if ( std::isinf( t ) ) {
                return { 0., 1., 0. };
            }

            const double a { constants.a };
            const double b { 0.5 };
            const double s { t / std::sqrt( constants.dof ) };
            const double r { s * s };
            const double lnR { 2. * std::log( s ) };
            const double ln1pR { std::isinf( r ) ? lnR : std::log1p( r ) };
            const double x { 1. / ( 1. + r ) };
            const double y { 1. / ( 1. + 1. / r ) };

            // x^a * y^b / B( a, b )
            const double prefactor {
                std::exp( -a * ln1pR + b * ( lnR - ln1pR ) - constants.lnBeta )
            };

            StudentTProbabilities result {};
            result.pdf = std::exp(
                constants.lnPdfConstant - ( a + b ) * ln1pR
            );
            if ( x < ( a + 1. ) / ( a + b + 2. ) ) {
                result.tail =
                    0.5 * prefactor * betaContinuedFraction( a, b, x ) / a;
                result.central = 1. - 2. * result.tail;
            }
            else {
                result.central =
                    prefactor * betaContinuedFraction( b, a, y ) / b;
                result.tail = 0.5 * ( 1. - result.central );
            }
            return result;
        }

        double hillApproximation( double q, double dof ) {
            // Approximation of the upper quantile of the Student's
            // t-distribution for the upper tail probability q according to
            // G.W. Hill, Algorithm 396: Student's t-Quantiles, Communications
            // of the ACM 13(10), 1970. Used as starting value for the iterative
            // refinement.
            const double p { 2. * q };
            const double a { 1. / ( dof - 0.5 ) };
            const double b { 48. / ( a * a ) };
            double c { ( ( 20700. * a / b - 98. ) * a - 16. ) * a + 96.36 };
            const double d {
                ( ( 94.5 / ( b + c ) - 3. ) / b + 1. ) *
                std::sqrt( a * MathConstants::pi / 2. ) * dof
            };
            double y { std::pow( d * p, 2. / dof ) };
            if ( y > 0.05 + a ) {
                // Asymptotic inverse expansion about the normal distribution
                const double x { invCDFStandardNormal( q ) };
                y = x * x;
                if ( dof < 5. ) {
                    c += 0.3 * ( dof - 4.5 ) * ( x + 0.6 );
                }
                c = ( ( ( 0.05 * d * x - 5. ) * x - 7. ) * x - 2. ) * x + b + c;
                y = (
                    ( ( ( ( 0.4 * y + 6.3 ) * y + 36. ) * y + 94.5 ) / c -
                      y - 3. ) / b + 1.
                ) * x;
                y = std::expm1( a * y * y );
            }
            else {
                y = (
                    ( 1. / (
                        ( ( dof + 6. ) / ( dof * y ) - 0.089 * d - 0.822 ) *
                        ( dof + 2. ) * 3.
                    ) + 0.5 / ( dof + 4. ) ) * y - 1.
                ) * ( dof + 1. ) / ( dof + 2. ) + 1. / y;
            }
            return std::sqrt( dof * y );
        }

        double standardNormalUpperQuantile( double q, double c ) {
            // Upper quantile z > 0 of the standard normal distribution for the
            // upper tail probability q = P( Z > z ) and the central probability
            // c = P( |Z| < z ) = 1 - 2q. Both are passed to prevent the loss of
            // precision of 1 - 2q. The quantile from invCDFStandardNormal() is
            // refined using two Halley steps on q or c (whichever is more
            // accurate).
            constexpr double sqrt1_2 {
                0.70710678118654752440084436210484903928
            };
            constexpr double sqrt2Pi {
                2.50662827463100050241576528481104525301
            };

            if ( q >= 0.5 ) {
                return 0.;
            }
            if ( q <= 0. ) {
                return MathConstants::maxInfinity;
            }

            double z { -invCDFStandardNormal( q ) };
            for ( int i { 0 }; i < 2; ++i ) {
                const double pdf { std::exp( -0.5 * z * z ) / sqrt2Pi };
                const double u {
                    q < 0.25 ? -( 0.5 * std::erfc( z * sqrt1_2 ) - q ) / pdf
                             : ( std::erf( z * sqrt1_2 ) - c ) / ( 2. * pdf )
                };
                z -= u / ( 1. + 0.5 * u * z );
            }
            return z;
        }

        double cornishFisherExpansion( double z, double dof ) {
            // Asymptotic expansion of the Student's t quantile in terms of the
            // standard normal quantile z for large degrees of freedom
            // (Abramowitz and Stegun 26.7.5). The relative truncation error is
            // smaller than 1e-3 * ( z^2 / dof )^5.
            const double z2 { z * z };
            const double g1 { ( z2 + 1. ) / 4. };
            const double g2 { ( ( 5. * z2 + 16. ) * z2 + 3. ) / 96. };
            const double g3 {
                ( ( ( 3. * z2 + 19. ) * z2 + 17. ) * z2 - 15. ) / 384.
            };
            const double g4 {
                ( ( ( ( 79. * z2 + 776. ) * z2 + 1482. ) * z2 - 1920. ) * z2 -
                  945. ) / 92160.
            };
            const double v { 1. / dof };
            return z * ( 1. + v * ( g1 + v * ( g2 + v * ( g3 + v * g4 ) ) ) );
        }

        double studentTUpperQuantile(
            double q,
            double c,
            const StudentTConstants &constants
        ) {
            // Upper quantile t > 0 of the Student's t-distribution for the
            // upper tail probability q = P( T > t ) and the central probability
            // c = P( |T| < t ) = 1 - 2q. Both are passed to prevent the loss of
            // precision of 1 - 2q.
            const double dof { constants.dof };

            if ( q >= 0.5 ) {
                return 0.;
            }
            if ( q <= 0. ) {
                return MathConstants::maxInfinity;
            }
            if ( std::isinf( dof ) ) {
                return standardNormalUpperQuantile( q, c );
            }

            // Closed-form solutions
            if ( dof == 1. ) {
                if ( q < 0.25 ) {
                    return 1. / std::tan( MathConstants::pi * q );
                }
                return std::tan( 0.5 * MathConstants::pi * c );
            }
            if ( dof == 2. ) {
                return c / std::sqrt( 2. * q * ( 1. - q ) );
            }

            // Asymptotic expansion for large degrees of freedom when the
            // truncation error is negligible
            constexpr double minDOFExpansion { 1e5 };
            constexpr double maxZ2DOFRatioExpansion { 0.01 };
            if ( dof >= minDOFExpansion ) {
                const double z { standardNormalUpperQuantile( q, c ) };
                if ( z * z <= maxZ2DOFRatioExpansion * dof ) {
                    return cornishFisherExpansion( z, dof );
                }
            }

            // Newton-Raphson refinement of the Hill approximation. In the tail
            // region ln( P( T > t ) ) is solved as function of ln( t ) to
            // handle the heavy tails of the distribution, in the central
            // region P( |T| < t ) is solved as function of t. The root is
            // bracketed by [lower, upper] to guarantee convergence. Due to the
            // quadratic convergence, the error after a Newton step with a
            // relative step size smaller than the tolerance is negligible.
            constexpr int maxIterations { 100 };
            constexpr double tolerance { 1e-9 };
            constexpr double tailThreshold { 0.25 };

            const bool tailRegion { q < tailThreshold };
            const double lnQ { std::log( q ) };
            double lower { 0. };
            double upper { MathConstants::maxInfinity };

            double t { hillApproximation( q, dof ) };
            if ( !std::isfinite( t ) || t <= 0. ) {
                t = standardNormalUpperQuantile( q, c );
            }

            for ( int i { 0 }; i < maxIterations; ++i ) {
                const StudentTProbabilities probabilities {
                    studentTProbabilities( t, constants )
                };
                double tNew {};
                if ( tailRegion ) {
                    const double tail { probabilities.tail };
                    if ( tail == q ) {
                        return t;
                    }
                    ( tail > q ? lower : upper ) = t;
                    tNew = t * std::exp(
                        ( std::log( tail ) - lnQ ) * tail /
                        ( t * probabilities.pdf )
                    );
                }
                else {
                    const double difference { probabilities.central - c };
                    if ( difference == 0. ) {
                        return t;
                    }
                    ( difference < 0. ? lower : upper ) = t;
                    tNew = t - difference / ( 2. * probabilities.pdf );
                }

                if ( !( tNew > lower && tNew < upper ) ) {
                    // Newton step outside the bracket, use bisection instead
                    tNew = std::isinf( upper ) ? 2. * std::max( lower, t )
                                               : 0.5 * ( lower + upper );
                }
                else if ( std::abs( tNew - t ) <= tolerance * tNew ) {
                    return tNew;
                }
                t = tNew;
            }
            return t;
        }

        double studentTQuantile(
            double p,
            const StudentTConstants &constants
        ) {
            // Quantile of the standard Student's t-distribution. The upper tail
            // probability q and central probability c are exact in floating
            // point arithmetic in the range where they are used.
            if ( std::isnan( p ) ) {
                return MathConstants::nan;
            }
            if ( p <= 0. ) {
                return MathConstants::minInfinity;
            }
            if ( p >= 1. ) {
                return MathConstants::maxInfinity;
            }
            const bool upperHalf { p > 0.5 };
            const double q { upperHalf ? 1. - p : p };
            const double c { upperHalf ? 2. * p - 1. : 1. - 2. * p };
            const double t { studentTUpperQuantile( q, c, constants ) };
            return upperHalf ? t : -t;
        }
    }


    double coverageFactor( double confidence, double dof ) {
        // GUM JCGM 100:2008 G.3 and G.6.4: the coverage factor is the
        // Student's t quantile for the effective degrees of freedom such that
        // the two-sided interval [-k, k] contains the fraction 'confidence'
        // of the distribution.
        if ( std::isnan( confidence ) || std::isnan( dof ) || dof <= 0. ) {
            return MathConstants::nan;
        }
        if ( confidence <= 0. ) {
            return 0.;
        }
        if ( confidence >= 1. ) {
            return MathConstants::maxInfinity;
        }
        return studentTUpperQuantile(
            0.5 * ( 1. - confidence ),
            confidence,
            studentTConstants( dof )
        );
    }


    double invCDFStandardStudentT( double p, double dof ) {
        if ( std::isnan( dof ) || dof <= 0. ) {
            return MathConstants::nan;
        }
        return studentTQuantile( p, studentTConstants( dof ) );
    }


    InvCDF invCDFStudentsT( double mean, double stdDev, double dof ) {
        // For Student’s t distributions, the entered standard uncertainty is
        // interpreted as the experimental standard deviation s of the mean
        // (GUM Type A). The constants only depend on the degrees of freedom
        // and are calculated once instead of for every sample.

        Q_ASSERT_X(
            dof > 0.,
            "Distribution::invCDFStudentsT",
            "Student's t doesn't exist for dof <= 0"
        );

        const StudentTConstants constants { studentTConstants( dof ) };
        return [ = ]( double u ) {
            return mean + stdDev * studentTQuantile( u, constants );
        };
    }


    double studentTCDF( double t, double dof ) {
        if ( std::isnan( t ) || std::isnan( dof ) || dof <= 0. ) {
            return MathConstants::nan;
        }
        if ( std::isinf( dof ) ) {
            return normalCDF( t );
        }
        const double tail {
            studentTProbabilities(
                std::abs( t ),
                studentTConstants( dof )
            ).tail
        };
        return t > 0. ? 1. - tail : tail;
    }


    bool isNormal( Type distribution, bool dofInfinite ) {
        // A Student's t distribution with infinite DOF is equal to the normal
        // distribution
        return distribution == Type::normal ||
               ( distribution == Type::studentst && dofInfinite );
    }
}
