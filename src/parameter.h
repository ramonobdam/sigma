// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef PARAMETER_H
#define PARAMETER_H

#include "data.h"
#include "QLatin1StringView"
#include <QObject>
#include <QString>
#include <string>

// Abstract Parameter class with the attributes name, unit, nominal value,
// locked and valid. Derives from QObject to enable the 'signals and slots'
// mechanism and Q_INVOKABLE methods that can be invoked from QML.
class Parameter : public QObject, public Data {
    Q_OBJECT

public:
    Parameter( QObject *parent = nullptr );
    Parameter(
        QObject *parent,
        const QString &name,
        const QString &unit,
        double nominalValue,
        bool locked,
        bool valid
    );
    Parameter( const Parameter &par );
    virtual ~Parameter() = default;

    Parameter & operator= (const Parameter &par );

    virtual bool getLocked() const;
    virtual std::wstring getNameStdWString() const;
    virtual void setValid( bool valid );

    Q_INVOKABLE virtual QString getName( bool csvMode = false ) const;
    Q_INVOKABLE virtual QString getUnit( bool csvMode = false ) const;
    Q_INVOKABLE virtual bool getValid() const;
    Q_INVOKABLE virtual double getNominalValue() const;
    Q_INVOKABLE virtual void setLocked( bool locked );
    Q_INVOKABLE virtual void setName( const QString &name );
    Q_INVOKABLE virtual void setNominalValue( double nominalValue );
    Q_INVOKABLE virtual void setUnit( const QString &unit );

protected:
    QString mName;
    QString mUnit;
    bool mLocked;
    bool mValid;
    double mNominalValue;

    static constexpr QLatin1StringView sNameString { "name" };
    static constexpr QLatin1StringView sNominalValueString { "nominalValue" };
    static constexpr QLatin1StringView sUnitString { "unit" };
    static constexpr QLatin1StringView sValidString { "valid" };
};

#endif // PARAMETER_H
