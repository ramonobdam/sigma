// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef BUDGETMODEL_H
#define BUDGETMODEL_H

#include "outputmodel.h"
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

// This class creates a QAbstractTableModel-interface for the selected
// OutputParameter to show its uncertainty budget components
class BudgetModel: public OutputModel {
    Q_OBJECT
    QML_ELEMENT

public:
    BudgetModel( QObject *parent = nullptr );
    ~BudgetModel();

    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole
    ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

private:
    static constexpr int sNumScaleColors { 8 };

    static int getContributionColorIndex( double contri );
};

#endif // BUDGETMODEL_H
