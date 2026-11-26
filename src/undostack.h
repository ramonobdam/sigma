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
// The stack also keeps track of the clean state: the cursor position at which
// the data equals the last saved (or loaded/new) project. The project has
// unsaved changes when the cursor differs from the clean index. When the clean
// state is part of truncated redo history, it can no longer be reached and the
// clean index becomes -1 (until setClean() or clear() is called).
class UndoStack : public QObject {
    Q_OBJECT

public:
    QString getLabelAt( int index ) const;
    bool canRedo() const;
    bool canUndo() const;
    bool isClean() const;
    int getCleanIndex() const;
    int getCursor() const;
    int getStackSize() const;
    void clear();
    void pushTransaction( const Transaction &transaction );
    void redo();
    void restoreState( int index );
    void setClean();
    void undo();

    static UndoStack &instance();

signals:
    void canUndoChanged();
    void canRedoChanged();
    // Emitted when isClean() changes, i.e. when the current state of the data
    // becomes equal to, or different from, the state marked with setClean()
    void cleanChanged();
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

    void emitCleanChangedIfNeeded( bool wasClean );

    QList<Transaction> mStack;
    int mCursor;
    // Cursor position of the clean (saved) state. -1 when the clean state can
    // no longer be reached by undo/redo (it was in truncated redo history).
    int mCleanIndex;
};

#endif // UNDOSTACK_H
