// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "outputmodel.h"


OutputModel::OutputModel( QObject *parent )
    :   QAbstractTableModel { parent },
        mCurrentIndex {},
        mInputParameterModel { InputParameter::getInputModel() },
        mOutputParameterModel { OutputParameter::getOutputModel() }
{
    // Create connections to the OutputParameter model
    connect(
        mOutputParameterModel->selectionModel(),
        &QItemSelectionModel::currentRowChanged,
        this,
        &OutputModel::setCurrentIndex
    );

    connect(
        mOutputParameterModel->itemModel(),
        &QAbstractItemModel::dataChanged,
        this,
        &OutputModel::onOutputParameterDataChanged
    );

    connect(
        mOutputParameterModel->itemModel(),
        &QAbstractItemModel::rowsRemoved,
        this,
        &OutputModel::onOutputParameterRowsRemoved
    );

    connect(
        mOutputParameterModel->itemModel(),
        &QAbstractItemModel::modelReset,
        this,
        &OutputModel::onOutputParameterModelReset
    );

    // Create connections to the InputParameter model
    connect(
        mInputParameterModel->itemModel(),
        &QAbstractItemModel::dataChanged,
        this,
        &OutputModel::onInputParameterDataChanged
    );
    connect(
        mInputParameterModel->itemModel(),
        &QAbstractItemModel::modelReset,
        this,
        &OutputModel::onInputParameterDataChanged
    );
}


QPersistentModelIndex OutputModel::getCurrentIndex() const {
    return mCurrentIndex;
}


void OutputModel::onInputParameterDataChanged() {
    // An InputParameter was updated. If the InputParameter does not change
    // numerically, the OutputParameters are not recompiled. The OutputModel is
    // reset here to make sure the updated InputParameter is displayed.
    beginResetModel();
    endResetModel();
}


void OutputModel::onOutputParameterDataChanged(
    const QModelIndex &topLeft,
    const QModelIndex &bottomRight,
    const QList<int> &roles
) {
    if ( !mCurrentIndex.isValid() ) return;

    const int row { mCurrentIndex.row() };
    if ( row < topLeft.row() || row > bottomRight.row() ) {
        // The changed data is not displayed
        return;
    }

    beginResetModel();
    endResetModel();
}


void OutputModel::onOutputParameterModelReset() {
    // Reset the model and display the current index of the OutputParameter
    // model
    beginResetModel();
    mCurrentIndex = mOutputParameterModel->selectionModel()->currentIndex();
    endResetModel();
}


void OutputModel::onOutputParameterRowsRemoved() {
    // The QPersistentModelIndex becomes invalid when the item it is
    // referencing is removed
    if ( mCurrentIndex.isValid() )
        return;

    // Displayed OutputParameter was removed
    beginResetModel();
    endResetModel();
}


void OutputModel::setCurrentIndex( const QModelIndex &index ) {
    if ( mCurrentIndex != index ) {
        beginResetModel();
        mCurrentIndex = index;
        endResetModel();
    }
}


OutputParameter * OutputModel::getOutputParameter() const {
    if ( mOutputParameterModel ) {
        return mOutputParameterModel->getByRow( mCurrentIndex.row() );
    }
    return nullptr;
}
