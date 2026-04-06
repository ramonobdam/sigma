// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "modelcontrol.hpp"
#include "outputparameter.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>

// Abstract class that creates a QAbstractTableModel-interface for the selected
// OutputParameter. This interface is used to display the results in table
// format in the QML interface. Virtual methods data, headerData, rowCount and
// columnCount need to be implemented in the subclass.
class OutputModel: public QAbstractTableModel {
    Q_OBJECT

public:
    OutputModel( QObject *parent = nullptr );
    virtual ~OutputModel() = default;

    ModelControl<OutputParameter *> *getOutputParameterModel() const;
    int getOutputRow() const;
    void emitAllDataChanged();
    void emitModelAboutToBeReset();
    void emitModelReset();
    void setOutputParameterModel( ModelControl<OutputParameter *> *outputModel);

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
    void setOutputRow( int row = -1 );

protected:
    OutputParameter *getOutputParameter() const;

    ModelControl<OutputParameter *> *mOutputParameterModel;
    int mOutputRow;
};

#endif // OUTPUTMODEL_H
