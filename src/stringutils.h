// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QLatin1StringView>
#include <QString>

// Namespace with string conversion functions and string constants
namespace StringUtils {
    QString doubleToString( double value, int precision );

    constexpr QLatin1StringView csvSeparator { "," };
}

#endif // STRINGUTILS_H
