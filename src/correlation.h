// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

#ifndef CORRELATION_H
#define CORRELATION_H

#include "data.h"
#include "datatype.h"
#include "inputparameter.h"
#include "modelcontrol.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QtQmlIntegration/qqmlintegration.h>

// Class that defines the correlation coefficient between 2 InputParameters.
class Correlation : public QObject, public Data {
    Q_OBJECT
    QML_ELEMENT

public:
    Correlation(
        QObject *parent = nullptr,
        InputParameter *inputParameterA = nullptr,
        InputParameter *inputParameterB = nullptr,
        const double &correlation = 0.
    );
    Correlation( const Correlation &cor );

    Correlation& operator= ( const Correlation &cor );
    bool operator== ( const Correlation &cor ) const;
    bool operator!= ( const Correlation &cor ) const;

    Correlation *addToModel();
    Correlation *updateSelectedModelRow();
    DataType dataType() const override;
    InputParameter *getInputParameterA() const;
    InputParameter *getInputParameterB() const;
    QJsonObject toJson() const override;
    QString getName( bool csvMode = false ) const override;
    QString toCSVString() const;
    QVariant get( int column, bool csvMode = false ) const override;
    QVariant headerData( int column ) const override;
    QUuid getInputParameterAId() const;
    QUuid getInputParameterBId() const;
    void updateFromJson( const QJsonObject &json ) override;

    Q_INVOKABLE QString getInputParameterNameA(
        const bool csvMode = false
    ) const;
    Q_INVOKABLE QString getInputParameterNameB(
        const bool csvMode = false
    ) const;
    Q_INVOKABLE bool isUnique(
        const bool &checkCurrentSelection = false
    ) const;
    Q_INVOKABLE bool setInputParameterAByName( const QString &name );
    Q_INVOKABLE bool setInputParameterBByName( const QString &name );
    Q_INVOKABLE double getCorrelation() const;
    Q_INVOKABLE void setCorrelation( const double &correlation = 0. );
    Q_INVOKABLE void setToSelected();

    bool getValid() const override;
    int columnCount() const override;
    void reconnectInputParameters();
    void reset();
    void set( int column, const QVariant &value ) override;
    void setInputParameterA( InputParameter *inputParameter = nullptr );
    void setInputParameterB( InputParameter *inputParameter = nullptr );
    void setInputParameterAById( const QUuid &id );
    void setInputParameterBById( const QUuid &id );

    static Correlation *getCorrelation(
        InputParameter *inputParamA,
        InputParameter *inputParamB
    );
    static Correlation fromJson(
        const QJsonObject &json,
        const bool &addToModel = true,
        QObject *parent = nullptr
    );
    static ModelControl<Correlation *> *getCorrelationModel();
    static QJsonArray correlationsToJson();
    static QList<int> columnWidths;
    static QString correlationsToCSVString();
    static QStringList headerLabels;
    static bool correlationIsUnique(
        const Correlation *newCorrelation,
        const bool &checkCurrentSelection = false
    );
    static bool inputParameterCorrelated( InputParameter *inputParameter );
    static bool removeSelectedModelRow();
    static void removeCorrelatedInputParameter(
        InputParameter *inputParameter
    );
    static void clearModel();
    static void correlationsFromJson(
        const QJsonArray &jsonArray,
        QObject *parent = nullptr
    );
    static void reconnectAllCorrelations();

private:
    InputParameter *mInputParameterA;
    InputParameter *mInputParameterB;
    double mCorrelation;

    static ModelControl<Correlation *> mCorrelationModel;
    static QString mCorrelationsHeaderString;
    static QString mCorrelationString;
    static QString mIdInputAString;
    static QString mIdInputBString;
};

#endif // CORRELATION_H
