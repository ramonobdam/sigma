// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "budgetmodel.h"
#include "outputparameter.h"
#include "uncertaintycomponent.h"


BudgetModel::BudgetModel( QObject *parent ) : OutputModel { parent } {}


BudgetModel::~BudgetModel() {}


QVariant BudgetModel::data( const QModelIndex &index, int role ) const {
    int row { index.row() };
    int column { index.column() };
    if ( !index.isValid() || row >= rowCount( QModelIndex() ) ||
        column >= columnCount( QModelIndex() ) ) {
        return QVariant();
    }

    OutputParameter *parameter { getOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        UncertaintyComponent *component { parameter->getComponent( row ) };
        // First columns can be derived from the component directly
        if ( column < ( UncertaintyComponent::headerLabels.size() - 3 ) ) {
            if ( role == Qt::DisplayRole ) {
                return component->get( column );
            }
        }
        else {
            // Component contributions depend on the combined standard
            // uncertainty of the OutputParameter
            if ( role == Qt::DisplayRole ) {
                return parameter->getContributionAsString( row, column );
            }
            else if ( role == Qt::DecorationRole ) {
                double contri { parameter->getContribution( row, column )};
                return getContributionColorIndex( contri );
            }
        }
    }
    return QVariant();
}


QVariant BudgetModel::headerData(
    int section, Qt::Orientation orientation,
    int role
) const {
    if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
        return UncertaintyComponent::headerLabels[ section ];
    }
    return QVariant();
}


int BudgetModel::columnCount( const QModelIndex &parent ) const {
    return UncertaintyComponent::headerLabels.size();
}


int BudgetModel::rowCount( const QModelIndex &parent ) const {
    OutputParameter *parameter { getOutputParameter() };
    if ( parameter && parameter->getValid() ) {
        return parameter->getNumberOfComponents();
    }
    return 0;
}
