// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "samplewriter.h"
#include "samplebatch.h"
#include "stringutils.h"
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <vector>


namespace {
    // Appends value to buffer using the shortest representation that
    // round-trips exactly (locale independent)
    void appendDouble( QByteArray &buffer, double value ) {
        buffer.append(
            QByteArray::number( value, 'g', QLocale::FloatingPointShortest )
        );
    }
}


SampleWriter::SampleWriter()
    :   mFile {},
        mError {},
        mNumberOfInputs { 0 }
{}


bool SampleWriter::open(
    const QUrl &url,
    const QStringList &inputNames,
    const QString &outputName
) {
    mError.clear();
    if ( !url.isLocalFile() ) {
        mError = sInvalidUrlString.arg( url.toDisplayString() );
        return false;
    }

    mFile.setFileName( url.toLocalFile() );
    if ( !mFile.open( QIODeviceBase::WriteOnly ) ) {
        mError = sOpenFailedString.arg( getFileName(), mFile.errorString() );
        return false;
    }
    mNumberOfInputs = static_cast<int>( inputNames.size() );

    // Header: the input names followed by the output name
    QStringList header {};
    for ( const QString &name : inputNames ) {
        header.append( StringUtils::addQuotes( name ) );
    }
    header.append( StringUtils::addQuotes( outputName ) );
    return writeBuffer(
        ( header.join( StringUtils::csvSeparator ) + StringUtils::endl ).toUtf8()
    );
}


bool SampleWriter::write( const SampleBatch &batch ) {
    if ( !isOpen() ) {
        return false;
    }

    const std::vector<double> &outputs { batch.getSamples() };
    const std::vector<double> &inputs { batch.getInputValues() };
    const size_t nInputs { static_cast<size_t>( mNumberOfInputs ) };
    if (
        static_cast<size_t>( batch.getNumberOfInputs() ) != nInputs ||
        inputs.size() != outputs.size() * nInputs
    ) {
        mError = sInputsMismatchString.arg( getFileName() );
        cancel();
        return false;
    }

    // Build the rows of the whole batch in memory and write them at once.
    // At most 25 characters per value, plus a separator or line end.
    QByteArray buffer {};
    buffer.reserve(
        static_cast<qsizetype>( outputs.size() * ( nInputs + 1 ) * 26 )
    );
    for ( size_t i { 0 }; i < outputs.size(); ++i ) {
        const double *row { inputs.data() + i * nInputs };
        for ( size_t j { 0 }; j < nInputs; ++j ) {
            appendDouble( buffer, row[ j ] );
            buffer.append( ',' );
        }
        appendDouble( buffer, outputs[ i ] );
        buffer.append( '\n' );
    }
    return writeBuffer( buffer );
}


bool SampleWriter::commit() {
    if ( !isOpen() ) {
        return false;
    }
    if ( !mFile.commit() ) {
        mError = sWriteFailedString.arg( getFileName(), mFile.errorString() );
        return false;
    }
    return true;
}


void SampleWriter::cancel() {
    if ( isOpen() ) {
        mFile.cancelWriting();
        mFile.commit();  // Closes the file and discards the temporary file
    }
}


bool SampleWriter::isOpen() const {
    return mFile.isOpen();
}


const QString & SampleWriter::getError() const {
    return mError;
}


QString SampleWriter::getFileName() const {
    return QFileInfo( mFile.fileName() ).fileName();
}


bool SampleWriter::writeBuffer( const QByteArray &buffer ) {
    if ( mFile.write( buffer ) != buffer.size() ) {
        mError = sWriteFailedString.arg( getFileName(), mFile.errorString() );
        cancel();
        return false;
    }
    return true;
}


QString SampleWriter::fileBaseName( const QString &name ) {
    // Replace the characters that are not allowed in file names (on any
    // platform) by an underscore
    QString baseName { name };
    static const QString invalidChars { "<>:\"/\\|?*" };
    for ( QChar &c : baseName ) {
        if ( invalidChars.contains( c ) || c.unicode() < 32 ) {
            c = '_';
        }
    }

    // Windows reserves device names (also when followed by an extension, e.g.
    // 'CON.csv'), so an underscore is added to these names
    static const QRegularExpression reservedNames {
        "^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?=\\.|$)",
        QRegularExpression::CaseInsensitiveOption
    };
    const QRegularExpressionMatch match { reservedNames.match( baseName ) };
    if ( match.hasMatch() ) {
        baseName.insert( match.capturedEnd(), '_' );
    }
    return baseName;
}


QStringList SampleWriter::uniqueFileNames( const QStringList &names ) {
    // The base names of all names are reserved, so that a suffixed name never
    // takes the file name of a later name
    QStringList baseNames {};
    QSet<QString> reserved {};
    for ( const QString &name : names ) {
        baseNames.append( fileBaseName( name ) );
        reserved.insert( baseNames.last().toLower() );
    }

    QStringList fileNames {};
    QSet<QString> used {};
    for ( const QString &baseName : std::as_const( baseNames ) ) {
        QString uniqueName { baseName };
        if ( used.contains( uniqueName.toLower() ) ) {
            int suffix { 2 };
            do {
                uniqueName = baseName + "_" + QString::number( suffix++ );
            } while (
                used.contains( uniqueName.toLower() ) ||
                reserved.contains( uniqueName.toLower() )
            );
        }
        used.insert( uniqueName.toLower() );
        fileNames.append( uniqueName + sCSVExtension );
    }
    return fileNames;
}
