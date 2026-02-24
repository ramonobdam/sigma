// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef MODELCONTROL_H
#define MODELCONTROL_H

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
    typedef typename std::remove_pointer<pT>::type T;

public:
    ModelControl( QObject *parent = nullptr )
        :   mModel { Model<T>( this ) },
            mSelection { LockableItemSelectionModel { &mModel, this } }
    {
        setParent( parent );
    }


    ~ModelControl() {}


    QObject *itemModel() {
        return dynamic_cast<QObject *> ( &mModel );
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


    int appendRow( const T &parameter ) {
        mModel.appendRow( parameter );
        int row { rowCount() - 1 };
        selectRow( row );
        return row;
    }


    void updateSelectedRow( const T &parameter ) {
        int row { selectedRow() };
        if ( row >= 0 )
        {
            updateRow( row, parameter );
        }
    }


    void emitRowChanged( const int &row ) {
        mModel.emitRowChanged( row );
    }


    void emitAllDataChanged() {
        mModel.emitAllDataChanged();
    }


    void updateRow( const int &row, const T &parameter ) {
        mModel.updateRow( row, parameter );
    }


    T *getRow( const int &row ) const {
        return mModel.getRow( row );
    }


    QList<T *> getAllRows() const {
        return mModel.getAllRows();
    }


    T *getSelectedRow() const {
        int row = { selectedRow() };
        if ( row >= 0 ) {
            return getRow( row );
        }
        return nullptr;
    }


    QVariant getSelectedItem( const int &column = 0 ) const {
        T *parameter { getSelectedRow() };
        if ( parameter ) {
            return parameter->get( column );
        }
        return QVariant();
    }


    bool removeRow( const int &row ) {
        if ( row >= 0 && row < rowCount() ) {
            mModel.removeRow( row );
            return true;
        }
        return false;
    }


    bool removeSelectedRow() {
        int row { selectedRow() };
        if ( row >= 0 ) {
            mModel.removeRow( row );
            return true;
        }
        return false;
    }


    bool nameIsPresent( const QString &name ) const {
        return mModel.nameIsPresent( name );
    }


    bool nameIsCurrent( const QString &name ) const {
        T *parameter { getSelectedRow() };
        if ( parameter ) {
            return parameter->getName().toLower() == name.toLower();
        }
        return false;
    }


    void clear() {
        mModel.clear();
    }


    void clearSelection() {
        mSelection.clearSelection();
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


    void setSelectionLocked( const bool &locked ) {
        mSelection.setSelectionLocked(locked);
    }


private:
    Model<T> mModel;
    LockableItemSelectionModel mSelection;
};

#endif // MODELCONTROL_H
