// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "stringutils.h"
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <cmath>
#ifdef Q_OS_WINDOWS
#include <clocale>
#endif
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


    void setupLocale() {
        // The Qlocale is set to US to have the number format (decimal . and
        // thousand separator ,) consistent wih exprtk
        QLocale::setDefault(
            QLocale( QLocale::English, QLocale::UnitedStates )
        );

        // The C-locale is set to UTF-8 to make sure std::towlower() handles
        // Unicode correctly on Windows
#ifdef Q_OS_WINDOWS
        std::setlocale( LC_ALL, ".UTF-8" );
#endif
    }


    QString urlToLocalPath( const QUrl &url ) {
        // QUrl::toLocalFile() handles drive letters, network paths and encoded
        // characters. Other URLs are shown as they are.
        if ( url.isLocalFile() ) {
            return QDir::toNativeSeparators( url.toLocalFile() );
        }
        return url.toDisplayString();
    }


    QString urlToFileName( const QUrl &url ) {
        if ( url.isLocalFile() ) {
            return QFileInfo( url.toLocalFile() ).fileName();
        }
        return url.fileName();
    }

}
