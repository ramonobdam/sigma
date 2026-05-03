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


int UndoStack::getCursor() const {
    return mCursor;
}


int UndoStack::getStackSize() const {
    return mStack.size();
}


void UndoStack::clear() {
    mStack.clear();
    mCursor = 0;
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emit stackSizeChanged();
}


void UndoStack::pushTransaction( const Transaction &transaction ) {
    // Truncate redo history
    mStack.resize( mCursor );

    mStack.append( transaction );
    ++mCursor;
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emit stackSizeChanged();
}


void UndoStack::redo() {
    if ( !canRedo() ) {
        return;
    }

    applyTransaction( mStack[ mCursor ], Direction::Redo );
    ++mCursor;
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
}


void UndoStack::restoreState( int index ) {
    // Apply transactions to restore the state at index
    if ( index < 0 || index > mStack.size() ) return;
    if ( index == mCursor ) return;

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

    // Emit all signals once after all transactions are applied
    blockSignals( false );
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
    emit transactionApplied();
}


void UndoStack::undo() {
    if ( !canUndo() ) {
        return;
    }

    --mCursor;
    applyTransaction( mStack[ mCursor ], Direction::Undo );
    emit canUndoChanged();
    emit canRedoChanged();
    emit cursorChanged();
}


UndoStack & UndoStack::instance() {
    static UndoStack stack;
    return stack;
}


UndoStack::UndoStack()
    :   mStack {},
        mCursor { 0 }
{}


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
