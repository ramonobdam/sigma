#ifndef DIFFUTIL_H
#define DIFFUTIL_H

#include <QJsonObject>

// Utility class that determines and applies diffs between QJsonObjects.
// Inspired by the DiffUtil approach described in:
// "Creating a Tools Pipeline for Horizon Zero Dawn" (GDC 2017)
// Sumaili & Van Der Steen, Guerrilla Games
class DiffUtil {
public:
    static QJsonObject apply(
        const QJsonObject &base,
        const QJsonObject &patch
    );
    static QJsonObject diff(
        const QJsonObject &before,
        const QJsonObject &after
    );
    static bool isUnchanged(
        const QJsonObject &before,
        const QJsonObject &after
    );
};

#endif // DIFFUTIL_H
