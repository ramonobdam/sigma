// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "jsondiff.h"
#include <QList>
#include <QString>

// A transaction groups all JsonDiffs produced by a single user action into one
// undoable unit
class Transaction {
public:
    Transaction();

    QString getLabel() const;
    const QList<JsonDiff> &getDiffs() const;
    QList<JsonDiff> &getDiffs();
    bool isEmpty() const;
    void addDiff( const JsonDiff &diff );
    void setLabel( const QString &label );

private:
    QList<JsonDiff> mDiffs;
    QString mLabel;
};

#endif // TRANSACTION_H
