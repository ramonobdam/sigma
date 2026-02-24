// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MODEL_H
#define MODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QModelIndex>

// Specialization of the QAbstractTableModel class for parameter type T. It is
// used to store and display Parameter (derived) and Correlation objects.
template <typename T>
class Model : public QAbstractTableModel {

public:
    Model( QObject *parent = nullptr ) {
        setParent( parent );
    }


    ~Model() {}


    void clear() {
        beginResetModel();
        for ( T* parameter : mParameters )
        {
            if ( parameter ) {
                delete parameter;
            }
        }
        mParameters.clear();
        endResetModel();
    }


    int columnCount(
        const QModelIndex &parent = QModelIndex()
    ) const override {
        return T::headerLabels.size();
    }


    int rowCount( const QModelIndex &parent = QModelIndex() ) const override {
        return mParameters.size();
    }


    QVariant data( const QModelIndex &index, int role ) const override {
        int row { index.row() };
        int column { index.column() };
        if (
            !index.isValid() ||
            row >= rowCount() ||
            column >= columnCount()
        ) {
            return QVariant();
        }
        T *parameter { mParameters.at( row ) };
        if ( role == Qt::DisplayRole ) {
            return parameter->get( index.column() );
        }
        if ( role == Qt::DecorationRole ) {
            return parameter->getValid();
        }
        return QVariant();
    }


    T *getRow( const int &row ) const {
        if ( row >= 0 && row < rowCount() ) {
            return mParameters.at( row );
        }
        return nullptr;
    }


    QList<T *> getAllRows() const {
        QList<T *> rows {};
        for ( int row { 0 }; row < rowCount(); ++row ) {
            rows.append( getRow( row ) );
        }
        return rows;
    }


    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole
    ) const override {
        if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
            return T::headerLabels[ section ];
        }
        return QVariant();
    }


    void appendRow( const T &parameter ) {
        beginInsertRows( QModelIndex(), rowCount(), rowCount() );
        T *newParameter { new T { parameter } };
        if ( !newParameter->parent() ) {
            // Parameter has no parent, set its parent to this
            newParameter->setParent ( this );
        }
        mParameters.append( newParameter );
        endInsertRows();
    }


    void updateRow( const int &row, const T &parameter ) {
        if ( row >= 0 && row < rowCount() ) {
            *mParameters.at( row ) = parameter;
            emitRowChanged( row );
        }
    }


    void removeRow( const int &row ) {
        if ( row >= 0 && row < rowCount() ) {
            beginRemoveRows( QModelIndex(), row, row );
            delete mParameters.at( row );
            mParameters.remove( row );
            endRemoveRows();
        }
    }


    bool nameIsPresent( const QString &name ) const {
        for ( const T *parameter : getAllRows()) {
            if ( parameter->getName().toLower() == name.toLower() ) {
                return true;
            }
        }
        return false;
    }


    Qt::ItemFlags flags( const QModelIndex &index ) const override {
        return QAbstractTableModel::flags( index ) | Qt::ItemIsEditable;
    }


    bool setData(
        const QModelIndex &index,
        const QVariant &value,
        int role
    ) override {
        if ( role != Qt::EditRole || value.isNull() ) {
            return false;
        }
        int row { index.row() };
        int column { index.column() };
        if (
            !index.isValid() ||
            row >= rowCount() ||
            column >= columnCount()
        ) {
            return false;
        }
        T *parameter { getRow( row ) };
        parameter->set( column, value );
        emit dataChanged(
            index,
            index,
            { Qt::DisplayRole, Qt::DecorationRole }
        );
        return true;
    }


    void emitRowChanged( const int &row ) {
        emit dataChanged(
            index( row, 0 ),
            index( row, columnCount() -1 ),
            { Qt::DisplayRole, Qt::DecorationRole }
        );
    }


    void emitAllDataChanged() {
        emit dataChanged(
            index( 0, 0 ),
            index( rowCount() - 1, columnCount() - 1 ),
            { Qt::DisplayRole, Qt::DecorationRole }
        );
    }


private:
    QList<T *> mParameters;
};

#endif // MODEL_H
