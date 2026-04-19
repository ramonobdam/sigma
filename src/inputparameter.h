// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef INPUTPARAMETER_H
#define INPUTPARAMETER_H

#include "datatype.h"
#include "distribution.h"
#include "third_party/exprtk/exprtk.hpp"
#include "jsondiff.h"
#include "modelcontrol.hpp"
#include "parameter.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QLatin1StringView>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariant>
#include <QtQmlIntegration/qqmlintegration.h>

typedef exprtk::symbol_table<double> symbol_table_t;

// Class that stores the data of an input parameter and defines static objects
// for the symbol table of ExprTK and the ModelControl that stores the input
// parameters
class InputParameter: public Parameter {
    Q_OBJECT
    QML_ELEMENT

public:
    InputParameter( QObject *parent = nullptr );
    InputParameter( const InputParameter &ip );

    InputParameter & operator= ( const InputParameter &ip );
    bool operator== ( const InputParameter &ip ) const;
    bool operator!= ( const InputParameter &ip ) const;

    DataType dataType() const override;
    Distribution::Type getDistribution() const;
    InputParameter *appendToModel();
    QJsonObject toJson() const override;
    QString getDOFAsString() const;
    QString toCSVString() const;
    QVariant get( int column, bool csvMode = false ) const override;
    QVariant headerData( int column ) const override;
    Distribution::InvCDF getInvCDF() const;
    double getSymbolValue() const;
    int columnCount() const override;
    void resetSymbolValue();
    void set( int column, const QVariant &value ) override;
    void setDistribution( Distribution::Type distribution );
    void setSymbolValue( double value );
    void updateFromJson( const QJsonObject &json ) override;

    Q_INVOKABLE QString getDistributionAsString() const;
    Q_INVOKABLE bool getDOFInfinite() const;
    Q_INVOKABLE double getStdUncertainty() const;
    Q_INVOKABLE int getDOF() const;
    Q_INVOKABLE void setDOF( int DOF );
    Q_INVOKABLE void setDOFInfinite( bool DOFInfinite );
    Q_INVOKABLE void setDistribution( const QString &distributionString );
    Q_INVOKABLE void setStdUncertainty( double stdUncertainty );
    Q_INVOKABLE void setToSelected();

    static InputParameter fromJson(
        const QJsonObject &json,
        bool appendToModel = true,
        QObject *parent = nullptr
    );
    static InputParameter *getByName( const QString &name );
    static InputParameter *getById( const QUuid &id );
    static InputParameter *getSelected();
    static ModelControl<InputParameter *> *getInputModel();
    static QJsonArray parametersToJson();
    static QJsonObject currentJson( const QUuid &id );
    static const QList<InputParameter *> &getAll();
    static QString parametersToCSVString();
    static QUuid getSelectedId();
    static bool inputParameterIsConstant( const QString &name );
    static bool remove( const QUuid &id );
    static bool update( const QUuid &id, InputParameter *parameter );
    static bool validName(
        const QString &name,
        bool checkCurrentSelection = false
    );
    static int getRowIndex( const QUuid &id );
    static void addConstantsToSymbolTable();
    static void applyDiff( const JsonDiff &diff );
    static void clearModel();
    static void clearSymbolTable();
    static void onDisplayPrecisionChanged();
    static void parametersFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );
    static void setSelectionLocked( bool locked );

    static constexpr QLatin1StringView defaultName { "X" };
    inline static const QList<int> columnWidths { 90, 55, 105, 148, 92, 144 };
    inline static const QStringList headerLabels {
        "Input name",
        "Unit",
        "Input estimate",
        "Standard uncertainty",
        "Distribution",
        "Degrees of freedom"
    };
    inline static symbol_table_t symbolTable {};

private:
    InputParameter *insertIntoModel( int row );
    bool addToSymbolTable();

    static bool removeSymbol( const QString &name );
    static bool symbolExists( const QString &name );
    static bool validSymbol( const QString &name );

    Distribution::Type mDistribution;
    bool mDOFInfinite;
    double mSymbolValue;
    double mStdUncertainty;
    int mDOF;

    static ModelControl<InputParameter *> mInputModel;
    static constexpr QLatin1StringView sDOFInfiniteString { "DOFInfinite" };
    static constexpr QLatin1StringView sDOFString { "DOF" };
    static constexpr QLatin1StringView sDistributionString { "distribution" };
    static constexpr QLatin1StringView sInputParametersHeaderString {
        "Input parameters:"
    };
    static constexpr QLatin1StringView sStdUncertaintyString {
        "stdUncertainty"
    };
};

#endif // INPUTPARAMETER_H
