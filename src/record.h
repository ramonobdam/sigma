// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef RECORD_H
#define RECORD_H

#include <QVariant>
#include <QString>

// Abstract class that defines the interface for data model records
class Record {

public:
    virtual ~Record() = default;

    virtual QVariant get( int column, bool csvMode = false ) const = 0;
    virtual QVariant headerData( int column ) const = 0;
    virtual QString getName( bool csvMode = false ) const = 0;
    virtual bool getValid() const = 0;
    virtual int columnCount() const = 0;
    virtual void set( int column, const QVariant &value ) = 0;
};

#endif // RECORD_H
