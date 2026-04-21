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
#include <QUuid>
#include <QVariant>
#include <QtMinMax>

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


    Model<T> *itemModel() {
        return &mModel;
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


    T *appendRow( const T &record ) {
        return insertRow( rowCount(), record );
    }


    T *insertRow( int row, const T &record ) {
        const int boundedRow { qBound( 0, row, rowCount() ) };
        mModel.insertRow( boundedRow, record );
        selectRow( boundedRow );   // The new row becomes the selected row
        return getByRow( boundedRow );
    }


    T* updateSelectedRow( const T &record ) {
        const int row { selectedRow() };
        if ( row >= 0 ) {
            return updateByRow( row, record );
        }
        return nullptr;
    }


    void emitIdChanged( const QUuid &id ) {
        mModel.emitIdChanged( id );
    }


    void emitRowChanged( int row ) {
        mModel.emitRowChanged( row );
    }


    void emitAllDataChanged() {
        mModel.emitAllDataChanged();
    }


    T* updateByRow( int row, const T &record ) {
        return mModel.updateByRow( row, record );
    }


    T* updateById( const QUuid &id, const T &record ) {
        return mModel.updateById( id, record );
    }


    T *getByRow( int row ) const {
        return mModel.getByRow( row );
    }


    T *getById( const QUuid &id ) const {
        return mModel.getById( id );
    }


    T *getByName( const QString &name ) const {
        return mModel.getByName( name );
    }


    const QList<T *> &getAllRows() const {
        return mModel.getAllRows();
    }


    int getRowIndex( const QUuid &id ) const {
        return mModel.getRowIndex( id );
    }


    T *getSelected() const {
        const int row = { selectedRow() };
        if ( row >= 0 ) {
            return getByRow( row );
        }
        return nullptr;
    }


    QUuid getSelectedId() const {
        T *record { getSelected() };
        if ( record ) {
            return record->getId();
        }
        return QUuid();
    }


    QVariant getSelectedItem( int column = 0 ) const {
        T *record { getSelected() };
        if ( record ) {
            return record->get( column );
        }
        return QVariant();
    }


    bool removeRow( int row ) {
        if ( row >= 0 && row < rowCount() ) {
            if ( mModel.removeRow( row ) ) {
                // Update the selected row when the table is not empty, else
                // clear the selection
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
        }
        return false;
    }


    bool removeById( const QUuid &id ) {
        // Removed via removeRow() to also update the selection
        return removeRow( getRowIndex( id ) );
    }


    bool removeSelectedRow() {
        return removeRow( selectedRow() );
    }


    bool nameIsPresent( const QString &name ) const {
        return mModel.nameIsPresent( name );
    }


    bool nameIsSelected( const QString &name ) const {
        T *record { getSelected() };
        if ( record ) {
            return record->getName().toLower() == name.toLower();
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
        QModelIndex index { mSelection.currentIndex() };
        if ( index.isValid() ) {
            return index.row();
        }
        return -1;
    }


    void setSelectionLocked( bool locked ) {
        mSelection.setSelectionLocked( locked );
    }


private:
    Model<T> mModel;
    LockableItemSelectionModel mSelection;
};

#endif // MODELCONTROL_HPP
