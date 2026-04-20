// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "inputparameter.h"
#include "modelcontrol.hpp"
#include "outputparameter.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QPersistentModelIndex>
#include <QVariant>

// Abstract class that creates a QAbstractTableModel-interface for the selected
// OutputParameter. This interface is used to display the results in table
// format in the QML interface. Connections are created to the OutputParameter
// and InputParameter models to respond to selection and data changed signals.
// Virtual methods data, headerData, rowCount and columnCount need to be
// implemented in the subclass.
class OutputModel: public QAbstractTableModel {
    Q_OBJECT

public:
    OutputModel( QObject *parent = nullptr );
    virtual ~OutputModel() = default;

    QPersistentModelIndex getCurrentIndex() const;

    virtual int columnCount(
        const QModelIndex &parent = QModelIndex()
    ) const = 0;
    virtual QVariant data( const QModelIndex &index, int role ) const = 0;
    virtual QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole
    ) const = 0;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const = 0;

public slots:
    void onInputParameterDataChanged();
    void onOutputParameterDataChanged(
        const QModelIndex &topLeft,
        const QModelIndex &bottomRight,
        const QList<int> &roles
    );
    void onOutputParameterModelReset();
    void onOutputParameterRowsRemoved();
    void setCurrentIndex( const QModelIndex &index );

protected:
    OutputParameter *getOutputParameter() const;

    ModelControl<InputParameter *> *mInputParameterModel;
    ModelControl<OutputParameter *> *mOutputParameterModel;
    QPersistentModelIndex mCurrentIndex;
};

#endif // OUTPUTMODEL_H
