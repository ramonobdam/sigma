// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNDOHISTORYMODEL_H
#define UNDOHISTORYMODEL_H

#include "undostack.h"
#include <QAbstractTableModel>
#include <QLatin1StringView>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <Qt>

// Read-only table model that exposes the UndoStack transaction history to QML.
// Each row represents one transaction. Method refresh() is connected to the
// UndoStack::stackSizeChanged signal to reset the model when the undoStack is
// updated.
class UndoHistoryModel : public QAbstractTableModel {
    Q_OBJECT

public:
    UndoHistoryModel( QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

private:
    void refresh();

    static constexpr QLatin1StringView sInitialStateString {
        "[Initial state]"
    };
};

#endif // UNDOHISTORYMODEL_H
