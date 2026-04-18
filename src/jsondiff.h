// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef JSONDIFF_H
#define JSONDIFF_H

#include "datatype.h"
#include <QJsonObject>
#include <QUuid>

// Stores the before and after state of a single object change. An empty before
// means the object did not exist yet. An empty after means the object was
// deleted.
struct JsonDiff {
    QUuid objectId;
    DataType objectType;
    int row { -1 };     // row index for restore
    QJsonObject before; // empty = object did not exist yet
    QJsonObject after;  // empty = object was deleted
};

#endif // JSONDIFF_H
