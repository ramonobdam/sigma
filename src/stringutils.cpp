// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"

namespace StringUtils {

    QString doubleToString( double value, int precision ) {
        return QString::number( value, 'g', precision );
    }

}
