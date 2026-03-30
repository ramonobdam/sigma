// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef INPUTPARAMETER_H
#define INPUTPARAMETER_H

#include "correlation.h"
#include "distribution.h"
#include "third_party/exprtk/exprtk.hpp"
#include "modelcontrol.hpp"
#include "parameter.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
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
    ~InputParameter();

    InputParameter& operator= ( const InputParameter &ip );
    bool operator== ( const InputParameter &ip ) const;
    bool operator!= ( const InputParameter &ip ) const;

    Distribution::Type getDistribution() const;
    InputParameter *addToModel();
    InputParameter *updateSelectedModelRow();
    QJsonObject toJson() const;
    QString getDOFAsString() const;
    QString toString() const;
    QVariant get( const int &column, const bool &csvMode = false ) const;
    Distribution::InvCDF getInvCDF() const;
    double *getSymbolPtr() const;
    void resetSymbolValue();
    void set( const int &column, const QVariant &value );
    void setDistribution( const Distribution::Type &distribution );
    void setSymbolPtr( double *symbolPtr );
    void setSymbolValue( const double &value );

    Q_INVOKABLE QString getDistributionAsString() const;
    Q_INVOKABLE bool getDOFInfinite() const;
    Q_INVOKABLE double getStdUncertainty() const;
    Q_INVOKABLE int getDOF() const;
    Q_INVOKABLE void setDOF( const int &DOF );
    Q_INVOKABLE void setDOFInfinite( const bool &DOFInfinite );
    Q_INVOKABLE void setDistribution( const QString &distributionString );
    Q_INVOKABLE void setStdUncertainty( const double &stdUncertainty );
    Q_INVOKABLE void setToSelected();

    static InputParameter fromJson(
        const QJsonObject &json,
        const bool &addToModel = true,
        QObject *parent = nullptr
    );
    static InputParameter *getInputParameter( const QString &name );
    static InputParameter *removeSelectedModelRow();
    static ModelControl<InputParameter *> *getInputModel();
    static QJsonArray parametersToJson();
    static QString parametersToString();

    static QList<int> columnWidths;
    static QString defaultName;
    static QString infiniteString;
    static QStringList headerLabels;
    static bool inputParameterIsConstant( const QString &name );
    static bool validName(
        const QString &name,
        const bool &checkCurrentSelection = false
    );
    static symbol_table_t symbolTable;
    static void addConstantsToSymbolTable();
    static void clearSymbolTable();
    static void parametersFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );

private:
    bool addToSymbolTable();

    static bool removeSymbol( const QString &name );
    static bool symbolExists( const QString &name );
    static bool validSymbol( const QString &name );

    Distribution::Type mDistribution;
    bool mDOFInfinite;
    double *mSymbolPtr;
    double mStdUncertainty;
    int mDOF;

    static ModelControl<InputParameter *> mInputModel;
    static QString mDOFInfiniteString;
    static QString mDOFString;
    static QString mDistributionString;
    static QString mInputParametersHeaderString;
    static QString mStdUncertaintyString;
};

#endif // INPUTPARAMETER_H
