// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"
#include <cmath>

namespace StringUtils {

    QString doubleToString( double value, int precision , char format ) {
        return QString::number( value, format, precision );
    }


    QString addQuotes( const QString &string ) {
        // A double quote is escaped (for CSV files) by preceding it with
        // another double quote
        QString result { string };
        result.replace( quote, quote + quote );
        return quote + result + quote;
    }


    QString contributionToPercentageString( double contri, int decimals ) {
        QString percentage { "-" };
        if ( std::isfinite( contri ) ) {
            percentage = QString::number( contri * 100., 'f', decimals ) + "%";
        }
        return percentage;
    }

}
