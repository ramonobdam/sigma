// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "lockableitemselectionmodel.h"


LockableItemSelectionModel::LockableItemSelectionModel(
    QAbstractItemModel *model,
    QObject *parent
)
    :   QItemSelectionModel { model, parent }
{}


LockableItemSelectionModel::~LockableItemSelectionModel() {}


void LockableItemSelectionModel::setSelectionLocked( const bool &locked ) {
    mSelectionLocked = locked;
}


bool LockableItemSelectionModel::getSelectionLocked() const {
    return mSelectionLocked;
}


void LockableItemSelectionModel::setCurrentIndex(
    const QModelIndex &index,
    SelectionFlags command
) {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::setCurrentIndex( index, command );
}


void LockableItemSelectionModel::select(
    const QModelIndex &index,
    SelectionFlags command
) {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::select( index, command );
}


void LockableItemSelectionModel::select(
    const QItemSelection &selection,
    SelectionFlags command
) {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::select( selection, command );
}


void LockableItemSelectionModel::clear() {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::clear();
}


void LockableItemSelectionModel::clearCurrentIndex() {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::clearCurrentIndex();
}


void LockableItemSelectionModel::reset() {
    if ( mSelectionLocked ) {
        return;
    }
    QItemSelectionModel::reset();
}
