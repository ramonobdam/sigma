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
#include <QUuid>
#include <QtMinMax>
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
    Model( QObject *parent = nullptr ) : QAbstractTableModel { parent } {}


    void clear() {
        removeRows( 0, rowCount() );
    }


    int columnCount(
        const QModelIndex &parent = QModelIndex()
    ) const override {
        // This item has no children because this is a flat table
        if ( parent.isValid() ) return 0;

        // Root of the tree (parent = invalid)
        return sHeaderRecord.columnCount();
    }


    int rowCount( const QModelIndex &parent = QModelIndex() ) const override {
        // This item has no children because this is a flat table
        if ( parent.isValid() ) return 0;

        // Root of the tree (parent = invalid)
        return mRecords.size();
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
        T *record { mRecords.at( row ) };
        if ( role == Qt::DisplayRole ) {
            return record->get( index.column() );
        }
        if ( role == Qt::DecorationRole ) {
            return record->getValid();
        }
        return QVariant();
    }


    T *getByRow( int row ) const {
        if ( row >= 0 && row < rowCount() ) {
            return mRecords.at( row );
        }
        return nullptr;
    }


    T *getById( const QUuid &id ) const {
        if ( !id.isNull() ) {
            for ( T *record : getAllRows() ) {
                if ( record->getId() == id ) {
                    return record;
                }
            }
        }
        return nullptr;
    }


    T *getByName( const QString &name ) const {
        if ( name.size() > 0 ) {
            for ( T *record : getAllRows() ) {
                if ( record->getName().toLower() == name.toLower() ) {
                    return record;
                }
            }
        }
        return nullptr;
    }


    int getRowIndex( const QUuid &id ) const {
        if ( !id.isNull() ) {
            for ( int row { 0 }; row < rowCount(); ++row ) {
                if ( mRecords.at( row )->getId() == id ) {
                    return row;
                }
            }
        }
        return -1;
    }


    const QList<T *> &getAllRows() const {
        return mRecords;
    }


    QVariant headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole
    ) const override {
        if ( role == Qt::DisplayRole && orientation == Qt::Horizontal ) {
            return sHeaderRecord.headerData( section );
        }
        return QVariant();
    }


    T* insertRow( int row, const T &record ) {
        const int boundedRow { qBound( 0, row, rowCount() ) };
        T *newRecord { new T { record } };
        if ( !newRecord->parent() ) {
            // Record has no parent, set its parent to this
            newRecord->setParent ( this );
        }
        beginInsertRows( QModelIndex(), boundedRow, boundedRow );
        mRecords.insert( boundedRow, newRecord );
        endInsertRows();
        return mRecords.at( boundedRow );
    }


    T* appendRow( const T &record ) {
        return insertRow( rowCount(), record );
    }


    T* updateByRow( int row, const T &record ) {
        if ( row >= 0 && row < rowCount() ) {
            T *existing { mRecords.at( row ) };
            *existing = record;
            emitRowChanged( row );
            return existing;
        }
        return nullptr;
    }


    T* updateById( const QUuid &id, const T &record ) {
        return updateByRow( getRowIndex( id ), record );
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
            T *record { mRecords.at( i ) };
            delete record;
            mRecords.remove( i );
        }
        endRemoveRows();
        return true;
    }


    bool removeById( const QUuid &id ) {
        return removeRows( getRowIndex( id ), 1 );
    }


    bool nameIsPresent( const QString &name ) const {
        return getByName( name );
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
        T *record { getByRow( row ) };
        record->set( column, value );
        emit dataChanged(
            index,
            index,
            { Qt::DisplayRole, Qt::DecorationRole }
        );
        return true;
    }


    void emitIdChanged( const QUuid &id ) {
        const int row { getRowIndex( id ) };
        if ( row >= 0 && row < rowCount() ) {
            emitRowChanged( row );
        }
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
    QList<T *> mRecords;

    // sHeaderRecord is used to determine the number of columns and the header
    // labels
    inline static const T sHeaderRecord {};
};

#endif // MODEL_HPP
