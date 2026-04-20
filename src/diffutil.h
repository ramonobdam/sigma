// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef DIFFUTIL_H
#define DIFFUTIL_H

#include "data.h"
#include "datatype.h"
#include "jsondiff.h"
#include "transaction.h"
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QUuid>
#include <functional>

// Class that records object state changes and produces transactions for the
// UndoStack. Only changed keys are stored in each JsonDiff. Static registries
// map DataType to per-type functions.

// Inspired by the DiffUtil approach described in:
// "Creating a Tools Pipeline for Horizon Zero Dawn" (GDC 2017)
// Sumaili & Van Der Steen, Guerrilla Games

// Workflow when mutating objects:
//   DiffUtil diffUtil {};
//   diffUtil.takeSnapshot( *object );              // capture before state
//   diffUtil.takeSnapshotOfNewObject( *object );   // register new object
//   ... make changes to objects...
//   diffUtil.commitChanges( "action label" );      // compute diffs and
                                                    // push to UndoStack

class DiffUtil {
public:
    DiffUtil();

    void abortChanges();
    void commitChanges( const QString &label = "" );
    void takeSnapshot( const Data *data );
    void takeSnapshotOfNewObject( const Data *data );

    static QJsonObject currentJson( const QUuid &id, DataType type );
    static void applyDiff( const JsonDiff &diff );
    static void registerApplyDiff(
        DataType type,
        std::function<void( const JsonDiff & )> function
    );
    static void registerCurrentJson(
        DataType type,
        std::function<QJsonObject( const QUuid & )> function
    );
    static void registerGetRowIndex(
        DataType type,
        std::function<int( const QUuid & )> function
    );

private:
    QList<JsonDiff> mSnapshots;

    static QJsonObject applyPatch(
        const QJsonObject &base,
        const QJsonObject &patch
    );
    static QJsonObject computeDiff(
        const QJsonObject &before,
        const QJsonObject &after
    );
    static bool isUnchanged(
        const QJsonObject &before,
        const QJsonObject &after
    );

    inline static QMap<DataType, std::function<QJsonObject( const QUuid & )> >
        sCurrentJsonFunctions {};
    inline static QMap<DataType, std::function<int( const QUuid & )> >
        sGetRowIndexFunctions {};
    inline static QMap<DataType, std::function<void( const JsonDiff & )> >
        sApplyDiffFunctions {};
};

#endif // DIFFUTIL_H
