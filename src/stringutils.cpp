// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"
#include <cmath>
#include <cstdint>

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


    bool unicodeLess( const std::wstring &a, const std::wstring &b ) {
        // Compare strings by Unicode code point rather than by locale.
        //
        // This deliberately does not use locale-aware collation, because the
        // sort order must be identical on all platforms and independent of the
        // user's locale. For example, Greek capital Delta (U+0394) sorts before
        // Greek small delta (U+03B4).
        return unicodeCodePoints( a ) < unicodeCodePoints( b );
    }


    std::u32string unicodeCodePoints( const std::wstring &s ) {
        // Convert std::wstring to a sequence of Unicode code points.
        //
        // std::wstring has a platform-dependent representation:
        //   - Windows/MSVC: wchar_t is 16-bit (UTF-16)
        //   - Linux/macOS:  wchar_t is typically 32-bit (UTF-32)
        //
        // Decoding to Unicode code points makes the comparison independent
        // of the platform's wchar_t representation.
        std::u32string result;
        result.reserve(s.size());

        for ( std::size_t i = 0; i < s.size(); ++i ) {
            const std::uint32_t c = static_cast<std::uint32_t>( s[ i ] );

            // On Windows, characters outside the BMP are represented by
            // a UTF-16 surrogate pair. Combine the pair into one code point
            // so that it is compared consistently with UTF-32 platforms.
            if ( c >= 0xD800 && c <= 0xDBFF && i + 1 < s.size() ) {
                const std::uint32_t c2 =
                    static_cast<std::uint32_t>( s [ ++i ] );

                if ( c2 >= 0xDC00 && c2 <= 0xDFFF ) {
                    const char32_t codePoint =
                        0x10000 +
                        ( (c - 0xD800 ) << 10 ) +
                        ( c2 - 0xDC00 );

                    result.push_back( codePoint );
                    continue;
                }

                // Invalid surrogate pair. Treat the first value as a
                // standalone character rather than silently discarding it.
                --i;
            }

            result.push_back( static_cast<char32_t>( c ));
        }

        return result;
    }
}
