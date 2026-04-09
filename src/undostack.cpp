#include "diffutil.h"
#include "undostack.h"


bool UndoStack::canRedo() const {
    return sCursor < sStack.size();
}


bool UndoStack::canUndo() const {
    return sCursor > 0;
}


void UndoStack::clear() {
    abortTransaction();
    sStack.clear();
    sCursor = 0;
    emit canUndoChanged();
    emit canRedoChanged();
}


bool UndoStack::isTransactionActive() const {
    return !sActiveTransaction.isEmpty();
}


void UndoStack::abortTransaction() {
    sActiveTransaction = {};
}


void UndoStack::beginTransaction( const QString &label ) {
    sActiveTransaction = Transaction {};
    sActiveTransaction.setLabel( label );
}


void UndoStack::commitTransaction() {
    if ( sActiveTransaction.isEmpty() ) {
        return;
    }

    // Capture after state for all snapshots
    for ( JsonDiff &diff : sActiveTransaction.getDiffs() ) {
        UndoableModel *model = sRegistry.value( diff.objectType );
        if ( model ) {
            QJsonObject after { model->currentJson( diff.objectId ) };
            if ( DiffUtil::isUnchanged( diff.before, after ) ) {
                continue;
            }

            // Keep only the changed keys
            diff.before = DiffUtil::diff( after, diff.before );
            diff.after  = DiffUtil::diff( diff.before, after );
        }
    }

    // Truncate redo history
    sStack.resize( sCursor );
    sStack.append( sActiveTransaction );
    ++sCursor;

    sActiveTransaction = {};
}


void UndoStack::redo() {
    if ( !canRedo() ) {
        return;
    }

    applyTransaction( sStack[ sCursor ], Direction::Redo );
    ++sCursor;
}


void UndoStack::registerModel( DataType type, UndoableModel *model ) {
    sRegistry.insert( type, model );
}


void UndoStack::snapshot(
    QUuid objectId,
    DataType type,
    const QJsonObject &before
) {
    // Called by Model before mutating a record
    if ( sApplying || !isTransactionActive() ) return;

    // Only snapshot before the first change to an object within a transaction
    const QList<JsonDiff> &diffs { sActiveTransaction.getDiffs() };
    for ( const JsonDiff &diff : diffs ) {
        if ( diff.objectId == objectId && diff.objectType == type ) {
            return;
        }
    }

    sActiveTransaction.addDiff( JsonDiff { objectId, type, before, {} } );
}


void UndoStack::undo() {
    if ( !canUndo() ) {
        return;
    }

    --sCursor;
    applyTransaction( sStack[ sCursor ], Direction::Undo );

    emit canUndoChanged();
    emit canRedoChanged();
}


UndoStack & UndoStack::instance() {
    static UndoStack stack;
    return stack;
}


void UndoStack::applyDiff( const JsonDiff &diff, const QJsonObject &state ) {
    // Get the data model for the objectType
    UndoableModel *model = sRegistry.value( diff.objectType );

    if ( model ) {
        model->applyDiff(
            JsonDiff { diff.objectId, diff.objectType, diff.before, state }
        );
    }
}


void UndoStack::applyTransaction(
    const Transaction &transaction,
    Direction direction
) {
    sApplying = true;   // Guard against taking snapshots

    const QList<JsonDiff> &diffs = transaction.getDiffs();

    if ( direction == Direction::Undo ) {
        // Undo in reverse order
        for ( auto it = diffs.rbegin(); it != diffs.rend(); ++it ) {
            applyDiff( *it, it->before );
        }
    } else {
        // Redo
        for ( const JsonDiff &diff : diffs ) {
            applyDiff( diff, diff.after );
        }
    }

    sApplying = false;
    emit canUndoChanged();
    emit canRedoChanged();
}
