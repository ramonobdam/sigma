// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MODEL_HPP
#define MODEL_HPP

#include "record.h"
#include <QAbstractTableModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QtAssert>
#include <type_traits>

// Template implementation of the QAbstractTableModel class to store objects
// derived from both Record and QObject

template <typename T>
class Model : public QAbstractTableModel {

    static_assert(
        std::is_base_of_v<Record, T> && std::is_base_of_v<QObject, T>,
        "Model class: Type T must derive from both Record and QObject"
    );

public:
    Model( QObject *parent = nullptr ) {
        setParent( parent );
    }


    void clear() {
        beginResetModel();
        for ( T *parameter : mParameters ) {
            delete parameter;
        }
        mParameters.clear();
        endResetModel();
    }


    int columnCount(
        const QModelIndex &parent = QModelIndex()
    ) const override {
        // This item has no children because this is a flat table
        if ( parent.isValid() ) return 0;

        // Root of the tree (parent = invalid)
        return T::staticColumnCount();
    }


    int rowCount( const QModelIndex &parent = QModelIndex() ) const override {
        // This item has no children because this is a flat table
        if ( parent.isValid() ) return 0;

        // Root of the tree (parent = invalid)
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


    T *getRow( int row ) const {
        if ( row >= 0 && row < rowCount() ) {
            return mParameters.at( row );
        }
        return nullptr;
    }


    QList<T *> getAllRows() const {
        return mParameters;
    }


    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole
    ) const override {
        if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
            return T::staticHeaderData( section );
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


    void updateRow( int row, const T &parameter ) {
        if ( row >= 0 && row < rowCount() ) {
            *mParameters.at( row ) = parameter;
            emitRowChanged( row );
        }
    }


    bool removeRows(
        int row,
        int count,
        const QModelIndex &parent = QModelIndex()
    ) override {
        if ( parent.isValid() ) return false;     // Flat table, no children

        const int last { row + count - 1 };
        if ( row < 0 || last >= rowCount() ) return false;

        beginRemoveRows( QModelIndex(), row, last );
        for ( int i { last }; i >= row; --i ) {
            delete mParameters.at( i );
            mParameters.remove( i );
        }
        endRemoveRows();
        return true;
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


    void emitRowChanged( int row ) {
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

#endif // MODEL_HPP
