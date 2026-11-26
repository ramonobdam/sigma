// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "diffutil.h"
#include "undostack.h"


QString UndoStack::getLabelAt( int index ) const {
    if ( index >= 0 && index < mStack.size() ) {
        return mStack[ index ].getLabel();
    }
    return "";
}


bool UndoStack::canRedo() const {
    return mCursor < mStack.size();
}


bool UndoStack::canUndo() const {
    return mCursor > 0;
}


bool UndoStack::isClean() const {
    return mCursor == mCleanIndex;
}


int UndoStack::getCleanIndex() const {
    return mCleanIndex;
}


int UndoStack::getCursor() const {
    return mCursor;
}


int UndoStack::getStackSize() const {
    return mStack.size();
}


void UndoStack::clear() {
    const bool wasClean { isClean() };
    mStack.clear();
    mCursor = 0;
    mCleanIndex = 0;    // An empty stack represents a clean (new) state
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emit stackSizeChanged();
    emitCleanChangedIfNeeded( wasClean );
}


void UndoStack::pushTransaction( const Transaction &transaction ) {
    const bool wasClean { isClean() };

    // Truncate redo history. When the clean state is part of the truncated
    // history, it can't be reached anymore.
    if ( mCleanIndex > mCursor ) {
        mCleanIndex = -1;
    }
    mStack.resize( mCursor );

    mStack.append( transaction );
    ++mCursor;
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emit stackSizeChanged();
    emitCleanChangedIfNeeded( wasClean );
}


void UndoStack::redo() {
    if ( !canRedo() ) {
        return;
    }

    const bool wasClean { isClean() };
    applyTransaction( mStack[ mCursor ], Direction::Redo );
    ++mCursor;
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emitCleanChangedIfNeeded( wasClean );
}


void UndoStack::restoreState( int index ) {
    // Apply transactions to restore the state at index
    if ( index < 0 || index > mStack.size() ) return;
    if ( index == mCursor ) return;

    const bool wasClean { isClean() };

    // Apply transactions silently — block signals
    blockSignals( true );

    if ( index < mCursor ) {
        // Undo down to index
        while ( mCursor > index ) {
            undo();
        }
    } else {
        // Redo up to index
        while ( mCursor < index ) {
            redo();
        }
    }

    // Emit all signals once after all transactions are applied. As for undo()
    // and redo(), transactionApplied() is emitted before cursorChanged(), so
    // the data is fully restored when the cursor change is signalled.
    blockSignals( false );
    emit transactionApplied();
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emitCleanChangedIfNeeded( wasClean );
}


void UndoStack::setClean() {
    // Mark the current state as clean (e.g. after saving the project)
    const bool wasClean { isClean() };
    mCleanIndex = mCursor;
    emitCleanChangedIfNeeded( wasClean );
}


void UndoStack::undo() {
    if ( !canUndo() ) {
        return;
    }

    const bool wasClean { isClean() };
    --mCursor;
    applyTransaction( mStack[ mCursor ], Direction::Undo );
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emitCleanChangedIfNeeded( wasClean );
}


UndoStack & UndoStack::instance() {
    static UndoStack stack;
    return stack;
}


UndoStack::UndoStack()
    :   mStack {},
        mCursor { 0 },
        mCleanIndex { 0 }
{}


void UndoStack::emitCleanChangedIfNeeded( bool wasClean ) {
    if ( isClean() != wasClean ) {
        emit cleanChanged();
    }
}


void UndoStack::applyTransaction(
    const Transaction &transaction,
    Direction direction
) {
    const QList<JsonDiff> &diffs { transaction.getDiffs() };
    if ( direction == Direction::Undo ) {
        // Apply diffs in reverse order
        for ( auto it { diffs.rbegin() }; it != diffs.rend(); ++it ) {
            DiffUtil::applyDiff( JsonDiff {
                it->objectId,
                it->objectType,
                it->row,
                it->after,      // swap before/after for undo
                it->before
            } );
        }
    } else {
        for ( const JsonDiff &diff : diffs ) {
            DiffUtil::applyDiff( diff );
        }
    }
    emit transactionApplied();
}
