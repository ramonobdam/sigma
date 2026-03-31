// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UTILS_H
#define UTILS_H

#include "third_party/exprtk/exprtk.hpp"
#include <QString>
#include <cmath>

// Small mixin class with utilities
class Utils {

public:
    Utils();
    ~Utils();

    static QString addQuotes( const QString &string );
    static QString contributionToPercentageString(
        const double &contri,
        const int &decimals = 1
    );
    static int getContributionColorIndex( const double &contri );

    static const int numScaleColors { 8 };
    static double pi;
    static QString endl;

};

#endif // UTILS_H
