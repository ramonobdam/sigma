// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef PARAMETER_H
#define PARAMETER_H

#include "settings.h"
#include "utils.h"
#include <QObject>
#include <QString>
#include <string>

// Base Parameter class with the properties name, unit, nominal value, locked
// and valid. Derives from QObject to enable the 'signals and slots' mechanism
// and Q_INVOKABLE methods that can be invoked from QML.
class Parameter : public QObject, protected Settings, protected Utils {
    Q_OBJECT

public:
    Parameter( QObject *parent = nullptr );
    Parameter(
        QObject *parent,
        const QString &name,
        const QString &unit,
        const double &nominalValue,
        const bool &locked,
        const bool &valid
    );
    ~Parameter();

    bool virtual getLocked() const;
    std::wstring virtual getNameStdWString() const;
    void virtual setValid( const bool &valid );

    Q_INVOKABLE virtual QString getName( const bool csvMode = false ) const;
    Q_INVOKABLE virtual QString getUnit( const bool csvMode = false ) const;
    Q_INVOKABLE virtual bool getValid() const;
    Q_INVOKABLE virtual double getNominalValue() const;
    Q_INVOKABLE virtual void setLocked( const bool &locked );
    Q_INVOKABLE virtual void setName( const QString &name );
    Q_INVOKABLE virtual void setNominalValue( const double &nominalValue );
    Q_INVOKABLE virtual void setUnit( const QString &unit );

protected:
    QString mName;
    QString mUnit;
    bool mLocked;
    bool mValid;
    double mNominalValue;

    static QString mNameString;
    static QString mNominalValueString;
    static QString mUnitString;
    static QString mValidString;
};

#endif // PARAMETER_H
