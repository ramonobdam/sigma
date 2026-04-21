// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "diffutil.h"
#include "undostack.h"
#include <QStringList>


DiffUtil::DiffUtil() : mSnapshots {}
{}


void DiffUtil::abortChanges() {
    mSnapshots.clear();
}


void DiffUtil::commitChanges( const QString &label ) {
    // Analyze changes, produce a transaction and push to UndoStack
    if ( mSnapshots.isEmpty() ) return;

    Transaction transaction {};
    transaction.setLabel( label );

    for ( JsonDiff &diff : mSnapshots ) {
        // Obtain the current state (after changes)
        QJsonObject after { currentJson( diff.objectId, diff.objectType ) };

        // Store the current row when the object is not deleted (after != empty)
        if ( !after.isEmpty() ) {
            const auto rowFunction {
                sGetRowIndexFunctions.value( diff.objectType )
            };
            diff.row = rowFunction ? rowFunction( diff.objectId ) : -1;
        }

        // Skip unchanged objects
        if ( isUnchanged( diff.before, after ) ) continue;

        // Preserve empty sentinels for add/delete — don't diff them
        if ( diff.before.isEmpty() || after.isEmpty() ) {
            diff.after = after;
            transaction.addDiff( diff );
            continue;
        }

        // Store only changed keys for updated object
        diff.after  = computeDiff( diff.before, after );
        diff.before = computeDiff( after, diff.before );
        transaction.addDiff( diff );
    }

    mSnapshots.clear();

    if ( transaction.isEmpty() ) return;

    UndoStack::instance().pushTransaction( transaction );
}


void DiffUtil::takeSnapshot( const Data *data ) {
    // Take a snapshot of a Data object's current state before changes
    if ( !data ) return;

    // Only snapshot the first change to an object within a transaction
    const QList<JsonDiff> &diffs { mSnapshots };
    for ( const JsonDiff &diff : diffs ) {
        if ( diff.objectId == data->getId() ) return;
    }

    const auto rowFunction { sGetRowIndexFunctions.value( data->dataType() ) };

    mSnapshots.append( JsonDiff {
        data->getId(),
        data->dataType(),
        rowFunction ? rowFunction( data->getId() ) : -1,
        data->toJson(),  // before state
        {}               // after filled in commitChanges()
    } );
}


void DiffUtil::takeSnapshotOfNewObject( const Data *data ) {
    // Register a new object (empty before state)
    if ( !data ) return;

    // Only snapshot the first change to an object within a transaction
    const QList<JsonDiff> &diffs { mSnapshots };
    for ( const JsonDiff &diff : diffs ) {
        if ( diff.objectId == data->getId() ) return;
    }

    mSnapshots.append( JsonDiff {
        data->getId(),
        data->dataType(),
        -1,          // row set in commitChanges()
        {},          // empty before = new object
        {}           // after filled in commitChanges()
    } );
}


QJsonObject DiffUtil::currentJson( const QUuid &id, DataType type ) {
    const auto function { sCurrentJsonFunctions.value( type ) };
    return function ? function( id ): QJsonObject {};
}


void DiffUtil::applyDiff( const JsonDiff &diff ) {
    const auto function { sApplyDiffFunctions.value( diff.objectType ) };
    if ( function ) function( diff );
}


void DiffUtil::registerApplyDiff(
    DataType type,
    std::function<void (const JsonDiff &)> function
) {
    sApplyDiffFunctions.insert( type, function );
}


void DiffUtil::registerCurrentJson(
    DataType type,
    std::function<QJsonObject (const QUuid &)> function
) {
    sCurrentJsonFunctions.insert( type, function );
}


void DiffUtil::registerGetRowIndex(
    DataType type,
    std::function<int ( const QUuid & )> function
) {
    sGetRowIndexFunctions.insert( type, function );
}


QJsonObject DiffUtil::applyPatch(
    const QJsonObject &base,
    const QJsonObject &patch
) {
    // Applies a patch produced by computeDiff() onto a base QJsonObject. Null
    // values in the patch signal key deletion.
    QJsonObject result { base };
    const QStringList &keys { patch.keys() };
    for ( const QString &key : keys ) {
        if ( patch[ key ].isNull() ) {
            result.remove( key );
        } else {
            result.insert( key, patch[ key ] );
        }
    }
    return result;
}


QJsonObject DiffUtil::computeDiff(
    const QJsonObject &before,
    const QJsonObject &after
) {
    // Returns a QJsonObject containing only keys whose values differ between
    // before and after. Keys present in before but absent in after are
    // represented as QJsonValue::Null to signal deletion.
    QJsonObject result {};

    // Keys in after — added or changed
    const QStringList &afterKeys { after.keys() };
    for ( const QString &key : afterKeys ) {
        if ( before[ key ] != after[ key ] ) {
            result.insert( key, after[ key ] );
        }
    }

    // Keys in before but missing in after — deleted
    const QStringList &beforeKeys { before.keys() };
    for ( const QString &key : beforeKeys ) {
        if ( !after.contains( key ) ) {
            result.insert( key, QJsonValue::Null );
        }
    }

    return result;
}


bool DiffUtil::isUnchanged(
    const QJsonObject &before,
    const QJsonObject &after
) {
    // Returns true if before and after are identical — no diff needed
    return before == after;
}
