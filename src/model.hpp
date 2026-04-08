// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MODEL_HPP
#define MODEL_HPP

#include "record.h"
#include "undostack.h"
#include <QAbstractTableModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QtAssert>
#include <type_traits>

// Template implementation of the QAbstractTableModel class to store objects
// derived from both Record and QObject

template <typename T>
class Model : public QAbstractTableModel, public UndoableModel {

    static_assert(
        std::is_base_of_v<Record, T> && std::is_base_of_v<QObject, T>,
        "Model class: Type T must derive from both Record and QObject"
    );

public:
    Model( QObject *parent = nullptr ) : QAbstractTableModel { parent } {}


    QJsonObject currentJson( const QString &objectId ) const override {
        T *record { getRowById( objectId ) };
        if ( record ) {
            return record->toJson();
        }
        return QJsonObject {};
    }


    void applyDiff( const JsonDiff &diff ) override {
        if ( diff.after.isEmpty() ) {
            // Remove object that was added before
            removeRowById( diff.objectId );
        } else if ( diff.before.isEmpty() ) {
            // Re-add object that was deleted before
            T record { T::fromJson( diff.after ) };
            appendRow( record );
        } else {
            // Object was modified — restore state
            T *record { getRowById( diff.objectId ) };
            if ( record ) {
                record->updateFromJson( diff.after );
                emitRowChanged( getRowIndex( record ) );
            }
        }
    }


    void recordWasUpdated( T *record ) {
        // Used when a record is updated outside of the Model interface (i.e.
        // in-place)
        if ( record ) {
            UndoStack::instance().snapshot(
                record->getName(),
                record->dataType(),
                record->toJson()
            );
            emitRowChanged( getRowIndex( record ) );
        }
    }


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


    T *getRow( int row ) const {
        if ( row >= 0 && row < rowCount() ) {
            return mRecords.at( row );
        }
        return nullptr;
    }


    T *getRowById( const QString &id ) const {
        for ( T *record : mRecords ) {
            if ( record->getName() == id ) {
                return record;
            }
        }
        return nullptr;
    }


    int getRowIndex( T *record ) const {
        return mRecords.indexOf( record );
    }


    QList<T *> getAllRows() const {
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


    void appendRow( const T &record ) {
        UndoStack::instance().snapshot(
            record.getName(),
            record.dataType(),
            QJsonObject {}    // empty = did not exist yet
        );

        beginInsertRows( QModelIndex(), rowCount(), rowCount() );
        T *newRecord { new T { record } };
        if ( !newRecord->parent() ) {
            // Record has no parent, set its parent to this
            newRecord->setParent ( this );
        }
        mRecords.append( newRecord );
        endInsertRows();
    }


    void updateRow( int row, const T &record ) {
        if ( row >= 0 && row < rowCount() ) {
            T *existing { mRecords.at( row ) };

            UndoStack::instance().snapshot(
                existing->getName(),
                existing->dataType(),
                existing->toJson()
            );

            *existing = record;
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
            T *record { mRecords.at( i ) };

            UndoStack::instance().snapshot(
                record->getName(),
                record->dataType(),
                record->toJson()
            );

            delete record;
            mRecords.remove( i );
        }
        endRemoveRows();
        return true;
    }


    void removeRowById( const QString &id ) {
        int row { getRowIndex( getRowById( id ) ) };
        if ( row >= 0 ) removeRows( row, 1 );
    }


    bool nameIsPresent( const QString &name ) const {
        for ( const T *record : getAllRows()) {
            if ( record->getName().toLower() == name.toLower() ) {
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
        T *record { getRow( row ) };

        UndoStack::instance().snapshot(
            record->getName(),
            record->dataType(),
            record->toJson()
        );

        record->set( column, value );
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
    QList<T *> mRecords;

    // sHeaderRecord is used to determine the number of columns and the header
    // labels
    inline static const T sHeaderRecord {};
};

#endif // MODEL_HPP
