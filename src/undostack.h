// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "diffutil.h"
#include "jsondiff.h"
#include "transaction.h"
#include <QList>
#include <QObject>
#include <QString>

// UndoStack singleton that stores transactions and can undo/redo them.
// Undo/redo applies transactions by dispatching each JsonDiff to
// DiffUtil::applyDiff().
// The cursor points to the next redo position:
//   cursor = 0          -> nothing to undo
//   cursor = stack size -> nothing to redo
// New transactions truncate any redo history above the cursor.
class UndoStack : public QObject {
    Q_OBJECT

public:
    QString getLabelAt( int index ) const;
    bool canRedo() const;
    bool canUndo() const;
    int getCursor() const;
    int getStackSize() const;
    void clear();
    void pushTransaction( const Transaction &transaction );
    void redo();
    void restoreState( int index );
    void undo();

    static UndoStack &instance();

signals:
    void canUndoChanged();
    void canRedoChanged();
    void cursorChanged();
    void stackSizeChanged();
    void transactionApplied();

private:
    UndoStack();

    enum class Direction { Undo, Redo };

    void applyTransaction(
        const Transaction &transaction,
        Direction direction
    );

    QList<Transaction> mStack;
    int mCursor;
};

#endif // UNDOSTACK_H
