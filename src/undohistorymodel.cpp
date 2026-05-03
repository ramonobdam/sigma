// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "undohistorymodel.h"


UndoHistoryModel::UndoHistoryModel( QObject *parent )
    :   QAbstractTableModel { parent }
{
    connect(
        &UndoStack::instance(),
        &UndoStack::stackSizeChanged,
        this,
        &UndoHistoryModel::refresh
    );
}


QVariant UndoHistoryModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid() || index.row() >= rowCount() ) {
        return QVariant {};
    }
    if ( role == Qt::DisplayRole ) {
        // The first row represents the intial state
        if ( index.row() == 0 ) return sInitialStateString;
        return UndoStack::instance().getLabelAt( index.row() - 1 );
    }
    return QVariant {};
}


int UndoHistoryModel::columnCount( const QModelIndex &parent ) const {
    // This item has no children because this is a flat table
    if ( parent.isValid() ) return 0;
    return 1;
}


int UndoHistoryModel::rowCount( const QModelIndex &parent ) const {
    // This item has no children because this is a flat table
    if ( parent.isValid() ) return 0;
    return UndoStack::instance().getStackSize() + 1;    // +1 for initial state
}


void UndoHistoryModel::refresh() {
    beginResetModel();
    endResetModel();
}
