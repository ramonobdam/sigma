#include "diffutil.h"
#include <QStringList>


QJsonObject apply( const QJsonObject &base, const QJsonObject &patch ) {
    // Applies a patch produced by diff() onto a base QJsonObject. Null values
    // in the patch signal key deletion. Returns the patched QJsonObject.

    QJsonObject result { base };

    const QStringList &keys { patch.keys() };
    for ( const QString &key : keys ) {
        if ( patch[ key ].isNull() ) {
            result.remove( key );   // deletion
        } else {
            result.insert( key, patch[ key ] );  // add or update
        }
    }

    return result;
}


QJsonObject DiffUtil::diff(
    const QJsonObject &before,
    const QJsonObject &after
) {
    // Returns a QJsonObject containing only the keys whose values differ
    // between before and after. Keys present in before but absent in after are
    // represented as QJsonValue::Null to signal deletion.

    QJsonObject result {};

    // Keys in after — changed or added
    const QStringList &keysAfter { after.keys() };
    for ( const QString &key : keysAfter ) {
        if ( before[ key ] != after[ key ] ) {
            result.insert( key, after[ key ] );
        }
    }

    // Keys in before but missing in after — deleted
    const QStringList &keysBefore { after.keys() };
    for ( const QString &key : keysBefore ) {
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
