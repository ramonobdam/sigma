// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef LOCKABLEITEMSELECTIONMODEL_H
#define LOCKABLEITEMSELECTIONMODEL_H

#include <QAbstractItemModel>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>

// LockableItemSelectionModel adds the ability to lock the selection and current
// index to QItemSelectionModel. When the selection model is not locked, the
// overridden methods use their base class version, otherwise they return
// without changing the selection or current index.
class LockableItemSelectionModel : public QItemSelectionModel {
    Q_OBJECT

public:
    LockableItemSelectionModel( QAbstractItemModel *model, QObject *parent );
    ~LockableItemSelectionModel();

    void setSelectionLocked( const bool &locked );
    bool getSelectionLocked() const;

public slots:
    void setCurrentIndex(
        const QModelIndex &index,
        QItemSelectionModel::SelectionFlags command
    ) override;
    void select(
        const QModelIndex &index,
        QItemSelectionModel::SelectionFlags command
    ) override;
    void select(
        const QItemSelection &selection,
        QItemSelectionModel::SelectionFlags command
    ) override;
    void clear() override;
    void clearCurrentIndex() override;
    void reset() override;

private:
    bool mSelectionLocked;
};

#endif // LOCKABLEITEMSELECTIONMODEL_H
