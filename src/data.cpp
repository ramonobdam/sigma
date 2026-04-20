// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#include "data.h"


Data::Data() : mId { QUuid::createUuid() } {}


Data::Data( const Data &dat ) : mId { dat.getId() } {}


Data & Data::operator= ( const Data &dat ) {
    if ( this != &dat ) {
        mId = dat.getId();
    }
    return *this;
}


QUuid Data::getId() const {
    return mId;
}


void Data::resetId() {
    mId = QUuid::createUuid();
}


void Data::setId( const QUuid &id ) {
    mId = id;
}
