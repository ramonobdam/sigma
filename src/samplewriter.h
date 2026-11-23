// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef SAMPLEWRITER_H
#define SAMPLEWRITER_H

#include <QLatin1StringView>
#include <QSaveFile>
#include <QString>
#include <QStringList>
#include <QUrl>

class SampleBatch;

// Writes Monte Carlo samples to a CSV file, one row per sample: the input
// values (one column per component) followed by the output value. The values
// are written with the shortest representation that round-trips exactly.
// The file is written through a QSaveFile, so the target file is only created
// or replaced when commit() succeeds. Without commit() (e.g. after cancel() or
// when the writer is destroyed) nothing is written to the target file.
class SampleWriter {

public:
    SampleWriter();
    SampleWriter( const SampleWriter & ) = delete;
    SampleWriter &operator= ( const SampleWriter & ) = delete;

    bool open(
        const QUrl &url,
        const QStringList &inputNames,
        const QString &outputName
    );
    bool write( const SampleBatch &batch );
    bool commit();
    void cancel();

    bool isOpen() const;
    const QString &getError() const;
    QString getFileName() const;

    // Returns name as a valid file base name (without extension) on any
    // platform: invalid characters are replaced by '_' and an '_' is added to
    // Windows reserved device names (e.g. 'CON' becomes 'CON_')
    static QString fileBaseName( const QString &name );
    // Returns a unique CSV file name for each of names. File names are
    // compared case-insensitively (as on Windows and macOS). A duplicate gets
    // '_2', '_3', etc. appended, in the order of names, without taking the
    // file name of another name.
    static QStringList uniqueFileNames( const QStringList &names );

private:
    bool writeBuffer( const QByteArray &buffer );

    QSaveFile mFile;
    QString mError;
    int mNumberOfInputs;

    static constexpr QLatin1StringView sInvalidUrlString {
        "'%1' is not a local file"
    };
    static constexpr QLatin1StringView sOpenFailedString {
        "File '%1' could not be opened: %2"
    };
    static constexpr QLatin1StringView sWriteFailedString {
        "File '%1' could not be written: %2"
    };
    static constexpr QLatin1StringView sInputsMismatchString {
        "Input values missing for file '%1'"
    };
    static constexpr QLatin1StringView sCSVExtension { ".csv" };
};

#endif // SAMPLEWRITER_H
