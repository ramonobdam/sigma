// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef BUDGETMODEL_H
#define BUDGETMODEL_H

#include "outputmodel.h"
#include "utils.h"
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

// This class shows the uncertainty budget components of the combined
// uncertainty for the selected OutputParameter in QAbstractTableModel format
class BudgetModel: public OutputModel, Utils {
    Q_OBJECT
    QML_ELEMENT

public:
    BudgetModel( QObject *parent = nullptr );
    ~BudgetModel();

    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role
    ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
};

#endif // BUDGETMODEL_H
