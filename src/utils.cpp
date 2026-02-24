// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "utils.h"


QString Utils::endl = "\n";

double Utils::pi = exprtk::details::numeric::constant::pi;


Utils::Utils() {}


Utils::~Utils() {}


QString Utils::addQuotes( const QString &string ) {
    // Double quotes are escaped by preceding it with another double quote.
    QString result { string };
    QString quote { "\"" };
    result.replace( quote, quote + quote );
    return quote + result + quote;
}

QString Utils::contributionToPercentageString(
    const double &contri,
    const int &decimals
) {
    QString percentage {"-"};
    if ( std::isfinite( contri ) ) {
        percentage = QString::number( contri * 100., 'f', decimals ) + "%";
    }
    return percentage;
}


int Utils::getContributionColorIndex( const double &contri ) {
    if ( std::isfinite( contri ) ) {
        int i {
            static_cast<int> (
                std::floor( std::abs( contri * numScaleColors ) )
                )
        };
        return i > numScaleColors - 1 ? numScaleColors - 1 : i;
    }
    return 0;
}
