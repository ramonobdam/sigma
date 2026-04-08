// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MODELCONTROL_HPP
#define MODELCONTROL_HPP

#include "lockableitemselectionmodel.h"
#include "model.hpp"
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>

// Class that combines a Model and LockableItemSelectionModel for type T
template <typename pT>
class ModelControl : public QObject {
    using T = typename std::remove_pointer_t<pT>;

public:
    ModelControl( QObject *parent = nullptr )
        :   mModel { Model<T>( this ) },
            mSelection { LockableItemSelectionModel { &mModel, this } }
    {
        setParent( parent );
    }


    QObject *itemModel() {
        return static_cast<QObject *> ( &mModel );
    }


    LockableItemSelectionModel *selectionModel() {
        return &mSelection;
    }


    int columnCount() const {
        return mModel.columnCount();
    }


    int rowCount() const {
        return mModel.rowCount();
    }


    bool aRowIsSelected() const {
        return selectedRow() >= 0;
    }


    T *appendRow( const T &parameter ) {
        mModel.appendRow( parameter );
        const int row { rowCount() - 1 };
        selectRow( row );
        return getRow( row );
    }


    void updateSelectedRow( const T &parameter ) {
        const int row { selectedRow() };
        if ( row >= 0 ) {
            updateRow( row, parameter );
        }
    }


    void emitRowChanged( int row ) {
        mModel.emitRowChanged( row );
    }


    void emitAllDataChanged() {
        mModel.emitAllDataChanged();
    }


    void updateRow( int row, const T &parameter ) {
        mModel.updateRow( row, parameter );
    }


    T *getRow( int row ) const {
        return mModel.getRow( row );
    }


    QList<T *> getAllRows() const {
        return mModel.getAllRows();
    }


    T *getSelectedRow() const {
        const int row = { selectedRow() };
        if ( row >= 0 ) {
            return getRow( row );
        }
        return nullptr;
    }


    QVariant getSelectedItem( int column = 0 ) const {
        T *parameter { getSelectedRow() };
        if ( parameter ) {
            return parameter->get( column );
        }
        return QVariant();
    }


    bool removeRow( int row ) {
        if ( row >= 0 && row < rowCount() ) {
            mModel.removeRow( row );
            // Update the selected row when the table is not empty, else clear
            // the selection
            if ( rowCount() > 0 ) {
                if ( row - 1 >= 0 ) {
                    selectRow( row - 1 );
                }
                else {
                    selectRow( row );
                }
            }
            else {
                clearSelection();
            }
            return true;
        }
        return false;
    }


    bool removeSelectedRow() {
        return removeRow( selectedRow() );
    }


    bool nameIsPresent( const QString &name ) const {
        return mModel.nameIsPresent( name );
    }


    bool nameIsSelected( const QString &name ) const {
        T *parameter { getSelectedRow() };
        if ( parameter ) {
            return parameter->getName().toLower() == name.toLower();
        }
        return false;
    }


    void clear() {
        mModel.clear();
        clearSelection();
    }


    void clearSelection() {
        mSelection.clearSelection();
        mSelection.clearCurrentIndex();
    }


    void selectRow( int row ) {
        // Note that the current index is also moved to the row
        int column { 0 };
        QModelIndex current = mSelection.currentIndex();
        if ( current.isValid() ) {
            column = current.column();
        }
        QModelIndex index { mModel.index( row, column ) };
        if ( index.isValid() ) {
            mSelection.setCurrentIndex(
                index,
                LockableItemSelectionModel::Clear |
                LockableItemSelectionModel::Rows
            );
        }
    }


    int selectedRow() const {
        // The row of the current index is used because the selection is
        // cleared in the QML TableView when the current index changes.
        int row { -1 };
        QModelIndex index = mSelection.currentIndex();
        if ( index.isValid() ) {
            row = index.row();
        }
        return row;
    }


    void setSelectionLocked( bool locked ) {
        mSelection.setSelectionLocked( locked );
    }


    void recordWasUpdated( T *record ) {
        // Used when a record is updated outside of the Model interface (i.e.
        // in-place)
        mModel.recordWasUpdated( record );
    }


private:
    Model<T> mModel;
    LockableItemSelectionModel mSelection;
};

#endif // MODELCONTROL_HPP
