// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef DATA_H
#define DATA_H

#include "datatype.h"
#include "record.h"
#include <QLatin1StringView>
#include <QString>

// Base class for all data objects that require a stable unique ID for undo/redo
class Data : public Record {
public:
    Data();

    Data( const Data &dat );
    Data &operator= ( const Data &dat );

    QUuid getId() const override;
    void resetId();

protected:
    void setId( const QUuid &id );

    static constexpr QLatin1StringView mIdString { "Id" };

private:
    QUuid mId;
};

#endif // DATA_H
