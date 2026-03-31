// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "outputmodel.h"
#include <QList>

OutputModel::OutputModel( QObject *parent )
    :   QAbstractTableModel { parent },
        mOutputRow { -1 },
        mOutputParameterModel {}
{}


int OutputModel::getOutputRow() const {
    return mOutputRow;
}


void OutputModel::emitAllDataChanged() {
    QList<int> roles { Qt::DisplayRole, Qt::DecorationRole };
    emit dataChanged(
        index( 0, 0 ),
        index( rowCount() - 1, columnCount() - 1 ),
        roles
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


void OutputModel::setOutputRow( const int &row ) {
    if ( row != mOutputRow ) {
        beginResetModel();
        mOutputRow = row;
        endResetModel();
    }
}


OutputParameter * OutputModel::getOutputParameter() const {
    if ( mOutputParameterModel ) {
        return mOutputParameterModel->getRow( mOutputRow );
    }
    return nullptr;
}
