// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include "modelcontrol.hpp"
#include "outputparameter.h"
#include <QAbstractTableModel>
#include <QObject>

// Base class that wraps a QAbstractTableModel around the selected
// OutputParameter. This is used to display the results in table format in the
// QML interface. Virtual methods data, headerData, rowCount and columnCount
// need to be overriden in the derived class.
class OutputModel: public QAbstractTableModel {
    Q_OBJECT

public:
    OutputModel( QObject *parent = nullptr );
    ~OutputModel();

    int getOutputRow() const;
    void emitAllDataChanged();
    void emitModelAboutToBeReset();
    void emitModelReset();
    void setOutputParameterModel( ModelControl<OutputParameter *> *outputModel);

public slots:
    void setOutputRow( const int &row = -1 );

protected:
    OutputParameter *getOutputParameter() const;

    ModelControl<OutputParameter *> *mOutputParameterModel;
    int mOutputRow;
};

#endif // OUTPUTMODEL_H
