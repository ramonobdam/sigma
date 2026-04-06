// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "outputmodel.h"


OutputModel::OutputModel( QObject *parent )
    :   QAbstractTableModel { parent },
        mOutputRow { -1 },
        mOutputParameterModel {}
{}


ModelControl<OutputParameter *> * OutputModel::getOutputParameterModel() const {
    return mOutputParameterModel;
}


int OutputModel::getOutputRow() const {
    return mOutputRow;
}


void OutputModel::emitAllDataChanged() {
    emit dataChanged(
        index( 0, 0 ),
        index( rowCount() - 1, columnCount() - 1 ),
        { Qt::DisplayRole, Qt::DecorationRole }
    );
}


void OutputModel::emitModelAboutToBeReset() {
    beginResetModel();
}


void OutputModel::emitModelReset() {
    endResetModel();
}


void OutputModel::setOutputParameterModel(
    ModelControl<OutputParameter *> *outputModel
) {
    mOutputParameterModel = outputModel;
}


void OutputModel::setOutputRow( int row ) {
    if ( row != getOutputRow() ) {
        beginResetModel();
        mOutputRow = row;
        endResetModel();
    }
}


OutputParameter * OutputModel::getOutputParameter() const {
    if ( getOutputParameterModel() ) {
        return getOutputParameterModel()->getRow( getOutputRow() );
    }
    return nullptr;
}
