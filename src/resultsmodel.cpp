// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "resultsmodel.h"
#include "outputparameter.h"


ResultsModel::ResultsModel( QObject *parent ) : OutputModel { parent } {}


ResultsModel::~ResultsModel() {}


QVariant ResultsModel::data( const QModelIndex &index, int role ) const {
    int row { index.row() };
    int column { index.column() };
    if ( !index.isValid() || row >= rowCount() ||
        column >= columnCount() ) {
        return QVariant();
    }

    OutputParameter *parameter { getOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getResult( column );
    }

    return QVariant();
}


QVariant ResultsModel::headerData(
    int section,
    Qt::Orientation orientation,
    int role
    ) const {
    if (
        role == Qt::DisplayRole &&
        orientation == Qt::Horizontal &&
        section >= 0 &&
        section < columnCount()
    ) {
        return OutputParameter::resultLabels[ section ];
    }
    return QVariant();
}


int ResultsModel::columnCount( const QModelIndex &parent ) const {
    return OutputParameter::resultLabels.size();
}


int ResultsModel::rowCount( const QModelIndex &parent ) const {
    // The combined uncertainty results have only one row
    OutputParameter *parameter { getOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return 1;
    }
    return 0;
}
