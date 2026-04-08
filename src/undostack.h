// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "jsondiff.h"
#include "transaction.h"
#include <QList>
#include <QMap>
#include <QObject>

// Interface for a model that supports undoable transactions
class UndoableModel {
public:
    virtual ~UndoableModel() = default;

    virtual QJsonObject currentJson( const QString &objectId ) const = 0;
    virtual void applyDiff( const JsonDiff &diff ) = 0;

};

// UndoStack singleton that stores transactions and can undo/redo them.
// Undo/redo is available for DataType objects in UndoableModels registered via
// registerModel(). The snapShot() method is used in Model class methods that
// change data.
class UndoStack : public QObject {
    Q_OBJECT

public:
    bool canRedo() const;
    bool canUndo() const;
    void clear();
    bool isTransactionActive() const;
    void abortTransaction();
    void beginTransaction( const QString &label = {} );
    void commitTransaction();
    void redo();
    void registerModel( DataType type, UndoableModel *model );
    void snapshot(
        const QString &objectId,
        DataType type,
        const QJsonObject &before
    );
    void undo();

signals:
    void canUndoChanged();
    void canRedoChanged();

private:
    UndoStack() = default;

    enum class Direction { Undo, Redo };

    void applyDiff( const JsonDiff &diff, const QJsonObject &state );
    void applyTransaction(
        const Transaction &transaction,
        Direction direction
    );

    inline static QList<Transaction> sStack {};
    inline static QMap<DataType, UndoableModel*> sRegistry {};
    inline static Transaction sActiveTransaction {};
    inline static int sCursor { 0 };
};

#endif // UNDOSTACK_H
