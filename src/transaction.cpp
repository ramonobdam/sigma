#include "transaction.h"


Transaction::Transaction() : mDiffs {}, mLabel {} {}


QString Transaction::getLabel() const {
    return mLabel;
}


const QList<JsonDiff> & Transaction::getDiffs() const {
    return mDiffs;
}


QList<JsonDiff> & Transaction::getDiffs() {
    return mDiffs;
}


bool Transaction::isEmpty() const {
    return mDiffs.isEmpty();
}


void Transaction::addDiff( const JsonDiff &diff ) {
    mDiffs.append( diff );
}


void Transaction::setLabel( const QString &label ) {
    mLabel = label;
}
